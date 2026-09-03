#pragma once

#include "Utility/System/NativePath.h"

/**
 * Crash log that outlives the process. Every crash chunk is appended to the file as it arrives, so that a
 * crash in a process with no console - a mac app bundle launched from finder, say - still leaves its trace
 * somewhere. A started line goes in at construction and an exit line at destruction, so a started line with
 * neither an exit line nor a trace after it reads as a death no handler ever sees - SIGKILL, OOM, power loss.
 *
 * The file is written with raw writes to a descriptor that stays open for the life of the process, never
 * through the file system abstraction. That one virtualizes game data, while this is process diagnostics, like
 * stderr, and has to be a physical file that survives the process.
 *
 * One per process, and only the game binary constructs one, on the main thread before any other thread is
 * spawned.
 */
class Blackbox {
 public:
    /**
     * Opens the crash log, rotating it to `.old` first if it has grown past a megabyte, writes the started
     * line, and registers a crash callback that appends every chunk to the file before handing it on to the
     * callback that was in effect before. Degrades silently when the file can't be opened - no home directory,
     * an unwritable path - and the previous callback then stays in charge on its own.
     *
     * @param path                      Native path of the crash log, `<userDir>/crash.log` for the game.
     */
    explicit Blackbox(const NativePath &path);

    /**
     * Writes the exit line - a clean exit, or an exit with an exception when the destructor runs during
     * unwinding. The file stays open and the callback stays registered, static destructors can crash too, and
     * a trace after an exit line is exactly how such a crash should read.
     */
    ~Blackbox();

    /**
     * @return                          Whether the crash log is open. False means the file couldn't be opened
     *                                  and nothing is being logged, which is not an error, but is worth knowing
     *                                  before telling a user where to find their crash log.
     */
    bool isLogging() const;
};
