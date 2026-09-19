#include "StackTraceOnCrash.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#ifdef _WINDOWS
#   include <windows.h> // NOLINT: not a C system header.
#   include <dbghelp.h> // NOLINT: not a C system header.
#   include <csignal>
#   include <exception>
#   include <mutex>
#elif !defined(__ANDROID__)
#   include <unistd.h> // NOLINT: not a C++ system header.
#   include <sys/mman.h> // NOLINT: not a C++ system header.
#   include <sys/ucontext.h> // NOLINT: not a C++ system header.
#   ifdef __APPLE__
#       include <libunwind.h> // NOLINT: not a C++ system header.
#       include <sys/sysctl.h> // NOLINT: not a C++ system header.
#   else
#       include <unwind.h> // NOLINT: not a C++ system header.
#   endif
#   include <csignal>
#   include <cstring>
#endif

#include "Library/StackTrace/StackTrace.h"

#include "Utility/String/Format.h"

#ifdef __ANDROID__

StackTraceOnCrash::StackTraceOnCrash(void (*)()) {}

#else

// Set by whichever handler runs first. Handlers chain into each other - terminate calls abort, and a crash
// inside a handler re-enters it - and one trace is what's actually useful.
static std::atomic_flag crashHandled = ATOMIC_FLAG_INIT;

static void (*crashCallback)() = nullptr;

static void runCrashCallback() {
    if (crashCallback)
        crashCallback();
}

bool detail::isRunningUnderRosetta() {
#ifdef __APPLE__
    int translated = 0;
    size_t size = sizeof(translated);
    return sysctlbyname("sysctl.proc_translated", &translated, &size, nullptr, 0) == 0 && translated == 1; // The key only exists in a translated process.
#else
    return false;
#endif
}

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

static void printCrashTrace(std::string_view reason) {
    printCrashHeader(reason);
    printTrace(stackTraceToString());
}

#ifdef _WINDOWS

namespace cpptrace {
inline namespace v1 {
namespace detail {
/**
 * Cpptrace serializes its own dbghelp calls behind this lock - dbghelp is single-threaded. Declared rather
 * than included, because it lives in cpptrace's internals - a signature change there becomes a link error
 * here, which is the failure mode we want.
 *
 * @return                              The dbghelp lock.
 */
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

    std::unique_lock<std::recursive_mutex> lock = cpptrace::detail::get_dbghelp_lock();

    // The module lookup below, StackWalk64 and the symbol callbacks all need the symbol handler initialized
    // for the handle they're passed, and cpptrace initializes a duplicate of it rather than this one. Failure
    // means it was already initialized, which is just as good. Done at crash time rather than at install so
    // the module list is current - a dll loaded since then must not read as a bad call target.
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    // A call through a bad pointer faults at the bad address, where there's nothing to walk from. The call
    // pushed its return address first though, so pop it into the pc, minus one - a return address points one
    // past the call, and the unwind data lookup has to land inside the caller, not one past it. A bad
    // address is one no module was loaded at.
    if (SymGetModuleBase64(GetCurrentProcess(), reinterpret_cast<DWORD64>(exceptionAddress)) == 0) {
#if defined(_M_IX86)
        context.Eip = *reinterpret_cast<const DWORD *>(context.Esp) - 1;
        context.Esp += sizeof(DWORD);
#elif defined(_M_X64)
        context.Rip = *reinterpret_cast<const DWORD64 *>(context.Rsp) - 1;
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

    cpptrace::raw_trace raw;
    while (raw.frames.size() < detail::MAX_TRACE_DEPTH &&
           StackWalk64(machineType, GetCurrentProcess(), GetCurrentThread(), &frame,
                       machineType == IMAGE_FILE_MACHINE_I386 ? nullptr : &context, nullptr,
                       SymFunctionTableAccess64, SymGetModuleBase64, nullptr) &&
           frame.AddrPC.Offset != 0) {
        // Cpptrace resolves call sites, and a return address points one past the call. The first frame is
        // exact as is - the faulting instruction itself, or the popped return address that already got its
        // minus one above.
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
        runCrashCallback();
    }

    // Continuing the search hands the exception to windows error reporting, which is what writes the crash
    // dump, and to any attached debugger.
    return EXCEPTION_CONTINUE_SEARCH;
}

static void onAbort(int signal) {
    if (!crashHandled.test_and_set()) {
        printCrashTrace("abort()");
        runCrashCallback();
    }
    // Returning is fine here, abort() goes on to terminate the process.
}

static void onTerminate() {
    if (!crashHandled.test_and_set()) {
        printCrashTrace("std::terminate()");
        runCrashCallback();
    }
    std::abort(); // Ends the process, as returning from a terminate handler is undefined behavior.
}

static void __cdecl onPureCall() {
    if (!crashHandled.test_and_set()) {
        printCrashTrace("pure virtual function call");
        runCrashCallback();
    }
    std::abort(); // Ends the process, as a pure virtual call leaves nothing sane to continue with.
}

static void __cdecl onInvalidParameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file,
                                       unsigned int line, uintptr_t reserved) {
    if (!crashHandled.test_and_set()) {
        printCrashTrace("invalid parameter passed to a CRT function");
        runCrashCallback();
    }
    std::abort(); // Ends the process, as the CRT was handed garbage and can't carry on.
}

static void installHandlers() {
    SetUnhandledExceptionFilter(&onStructuredException);

    // A stack overflow dispatches its exception on the stack that just ran out, and symbolizing needs tens of
    // kilobytes, so ask windows to keep some committed for exception dispatch. Per-thread, like the posix
    // alternate stack.
    ULONG stackGuarantee = 128 * 1024;
    SetThreadStackGuarantee(&stackGuarantee);

    // Drop the CRT's own abort message, we print a trace instead. _CALL_REPORTFAULT is left alone so that
    // abort() still gets a crash dump out of windows error reporting, the way a structured exception does.
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    std::signal(SIGABRT, &onAbort);

    std::set_terminate(&onTerminate);
    _set_purecall_handler(&onPureCall);
    _set_invalid_parameter_handler(&onInvalidParameter);
}

#else // _WINDOWS

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

#ifdef __APPLE__

/**
 * Fills a libunwind context from the register state a signal was delivered with. Libunwind keeps its own
 * layout rather than the kernel's, so this is a register-by-register copy.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @param[out] context                  Libunwind context to walk from.
 */
static void fillUnwindContext(const ucontext_t &crashContext, unw_context_t *context) {
    std::memset(context, 0, sizeof(*context));
    uint64_t *regs = reinterpret_cast<uint64_t *>(context);
    const auto &ss = crashContext.uc_mcontext->__ss;
#if defined(__aarch64__)
    for (int i = 0; i < 29; i++) // x0-x28.
        regs[i] = ss.__x[i];
    regs[29] = ss.__fp;
    regs[30] = ss.__lr;
    regs[31] = ss.__sp;
    regs[32] = ss.__pc;
#elif defined(__x86_64__)
    const uint64_t order[] = {ss.__rax, ss.__rbx, ss.__rcx, ss.__rdx, ss.__rdi, ss.__rsi, ss.__rbp, ss.__rsp,
                              ss.__r8, ss.__r9, ss.__r10, ss.__r11, ss.__r12, ss.__r13, ss.__r14, ss.__r15,
                              ss.__rip};
    std::memcpy(regs, order, sizeof(order));
#else
#   error "Unsupported apple architecture."
#endif
}

/**
 * Walks the stack the signal interrupted, from the register state the crash left behind. The system unwinder
 * drops the frame a signal fired in when it walks across the trampoline from inside the handler, so the walk
 * is seeded with that frame's registers instead and never crosses the trampoline at all.
 *
 * A call through a bad pointer faults at the bad address, where there's nothing to walk from. The call pushed
 * its return address first though, so the pc is set back into that call and the walk carries on from there.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @param signal                        Signal that was delivered, `si_signo`.
 * @param faultAddress                  Address the signal was about, `si_addr`.
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const ucontext_t &crashContext, int signal, const void *faultAddress) {
    unw_context_t context;
    fillUnwindContext(crashContext, &context);

    // The patch goes into the context before the cursor exists. Setting the pc on a cursor doesn't make it
    // re-read the unwind info, and the walk would stop after one frame.
    uint64_t *regs = reinterpret_cast<uint64_t *>(&context);
#if defined(__aarch64__)
    uint64_t &pc = regs[32];
#else
    uint64_t &pc = regs[16];
#endif

    // A pc equal to si_addr means the pc itself is the problem - a jump to a bad address, or abort parked in
    // a kill syscall stub the unwinder can't step out of - and the pushed return address is the frame to
    // restart from. The instruction faults are the exception, SIGFPE, SIGILL and SIGTRAP put the pc of a
    // perfectly walkable instruction into si_addr, and restarting would seed the walk with garbage.
    if (signal != SIGFPE && signal != SIGILL && signal != SIGTRAP && pc == reinterpret_cast<uint64_t>(faultAddress)) {
#if defined(__aarch64__)
        pc = regs[30] - 1; // Minus one, so the pc points into the call, not one past it.
#else
        pc = *reinterpret_cast<const uint64_t *>(regs[7]) - 1; // Return address is at [rsp].
        regs[7] += sizeof(uint64_t); // And it's been popped.
#endif
    }

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    cpptrace::raw_trace raw;
    unw_word_t ip;
    do {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        // The first frame is the instruction itself, every later one is a return address one past its call.
        raw.frames.push_back(raw.frames.empty() ? ip : ip - 1);
    } while (raw.frames.size() < detail::MAX_TRACE_DEPTH && unw_step(&cursor) > 0);

    return raw.resolve().to_string();
}

#else // __APPLE__

struct dwarf_eh_bases {
    void *tbase, *dbase, *func;
};
extern "C" const void *_Unwind_Find_FDE(const void *pc, struct dwarf_eh_bases *bases); // Mirrors libgcc's own declaration, names and all.

/**
 * @param crashContext                  Register state the signal was delivered with.
 * @return                              Program counter the signal was delivered at.
 */
static uintptr_t faultingProgramCounter(const ucontext_t &crashContext) {
#if defined(__aarch64__)
    return crashContext.uc_mcontext.pc;
#elif defined(__x86_64__)
    return crashContext.uc_mcontext.gregs[REG_RIP];
#elif defined(__i386__)
    return crashContext.uc_mcontext.gregs[REG_EIP];
#elif defined(__arm__)
    return crashContext.uc_mcontext.arm_pc;
#else
#   error "Unsupported posix architecture."
#endif
}

static const size_t crashPageSize = getpagesize();

static bool isRangeMapped(uintptr_t address, size_t size) {
    uintptr_t mask = ~static_cast<uintptr_t>(crashPageSize - 1);
    uintptr_t last = (address + size - 1) & mask;
    for (uintptr_t page = address & mask; page <= last; page += crashPageSize)
        if (msync(reinterpret_cast<void *>(page), 1, MS_ASYNC) != 0)
            return false; // Fails with ENOMEM on an unmapped page, and touches nothing.
    return true;
}

/**
 * Walks the frame pointer chain from the register state a signal was delivered with. This is for a call
 * through a bad pointer, which faults at the bad address where the unwinder has nothing to go on. The call
 * pushed its return address first, and the frame pointer still points at the caller's frame, so both are
 * there to be read - which is why frame pointers are kept on every build.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @return                              Frames starting with the call that jumped to the bad address.
 */
static std::vector<cpptrace::frame_ptr> walkFramePointers(const ucontext_t &crashContext) {
    std::vector<cpptrace::frame_ptr> frames;
    uintptr_t returnAddress = 0;
    uintptr_t fp = 0;
#if defined(__aarch64__)
    returnAddress = crashContext.uc_mcontext.regs[30]; // lr
    fp = crashContext.uc_mcontext.regs[29];
#elif defined(__x86_64__) || defined(__i386__)
#   if defined(__x86_64__)
    uintptr_t sp = crashContext.uc_mcontext.gregs[REG_RSP];
    fp = crashContext.uc_mcontext.gregs[REG_RBP];
#   else
    uintptr_t sp = crashContext.uc_mcontext.gregs[REG_ESP];
    fp = crashContext.uc_mcontext.gregs[REG_EBP];
#   endif
    if (!isRangeMapped(sp, sizeof(uintptr_t)))
        return frames;
    returnAddress = *reinterpret_cast<const uintptr_t *>(sp);
#elif defined(__arm__)
    returnAddress = crashContext.uc_mcontext.arm_lr;
    fp = crashContext.uc_mcontext.arm_fp;
#endif
    frames.push_back(returnAddress - 1); // Minus one, so it points into the call, not one past it.

    while (frames.size() < detail::MAX_TRACE_DEPTH) {
        // Both checks precede the read and cover its whole span - [fp] is the caller's frame pointer and
        // [fp + 1] the return address, two words starting at fp. Checking alignment here rather than at the
        // bottom of the loop is what covers the fp that came out of the crash context.
        if (fp == 0 || fp % sizeof(uintptr_t) != 0)
            break;
        if (!isRangeMapped(fp, 2 * sizeof(uintptr_t)))
            break;

        const uintptr_t *frame = reinterpret_cast<const uintptr_t *>(fp);
        uintptr_t callerFp = frame[0];
        uintptr_t callerPc = frame[1];
        if (callerPc == 0)
            break;
        frames.push_back(callerPc - 1);

        // A frame built without a frame pointer holds something else in that slot. The caller's frame is always
        // further up the stack, so a value that isn't ends the walk - after the push, since at the frame above
        // main the return address is real while the slot is not.
        if (callerFp <= fp)
            break;
        fp = callerFp;
    }
    return frames;
}

/**
 * Walks the stack from inside the handler, then trims it back to the frame the signal interrupted. The libgcc
 * unwinder crosses the signal trampoline on its own and marks the interrupted frame as the one where the pc
 * is the faulting instruction rather than a return address, so that mark is where the trace starts.
 *
 * A call through a bad pointer is the one case the unwinder can't handle - there's no unwind info at the bad
 * address, so it stops there - and for that the frame pointer chain is walked by hand instead.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const ucontext_t &crashContext) {
    cpptrace::raw_trace raw;

    dwarf_eh_bases bases;
    if (!_Unwind_Find_FDE(reinterpret_cast<const void *>(faultingProgramCounter(crashContext)), &bases)) {
        raw.frames = walkFramePointers(crashContext);
        return raw.resolve().to_string();
    }

    struct Walk {
        std::vector<cpptrace::frame_ptr> frames;
        bool reachedFault = false;
    } walk;
    _Unwind_Backtrace([](struct _Unwind_Context *context, void *arg) {
        Walk &walk = *static_cast<Walk *>(arg);
        int isFault = 0;
        uintptr_t ip = _Unwind_GetIPInfo(context, &isFault);
        if (isFault)
            walk.reachedFault = true;
        if (!walk.reachedFault)
            return _URC_NO_REASON; // Still inside the handler.
        walk.frames.push_back(isFault ? ip : ip - 1);
        return walk.frames.size() < detail::MAX_TRACE_DEPTH ? _URC_NO_REASON : _URC_END_OF_STACK;
    }, &walk);

    raw.frames = std::move(walk.frames);
    return raw.resolve().to_string();
}

#endif // __APPLE__

static void onSignal(int signal, siginfo_t *info, void *context) {
    // Darwin honors SA_RESETHAND for every signal except SIGILL and SIGTRAP, and leaves this handler
    // installed for those two. Restoring the default by hand is what stops the raise below from re-entering
    // the handler instead of killing us.
    std::signal(info->si_signo, SIG_DFL);

    // Only the first crash prints. A second thread raising a different signal while this one symbolizes
    // would interleave with it, so it skips to the raise below instead - exiting here would cost the core
    // dump. A second thread raising the same signal never gets here, the default handler is back in place,
    // so the kernel kills the process and whatever was printed so far is all there is.
    if (!crashHandled.test_and_set()) {
        char reason[128];
        std::snprintf(reason, sizeof(reason), "%s at %p", strsignal(info->si_signo), info->si_addr);
        printCrashHeader(reason);
#ifdef __APPLE__
        printTrace(traceFromContext(*static_cast<ucontext_t *>(context), info->si_signo, info->si_addr));
#else
        printTrace(traceFromContext(*static_cast<ucontext_t *>(context)));
#endif
        runCrashCallback();
    }

    // Die of the original signal, so that a core dump still happens and whoever launched the process sees it
    // was killed by a SIGSEGV. The default handler is back in place, so raising it here is what actually
    // kills us.
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
        if (signal == SIGABRT && detail::isRunningUnderRosetta())
            continue; // A handler would hang the process on Rosetta's own abort, the default handler kills it.

        struct sigaction action;
        std::memset(&action, 0, sizeof(action));
        action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND;
        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, signal);
        action.sa_sigaction = &onSignal;
        sigaction(signal, &action, nullptr);
    }
}

#endif // _WINDOWS

StackTraceOnCrash::StackTraceOnCrash(void (*callback)()) {
    crashCallback = callback;

    // Symbols resolve lazily, so the first trace is the one that opens debug info and allocates. Better done
    // here than inside a handler, with the process already broken.
    warmUpCpptrace();
    installHandlers();
}

#endif // __ANDROID__
