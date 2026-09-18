#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "NativePath.h"

struct ProcessResult {
    int exitCode = 0; // On POSIX, 128 plus the signal number if the process was killed by a signal.
    std::string output; // Standard output and standard error, interleaved.
};

/**
 * Runs a process to completion, with nothing on its standard input.
 *
 * @param path                          Path to the executable, it's not looked up in `PATH`.
 * @param args                          Arguments, not including the executable name. WTF-8 on Windows, byte strings
 *                                      on POSIX.
 * @param timeout                       How long the process gets. It's killed once this runs out.
 * @return                              Exit code and output of the process.
 * @throw Exception                     If the process couldn't be started or didn't finish in time.
 */
ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout);

namespace detail {

/**
 * Windows takes a single command line string and leaves splitting it into `argv` to the child. This function quotes
 * the arguments so that the Microsoft C runtime splits them back into exactly what was passed in.
 *
 * @param program                       What the child will see as `argv[0]`.
 * @param args                          The rest of `argv`.
 * @return                              Command line string.
 */
std::string windowsCommandLine(std::string_view program, const std::vector<std::string> &args);

} // namespace detail
