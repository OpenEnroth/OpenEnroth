#include "StackTraceOnCrash.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#ifdef _WIN32
#   include <windows.h> // NOLINT: not a C system header.
#   include <csignal>
#   include <exception>
#elif !defined(__ANDROID__)
#   include <unistd.h> // NOLINT: not a C++ system header.
#   include <csignal>
#   include <cstring>
#endif

#include "Library/StackTrace/StackTrace.h"

#ifdef __ANDROID__

StackTraceOnCrash::StackTraceOnCrash() = default;

#else

// Set by whichever handler runs first. Handlers chain into each other - terminate calls abort, and a crash
// inside a handler re-enters it - and one trace is what's actually useful.
static std::atomic_flag crashHandled = ATOMIC_FLAG_INIT;

static void printCrashTrace() {
    std::string trace = stackTraceToString();

    std::fputs("\nCrashed, stack trace follows:\n", stderr);
    std::fputs(trace.c_str(), stderr);
    std::fputc('\n', stderr);
    std::fflush(stderr);
}

// Cpptrace resolves symbols lazily, so the first trace is the one that opens the debug info and allocates.
// Doing it here means the crash handler doesn't have to do it in an already broken process.
static void warmUpCpptrace() {
    (void) cpptrace::generate_trace(0, 1).to_string();
}

#ifdef _WIN32

// Structured exceptions are what access violations, illegal instructions and division by zero arrive as.
// Signals cover none of those on windows.
static LONG WINAPI onStructuredException(EXCEPTION_POINTERS *exceptionInfo) {
    if (!crashHandled.test_and_set())
        printCrashTrace();

    // The filter runs on the crashing thread with the faulting frames still below it, so the trace above is
    // the real one. Continuing the search hands the exception to WER and to any attached debugger.
    return EXCEPTION_CONTINUE_SEARCH;
}

// Runs inside abort(), which goes on to terminate the process once this returns.
static void onAbort(int signal) {
    if (!crashHandled.test_and_set())
        printCrashTrace();
}

// The three below must not return. A terminate handler that returns is undefined behavior, and returning from
// the other two resumes a process that just called a pure virtual function or passed garbage into the CRT.
static void onTerminate() {
    if (!crashHandled.test_and_set())
        printCrashTrace();
    std::abort();
}

static void onPureCall() {
    if (!crashHandled.test_and_set())
        printCrashTrace();
    std::abort();
}

static void onInvalidParameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file,
                               unsigned int line, uintptr_t reserved) {
    if (!crashHandled.test_and_set())
        printCrashTrace();
    std::abort();
}

static void installHandlers() {
    SetUnhandledExceptionFilter(&onStructuredException);

    // Without _CALL_REPORTFAULT abort() pops up the CRT dialog instead of reporting the fault.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
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
    if (crashHandled.test_and_set())
        _exit(EXIT_FAILURE); // Crashed inside the handler, printing again would just hang or recurse.

    printCrashTrace();

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
