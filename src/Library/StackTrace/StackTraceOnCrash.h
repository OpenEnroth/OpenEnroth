#pragma once

namespace detail {
bool isRunningUnderRosetta();
} // namespace detail

/**
 * Installs crash handlers that dump a stack trace to stderr, then let the crash proceed so that the OS still
 * produces a core dump or a crash report. The handlers are never uninstalled, the process is dying anyway.
 *
 * Construct one in `main` before anything else can crash. The handlers need stack of their own for when the
 * crash is stack exhaustion - an alternate signal stack on posix, a committed stack guarantee on windows -
 * and both are per-thread, so only the thread that constructs this gets one. On a worker thread the handlers
 * are left to run on whatever stack remains. The handlers themselves are process-wide.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks, so a crash while another thread holds one of those hangs the process instead of killing it. Crashing
 * inside the allocator does the same.
 *
 * Under Rosetta SIGABRT is left at its default disposition, so anything that ends in abort - a failed assert,
 * a terminate, a pure virtual call, a libc++ hardening check - dies there without a trace. Handling it would
 * deadlock the process instead, the faulting thread being parked by Rosetta where no signal reaches it.
 */
class StackTraceOnCrash {
 public:
    /**
     * @param callback                  Called after the crash trace is printed, right before the process
     *                                  dies.
     */
    explicit StackTraceOnCrash(void (*callback)() = nullptr);
};
