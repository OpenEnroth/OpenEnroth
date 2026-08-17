#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. On POSIX the handlers run on an alternate stack so
 * that they also work when the crash is stack exhaustion, and that stack is per-thread - only the thread that
 * constructs this gets one.
 */
class StackTraceOnCrash {
 public:
    StackTraceOnCrash();
};
