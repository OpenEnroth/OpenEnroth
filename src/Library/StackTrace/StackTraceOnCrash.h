#pragma once

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. Constructing more than one is harmless.
 */
class StackTraceOnCrash {
 public:
    StackTraceOnCrash();
};
