#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "NativePath.h"

struct ProcessResult {
    int exitCode = 0; // EXIT_FAILURE if the process was killed by a signal.
    std::string output; // Standard output and standard error, interleaved.
};

// TODO(captainurist): add WTF-8 support to subprocess.h upstream, it converts arguments with MB_ERR_INVALID_CHARS.
/**
 * Runs a process to completion, with nothing on its standard input.
 *
 * @param path                          Path to the executable.
 * @param args                          Arguments, not including the executable name. UTF-8 on Windows, byte strings
 *                                      on POSIX.
 * @param timeout                       How long the process gets. It's killed once this runs out.
 * @return                              Exit code and output of the process.
 * @throw Exception                     If the process couldn't be started or didn't finish in time.
 */
ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout);
