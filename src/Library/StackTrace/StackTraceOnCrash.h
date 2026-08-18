#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. On POSIX the handlers run on an alternate stack so
 * that they also work when the crash is stack exhaustion, and that stack is per-thread - only the thread that
 * constructs this gets one, so a stack overflow on a worker thread dies without a trace. Crashes that aren't
 * stack exhaustion are traced on every thread, the handlers themselves are process-wide. Windows has no
 * alternate stack to run on, so a stack overflow is never traced there.
 *
 * How much of a trace a crash produces varies. 32-bit windows gets none, because getting back across ntdll's
 * dispatcher needs the CONTEXT record and there is no way to hand it over, and macos x86_64 gets none in
 * optimized builds. The signal or exception, and the faulting address, are reported either way.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks, so a crash while another thread holds one of those hangs the process instead of killing it. Crashing
 * inside the allocator does the same. Doing better needs out-of-process tracing, which isn't worth what it
 * would cost.
 */
class StackTraceOnCrash {
 public:
    StackTraceOnCrash();
};
