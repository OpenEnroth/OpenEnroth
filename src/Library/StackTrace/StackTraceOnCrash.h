#pragma once

#include <string_view>

namespace detail {
bool isRunningUnderRosetta();
} // namespace detail

/**
 * Receives crash output, chunk by chunk. Runs inside a signal handler on an alternate stack, in a dying
 * process - so no allocation where it can be avoided, raw `write` rather than stdio for file output, and no
 * locks that might already be held.
 *
 * The chunk with `final` set may never arrive - symbolization can hang, and a second crash with the same
 * signal kills the process outright. Treat `final` as best effort.
 *
 * @param text                          Chunk text, without a trailing newline. Sinks append their own.
 * @param final                         Whether this is the last chunk of this crash.
 */
using CrashCallback = void (*)(std::string_view text, bool final);

/**
 * Writes a chunk and a newline to a file descriptor with raw writes - nothing buffered, no stdio lock taken,
 * nothing allocated. This is what `printCrashChunk` does with stderr, and what a crash log does with its own
 * descriptor.
 *
 * @param fd                            File descriptor to write to.
 * @param text                          Chunk text, without a trailing newline.
 */
void writeCrashChunk(int fd, std::string_view text);

/**
 * Default crash callback, writes the chunk to stderr. Exported so that app callbacks can compose with it
 * instead of reimplementing stderr output.
 *
 * @param text                          Chunk text.
 * @param final                         Ignored.
 */
void printCrashChunk(std::string_view text, bool final);

/**
 * Installs crash handlers that hand a stack trace to the crash callback, then let the crash proceed so that
 * the OS still produces a core dump or a crash report. The handlers are never uninstalled, the process is
 * dying anyway. Only the first call installs them, every call replaces the callback - last call wins and
 * nothing merges, so a callback that still wants stderr output calls `printCrashChunk` itself, or chains to
 * the callback returned.
 *
 * Call in `main` before anything else can crash, then again once there is more to hand a crash to, like a
 * crash log file. The handlers need stack of their own for when the crash is stack exhaustion - an alternate
 * signal stack on posix, a committed stack guarantee on windows - and both are per-thread, so only the thread
 * that makes the first call gets one. On a worker thread the handlers are left to run on whatever stack
 * remains. The handlers themselves are process-wide.
 *
 * The handlers are not async-signal-safe, and can't be - symbolizing a trace allocates, reads files and takes
 * locks, so a crash while another thread holds one of those hangs the process instead of killing it. Only
 * the trace is exposed to that though - the reason line goes out first, without allocating, so a crash inside
 * the allocator still leaves that line behind.
 *
 * Under Rosetta SIGABRT is left at its default disposition, so anything that ends in abort - a failed assert,
 * a terminate, a pure virtual call, a libc++ hardening check - dies there without a trace. Handling it would
 * deadlock the process instead, the faulting thread being parked by Rosetta where no signal reaches it.
 *
 * @param callback                      Callback to hand crash output to, or `nullptr` for `printCrashChunk`.
 * @return                              Callback that was in effect before this call, `printCrashChunk` on the
 *                                      first one. Never `nullptr`. Chain to it, or hand it back to restore it.
 */
CrashCallback initStackTraceOnCrash(CrashCallback callback = nullptr);
