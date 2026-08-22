#include "StackTraceOnCrash.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#ifdef _WIN32
#   include <windows.h> // NOLINT: not a C system header.
#   include <dbghelp.h> // NOLINT: not a C system header.
#   include <csignal>
#   include <exception>
#   include <mutex>
#elif !defined(__ANDROID__)
#   include <unistd.h> // NOLINT: not a C++ system header.
#   include <sys/ucontext.h> // NOLINT: not a C++ system header.
#   include <libunwind.h> // NOLINT: not a C++ system header.
#   include <csignal>
#   include <cstring>
#endif

#include "Library/StackTrace/StackTrace.h"

#include "Utility/String/Format.h"

#ifdef __ANDROID__

StackTraceOnCrash::StackTraceOnCrash() = default;

#else

// Set by whichever handler runs first. Handlers chain into each other - terminate calls abort, and a crash
// inside a handler re-enters it - and one trace is what's actually useful.
static std::atomic_flag crashHandled = ATOMIC_FLAG_INIT;

static void printCrashHeader(std::string_view reason) {
    // Flushed on its own because building the trace can hang, and then this is all anyone sees.
    fmt::println(stderr, "\nCrashed because of {}", reason);
    std::fflush(stderr);
}

static void printTrace(std::string_view trace) {
    fmt::println(stderr, "{}", trace);
    std::fflush(stderr);
}

static void warmUpCpptrace() {
    (void) cpptrace::generate_trace(0, 1).to_string();
}

#ifdef _WIN32

static void printCrashTrace(std::string_view reason) {
    printCrashHeader(reason);
    printTrace(stackTraceToString());
}

namespace cpptrace {
inline namespace v1 {
namespace detail {
// Cpptrace serializes its own dbghelp calls behind this, and dbghelp is single-threaded. Declared rather than
// included because it lives in cpptrace's internals - a signature change there becomes a link error here,
// which is the failure mode we want.
std::unique_lock<std::recursive_mutex> get_dbghelp_lock();
} // namespace detail
} // namespace v1
} // namespace cpptrace

/**
 * Walks the stack the exception was raised on, rather than the one the filter is running on. Cpptrace only
 * ever captures the latter, and getting from there back to the fault means crossing ntdll's dispatcher, which
 * has no frame pointers to follow - 64-bit manages it on unwind data, 32-bit doesn't get across at all.
 * Seeding the walk with the faulting context skips that problem instead of solving it.
 *
 * @param crashContext                  Register state the exception was raised with.
 * @param exceptionAddress              Address the exception was raised at.
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const CONTEXT &crashContext, const void *exceptionAddress) {
    CONTEXT context = crashContext; // StackWalk64 walks by mutating it.

    // A call through a bad pointer faults at the bad address, where there's nothing to walk from. The call
    // pushed its return address first though, so pop it back into the pc and carry on from the caller.
    if (exceptionAddress == nullptr) {
#if defined(_M_IX86)
        context.Eip = *reinterpret_cast<const DWORD *>(context.Esp);
        context.Esp += sizeof(DWORD);
#elif defined(_M_X64)
        context.Rip = *reinterpret_cast<const DWORD64 *>(context.Rsp);
        context.Rsp += sizeof(DWORD64);
#endif
    }

    STACKFRAME64 frame = {};
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
#if defined(_M_IX86)
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context.Eip;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrStack.Offset = context.Esp;
#elif defined(_M_X64)
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = context.Rip;
    frame.AddrFrame.Offset = context.Rsp;
    frame.AddrStack.Offset = context.Rsp;
#else
#   error "Unsupported windows architecture."
#endif

    std::unique_lock<std::recursive_mutex> lock = cpptrace::detail::get_dbghelp_lock();

    // StackWalk64 and the symbol callbacks below need the symbol handler initialized for the handle they're
    // passed, and cpptrace initializes a duplicate of it rather than this one. Failure means it was already
    // initialized, which is just as good.
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    cpptrace::raw_trace raw;
    while (raw.frames.size() < detail::MAX_TRACE_DEPTH &&
           StackWalk64(machineType, GetCurrentProcess(), GetCurrentThread(), &frame,
                       machineType == IMAGE_FILE_MACHINE_I386 ? nullptr : &context, nullptr,
                       SymFunctionTableAccess64, SymGetModuleBase64, nullptr) &&
           frame.AddrPC.Offset != 0) {
        // Cpptrace resolves call sites, and a return address points one past the call. The first frame is
        // the faulting instruction itself, so it's exact as is.
        cpptrace::frame_ptr pc = static_cast<cpptrace::frame_ptr>(frame.AddrPC.Offset);
        raw.frames.push_back(raw.frames.empty() ? pc : pc - 1);
    }
    lock.unlock();

    return raw.resolve().to_string();
}

/**
 * Structured exceptions are what access violations, illegal instructions and division by zero arrive as.
 * Signals cover none of those on windows.
 *
 * @param exceptionInfo                 Exception record and register state of the crash.
 * @return                              Always `EXCEPTION_CONTINUE_SEARCH`, so that the crash proceeds.
 */
static LONG WINAPI onStructuredException(EXCEPTION_POINTERS *exceptionInfo) {
    if (!crashHandled.test_and_set()) {
        char reason[128];
        std::snprintf(reason, sizeof(reason), "exception %#lx at %p",
                      exceptionInfo->ExceptionRecord->ExceptionCode, exceptionInfo->ExceptionRecord->ExceptionAddress);
        printCrashHeader(reason);
        printTrace(traceFromContext(*exceptionInfo->ContextRecord, exceptionInfo->ExceptionRecord->ExceptionAddress));
    }

    // Continuing the search hands the exception to windows error reporting, which is what writes the crash
    // dump, and to any attached debugger.
    return EXCEPTION_CONTINUE_SEARCH;
}

static void onAbort(int signal) {
    if (!crashHandled.test_and_set())
        printCrashTrace("abort()");
    // Returning is fine here, abort() goes on to terminate the process.
}

static void onTerminate() {
    if (!crashHandled.test_and_set())
        printCrashTrace("std::terminate()");
    std::abort(); // Ends the process, as returning from a terminate handler is undefined behavior.
}

static void __cdecl onPureCall() {
    if (!crashHandled.test_and_set())
        printCrashTrace("pure virtual function call");
    std::abort(); // Ends the process, as a pure virtual call leaves nothing sane to continue with.
}

static void __cdecl onInvalidParameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file,
                                       unsigned int line, uintptr_t reserved) {
    if (!crashHandled.test_and_set())
        printCrashTrace("invalid parameter passed to a CRT function");
    std::abort(); // Ends the process, as the CRT was handed garbage and can't carry on.
}

static void installHandlers() {
    SetUnhandledExceptionFilter(&onStructuredException);

    // Drop the CRT's own abort message, we print a trace instead. _CALL_REPORTFAULT is left alone so that
    // abort() still gets a crash dump out of windows error reporting, the way a structured exception does.
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    std::signal(SIGABRT, &onAbort);

    std::set_terminate(&onTerminate);
    _set_purecall_handler(&onPureCall);
    _set_invalid_parameter_handler(&onInvalidParameter);
}

#else // _WIN32

static const int handledSignals[] = {
    SIGABRT, // abort().
    SIGBUS, // Bad memory access.
    SIGFPE, // Floating point exception.
    SIGILL, // Illegal instruction.
    SIGQUIT, // Quit from keyboard.
    SIGSEGV, // Invalid memory reference.
    SIGSYS, // Bad argument to routine.
    SIGTRAP, // Trace/breakpoint trap.
    SIGXCPU, // CPU time limit exceeded.
    SIGXFSZ, // File size limit exceeded.
};

/**
 * Fills a libunwind context from the register state a signal was delivered with. Libunwind keeps its own
 * layout rather than the kernel's, so this is a register-by-register copy on every platform.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @param[out] context                  Libunwind context to walk from.
 */
static void fillUnwindContext(const ucontext_t &crashContext, unw_context_t *context) {
    std::memset(context, 0, sizeof(*context));
    uint64_t *regs = reinterpret_cast<uint64_t *>(context);
#if defined(__aarch64__)
    // x0-x28, fp, lr, sp, pc. Apple and linux name the fields differently and lay them out the same.
#   if defined(__APPLE__)
    const auto &ss = crashContext.uc_mcontext->__ss;
    for (int i = 0; i < 29; i++)
        regs[i] = ss.__x[i];
    regs[29] = ss.__fp;
    regs[30] = ss.__lr;
    regs[31] = ss.__sp;
    regs[32] = ss.__pc;
#   else
    for (int i = 0; i < 31; i++)
        regs[i] = crashContext.uc_mcontext.regs[i];
    regs[31] = crashContext.uc_mcontext.sp;
    regs[32] = crashContext.uc_mcontext.pc;
#   endif
#elif defined(__x86_64__)
    // rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, r8-r15, rip.
#   if defined(__APPLE__)
    const auto &ss = crashContext.uc_mcontext->__ss;
    const uint64_t order[] = {ss.__rax, ss.__rbx, ss.__rcx, ss.__rdx, ss.__rdi, ss.__rsi, ss.__rbp, ss.__rsp,
                              ss.__r8, ss.__r9, ss.__r10, ss.__r11, ss.__r12, ss.__r13, ss.__r14, ss.__r15,
                              ss.__rip};
#   else
    const auto &g = crashContext.uc_mcontext.gregs;
    const uint64_t order[] = {g[REG_RAX], g[REG_RBX], g[REG_RCX], g[REG_RDX], g[REG_RDI], g[REG_RSI],
                              g[REG_RBP], g[REG_RSP], g[REG_R8], g[REG_R9], g[REG_R10], g[REG_R11],
                              g[REG_R12], g[REG_R13], g[REG_R14], g[REG_R15], g[REG_RIP]};
#   endif
    std::memcpy(regs, order, sizeof(order));
#elif defined(__i386__)
    // eax, ebx, ecx, edx, edi, esi, ebp, esp, ss, eflags, eip. All 32-bit, so not the uint64_t view above.
    const auto &g = crashContext.uc_mcontext.gregs;
    const uint32_t order[] = {static_cast<uint32_t>(g[REG_EAX]), static_cast<uint32_t>(g[REG_EBX]),
                              static_cast<uint32_t>(g[REG_ECX]), static_cast<uint32_t>(g[REG_EDX]),
                              static_cast<uint32_t>(g[REG_EDI]), static_cast<uint32_t>(g[REG_ESI]),
                              static_cast<uint32_t>(g[REG_EBP]), static_cast<uint32_t>(g[REG_ESP]),
                              static_cast<uint32_t>(g[REG_SS]),  static_cast<uint32_t>(g[REG_EFL]),
                              static_cast<uint32_t>(g[REG_EIP])};
    std::memcpy(context, order, sizeof(order));
#else
#   error "Unsupported posix architecture."
#endif
}

/**
 * Walks the stack the signal interrupted rather than the handler's own. The walk starts from the register
 * state the crash left behind, so it names the faulting function first and carries on through its callers,
 * which walking from inside the handler can't do - nothing reliably unwinds across the signal trampoline.
 *
 * A call through a bad pointer faults at the bad address, where there's nothing to walk from. The call pushed
 * its return address first though, so the pc is set back into that call and the walk carries on from there.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @param faultAddress                  Address the signal was about, `si_addr`.
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const ucontext_t &crashContext, const void *faultAddress) {
    unw_context_t context;
    fillUnwindContext(crashContext, &context);

    // The patch has to go into the context, before the cursor exists. Setting the pc on a cursor doesn't make
    // it look up the unwind info for the new pc, so the walk would stop after one frame.
#if defined(__aarch64__) || defined(__x86_64__)
    uint64_t *regs = reinterpret_cast<uint64_t *>(&context);
#endif
#if defined(__aarch64__)
    uint64_t &pcReg = regs[32];
    if (pcReg == reinterpret_cast<uint64_t>(faultAddress))
        pcReg = regs[30] - 1; // Back into the call that jumped here, that's the frame to name.
#elif defined(__x86_64__)
    uint64_t &pcReg = regs[16];
    if (pcReg == reinterpret_cast<uint64_t>(faultAddress)) {
        pcReg = *reinterpret_cast<const uint64_t *>(regs[7]) - 1; // Return address is at [rsp].
        regs[7] += sizeof(uint64_t); // And it's been popped.
    }
#else
    uint32_t *regs32 = reinterpret_cast<uint32_t *>(&context);
    uint32_t &pcReg = regs32[10];
    if (pcReg == reinterpret_cast<uintptr_t>(faultAddress)) {
        pcReg = *reinterpret_cast<const uint32_t *>(regs32[7]) - 1; // Return address is at [esp].
        regs32[7] += sizeof(uint32_t);
    }
#endif

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    cpptrace::raw_trace raw;
    unw_word_t pc;
    do {
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        // The first frame is the instruction itself, every later one is a return address one past its call.
        raw.frames.push_back(raw.frames.empty() ? pc : pc - 1);
    } while (raw.frames.size() < detail::MAX_TRACE_DEPTH && unw_step(&cursor) > 0);

    return raw.resolve().to_string();
}

static void onSignal(int signal, siginfo_t *info, void *context) {
    // Only the first crash prints. A second thread raising a different signal while this one symbolizes
    // would interleave with it, so it skips to the raise below instead - exiting here would cost the core
    // dump. A second thread raising the same signal never gets here, SA_RESETHAND already put the default
    // handler back, so the kernel kills the process and whatever was printed so far is all there is.
    if (!crashHandled.test_and_set()) {
        char reason[128];
        std::snprintf(reason, sizeof(reason), "%s at %p", strsignal(info->si_signo), info->si_addr);
        printCrashHeader(reason);
        printTrace(traceFromContext(*static_cast<ucontext_t *>(context), info->si_addr));
    }

    // Die of the original signal, so that a core dump still happens and whoever launched the process sees it
    // was killed by a SIGSEGV. SA_RESETHAND put the default handler back before we were called, so raising it
    // here is what actually kills us.
    std::raise(info->si_signo);
    _exit(EXIT_FAILURE);
}

static void installHandlers() {
    // The handler runs on its own stack so that it also works when the crash is stack exhaustion. Symbolizing
    // a trace measures at 17kb, well past SIGSTKSZ, and the rest is margin for the platforms that weren't
    // measured. This is bss, so the pages past what's touched never get committed.
    static char alternateStack[1024 * 1024];

    stack_t stack;
    stack.ss_sp = alternateStack;
    stack.ss_size = sizeof(alternateStack);
    stack.ss_flags = 0;
    sigaltstack(&stack, nullptr);

    for (int signal : handledSignals) {
        struct sigaction action;
        std::memset(&action, 0, sizeof(action));
        action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND;
        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, signal);
        action.sa_sigaction = &onSignal;
        sigaction(signal, &action, nullptr);
    }
}

#endif // _WIN32

StackTraceOnCrash::StackTraceOnCrash() {
    // Symbols resolve lazily, so the first trace is the one that opens debug info and allocates. Better done
    // here than inside a handler, with the process already broken.
    warmUpCpptrace();
    installHandlers();
}

#endif // __ANDROID__
