#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "NativePath.h"

struct ProcessResult {
    std::optional<int> exitCode; // Empty if the process ran out of time. EXIT_FAILURE if it was killed by a signal.
    std::string output; // Standard output and standard error, interleaved.
};

// TODO(captainurist): add WTF-8 support to subprocess.h upstream, it converts arguments with MB_ERR_INVALID_CHARS.
/**
 * Runs a process to completion, with nothing on its standard input.
 *
 * @param path                          Path to the executable.
 * @param args                          Arguments, not including the executable name. UTF-8 on Windows, byte strings
 *                                      on POSIX.
 * @param timeout                       How long the process gets before it's killed, zero means no limit.
 * @return                              Exit code and output of the process.
 * @throw Exception                     If the process couldn't be started.
 */
ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout = {});
