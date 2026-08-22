#include "StackTraceOnCrash.h"

#include <algorithm>
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
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const CONTEXT &crashContext) {
    CONTEXT context = crashContext; // StackWalk64 walks by mutating it.

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
        printTrace(traceFromContext(*exceptionInfo->ContextRecord));
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
 * @param crashContext                  Register state the signal was delivered with.
 * @return                              Address of the instruction that faulted.
 */
static cpptrace::frame_ptr faultingProgramCounter(const ucontext_t &crashContext) {
#if defined(__APPLE__) && defined(__aarch64__)
    return crashContext.uc_mcontext->__ss.__pc;
#elif defined(__APPLE__) && defined(__x86_64__)
    return crashContext.uc_mcontext->__ss.__rip;
#elif defined(__aarch64__)
    return crashContext.uc_mcontext.pc;
#elif defined(REG_RIP)
    return crashContext.uc_mcontext.gregs[REG_RIP];
#elif defined(REG_EIP)
    return crashContext.uc_mcontext.gregs[REG_EIP];
#elif defined(__arm__)
    return crashContext.uc_mcontext.arm_pc;
#else
#   error "Unsupported posix architecture."
#endif
}

/**
 * @param crashContext                  Register state the signal was delivered with.
 * @return                              Where the faulting function would return to. For a call that jumped
 *                                      to a bad address this is the one frame the walk can't recover.
 */
static cpptrace::frame_ptr returnAddress(const ucontext_t &crashContext) {
#if defined(__APPLE__) && defined(__aarch64__)
    return crashContext.uc_mcontext->__ss.__lr;
#elif defined(__APPLE__) && defined(__x86_64__)
    return *reinterpret_cast<const cpptrace::frame_ptr *>(crashContext.uc_mcontext->__ss.__rsp);
#elif defined(__aarch64__)
    return crashContext.uc_mcontext.regs[30];
#elif defined(REG_RIP)
    return *reinterpret_cast<const cpptrace::frame_ptr *>(crashContext.uc_mcontext.gregs[REG_RSP]);
#elif defined(REG_EIP)
    return *reinterpret_cast<const cpptrace::frame_ptr *>(crashContext.uc_mcontext.gregs[REG_ESP]);
#elif defined(__arm__)
    return crashContext.uc_mcontext.arm_lr;
#else
#   error "Unsupported posix architecture."
#endif
}

/**
 * @param crashContext                  Register state the signal was delivered with.
 * @param faultAddress                  Address the signal was about, `si_addr`.
 * @return                              Program counter to start the trace from. When the crash was a jump to
 *                                      a bad address the PC is that address and there's nothing to unwind
 *                                      at it, so this hands back where the call came from instead.
 */
static cpptrace::frame_ptr startingProgramCounter(const ucontext_t &crashContext, const void *faultAddress) {
    cpptrace::frame_ptr pc = faultingProgramCounter(crashContext);
    if (pc == reinterpret_cast<cpptrace::frame_ptr>(faultAddress))
        return returnAddress(crashContext) - 1; // Back into the call, that's the frame the trace names.
    return pc;
}

/**
 * Walks from the handler, then trims the trace back to the instruction that faulted. Whether the walk crosses
 * the signal trampoline at all is up to the platform - a function that faulted without setting up a frame of
 * its own has nothing to find, and the walk picks up at its caller instead. The register state has the
 * address either way, so it's what decides where the trace starts.
 *
 * @param crashContext                  Register state the signal was delivered with.
 * @param faultAddress                  Address the signal was about, `si_addr`.
 * @return                              Stack trace starting at the faulting frame, one frame per line.
 */
static std::string traceFromContext(const ucontext_t &crashContext, const void *faultAddress) {
    cpptrace::raw_trace raw = cpptrace::generate_raw_trace(0, detail::MAX_TRACE_DEPTH);
    cpptrace::frame_ptr startPc = startingProgramCounter(crashContext, faultAddress);

    // Exact address first. The walk stores return addresses backed up into their call, so the frame below the
    // trampoline carries a pc one behind the real one on some platforms, and then this doesn't hit.
    auto fault = std::ranges::find(raw.frames, startPc);
    if (fault != raw.frames.end()) {
        raw.frames.erase(raw.frames.begin(), fault); // Walk got there, so drop the handler above it.
        return raw.resolve().to_string();
    }

    // Resolve and find the trampoline. Everything through it is the handler. No trampoline means there's
    // nowhere to put the fault that wouldn't be a lie, so the trace goes out as walked.
    // The trampoline is the last frame of the handler. Macos names it, linux doesn't name its restorer at
    // all, so there it's the first frame with no symbol. Anything else unnamed this early would be our own
    // handler, and that always resolves.
    cpptrace::stacktrace resolved = raw.resolve();
    auto trampoline = std::ranges::find_if(resolved.frames, [](const cpptrace::stacktrace_frame &frame) {
        return frame.symbol.contains("sigtramp") || frame.symbol.empty();
    });
    if (trampoline == resolved.frames.end())
        return resolved.to_string(); // Nowhere to put the fault that wouldn't be a lie.

    // Resolving expands inlined calls into frames of their own, so positions in the resolved trace aren't
    // positions in the raw one. Only frames that came from the walk count.
    auto rawIndexOf = [&](std::vector<cpptrace::stacktrace_frame>::iterator it) {
        return std::ranges::count_if(resolved.frames.begin(), it, [](const cpptrace::stacktrace_frame &frame) {
            return !frame.is_inline;
        });
    };
    size_t eraseThrough = rawIndexOf(std::next(trampoline));

    // The frame right below the trampoline is the walk's idea of where the fault was. Where it kept the
    // function at a stale pc it's the same function as the fault, and keeping both would list it twice.
    cpptrace::stacktrace_frame faultFrame = cpptrace::raw_trace{{startPc}}.resolve().frames.front();
    auto below = std::next(trampoline);
    if (below != resolved.frames.end() && !below->symbol.empty() && below->symbol == faultFrame.symbol)
        eraseThrough = rawIndexOf(std::next(below));

    raw.frames.erase(raw.frames.begin(), raw.frames.begin() + eraseThrough);
    raw.frames.insert(raw.frames.begin(), startPc);
    return raw.resolve().to_string();
}

static void onSignal(int signal, siginfo_t *info, void *context) {
    // Only the first crash prints. A second thread raising a different signal while this one symbolizes
    // would interleave with it, so it skips to the raise below instead - exiting here would cost the core
    // dump. A second thread raising the same signal never gets here, SA_RESETHAND already put the default
    // handler back, so the kernel kills the process and whatever was printed so far is all there is.
    if (!crashHandled.test_and_set()) {
        char reason[128];
        const ucontext_t &crashContext = *static_cast<ucontext_t *>(context);
        cpptrace::frame_ptr startPc = startingProgramCounter(crashContext, info->si_addr);
        std::string where = cpptrace::raw_trace{{startPc}}.resolve().frames.front().symbol;
        std::snprintf(reason, sizeof(reason), "%s at %p, in %s", strsignal(info->si_signo), info->si_addr,
                      where.empty() ? "an unknown function" : where.c_str());
        printCrashHeader(reason);
        printTrace(traceFromContext(crashContext, info->si_addr));
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
