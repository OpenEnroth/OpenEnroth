#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. On POSIX the handlers run on an alternate stack so
 * that they also work when the crash is stack exhaustion, and that stack is per-thread - only the thread that
 * constructs this gets one. Windows has no equivalent, and printing a trace needs stack space that a stack
 * overflow is precisely out of, so an overflow there dies without one. Backward-cpp kept a thread parked at
 * startup to have somewhere to report from, which is what that cost.
 *
 * How much of a trace a crash produces varies. 32-bit windows gets none, because getting back across ntdll's
 * dispatcher needs the CONTEXT record and cpptrace has no API that takes one, and macos x86_64 gets none in
 * optimized builds. The reported exception or signal, and the faulting address, are there either way.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks. So a crash that happens while another thread holds the malloc lock, or one of cpptrace's, deadlocks
 * the handler and the process hangs instead of dying. Crashing inside the allocator does the same. This is
 * what backward-cpp did before it too, and getting rid of it needs a separate tracer process - cpptrace's
 * signal-safe API only collects raw addresses, and resolving them in the handler is exactly what's unsafe.
 */
class StackTraceOnCrash {
 public:
    StackTraceOnCrash();
};
