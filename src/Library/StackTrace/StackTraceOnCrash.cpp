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

// The reason goes out first and on its own. Building a trace is what can hang, and when it does this is the
// only thing anyone gets to see.
static void printCrashHeader(std::string_view reason) {
    fmt::println(stderr, "\nCrashed because of {}", reason);
    std::fflush(stderr);
}

static void printTrace(std::string_view trace) {
    fmt::println(stderr, "{}", trace);
    std::fflush(stderr);
}

static void printCrashTrace(std::string_view reason) {
    printCrashHeader(reason);
    printTrace(stackTraceToString());
}

// Cpptrace resolves symbols lazily, so the first trace is the one that opens the debug info and allocates.
// Doing it here means the crash handler doesn't have to do it in an already broken process.
static void warmUpCpptrace() {
    (void) cpptrace::generate_trace(0, 1).to_string();
}

#ifdef _WIN32

// Cpptrace serializes its own dbghelp calls behind this, and dbghelp is single-threaded. Declared rather than
// included because it lives in cpptrace's internals - a signature change there becomes a link error here,
// which is the failure mode we want.
namespace cpptrace {
inline namespace v1 {
namespace detail {
std::unique_lock<std::recursive_mutex> get_dbghelp_lock();
} // namespace detail
} // namespace v1
} // namespace cpptrace

// Walks the stack the exception was raised on, rather than the one the filter is running on. Cpptrace only
// ever captures the latter, and getting from there back to the fault means crossing ntdll's dispatcher, which
// has no frame pointers to follow - 64-bit manages it on unwind data, 32-bit doesn't get across at all.
// Seeding the walk with the faulting context skips that problem instead of solving it.
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

    cpptrace::raw_trace raw;
    {
        std::unique_lock<std::recursive_mutex> lock = cpptrace::detail::get_dbghelp_lock();

        // Cpptrace has already done this from the warmup, doing it again is how we stop depending on that.
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);

        while (raw.frames.size() < MAX_TRACE_DEPTH &&
               StackWalk64(machineType, GetCurrentProcess(), GetCurrentThread(), &frame,
                           machineType == IMAGE_FILE_MACHINE_I386 ? nullptr : &context, nullptr,
                           SymFunctionTableAccess64, SymGetModuleBase64, nullptr) &&
               frame.AddrPC.Offset != 0) {
            raw.frames.push_back(static_cast<cpptrace::frame_ptr>(frame.AddrPC.Offset));
        }
    }

    return raw.resolve().to_string(); // Takes the same lock, which is why it's recursive.
}

// Structured exceptions are what access violations, illegal instructions and division by zero arrive as.
// Signals cover none of those on windows.
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

// Runs inside abort(), which goes on to terminate the process once this returns.
static void onAbort(int signal) {
    if (!crashHandled.test_and_set())
        printCrashTrace("abort()");
}

// The three below must not return. A terminate handler that returns is undefined behavior, and returning from
// the other two resumes a process that just called a pure virtual function or passed garbage into the CRT.
static void onTerminate() {
    if (!crashHandled.test_and_set())
        printCrashTrace("std::terminate()");
    std::abort();
}

static void __cdecl onPureCall() {
    if (!crashHandled.test_and_set())
        printCrashTrace("pure virtual function call");
    std::abort();
}

static void __cdecl onInvalidParameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file,
                                       unsigned int line, uintptr_t reserved) {
    if (!crashHandled.test_and_set())
        printCrashTrace("invalid parameter passed to a CRT function");
    std::abort();
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

#else

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

static void onSignal(int signal, siginfo_t *info, void *context) {
    // Only the first crash prints, a second thread faulting while this one symbolizes would interleave with
    // it. It still falls through to the raise below - exiting here instead would cost us the core dump.
    if (!crashHandled.test_and_set()) {
        char reason[128];
        std::snprintf(reason, sizeof(reason), "%s at %p", strsignal(info->si_signo), info->si_addr);
        printCrashTrace(reason);
    }

    // Die of the original signal rather than of us, so that a core dump still happens and whoever launched
    // the process sees it was killed by a SIGSEGV. SA_RESETHAND put the default handler back before we were
    // called, so raising it here is what actually kills us.
    std::raise(info->si_signo);
    _exit(EXIT_FAILURE);
}

static void installHandlers() {
    // The handler runs on its own stack so that it also works when the crash is stack exhaustion. The default
    // SIGSTKSZ is a few kb, and symbolizing a trace needs orders of magnitude more.
    static char alternateStack[8 * 1024 * 1024];

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

#endif

StackTraceOnCrash::StackTraceOnCrash() {
    warmUpCpptrace();
    installHandlers();
}

#endif
