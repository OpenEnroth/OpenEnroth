#include "Process.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#ifndef __ANDROID__
#   include <subprocess.h>
#endif

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"

#ifdef __ANDROID__

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &, std::chrono::milliseconds) {
    throw Exception("{}: can't start a process on Android", path.displayString()); // posix_spawn needs API level 28.
}

#else // __ANDROID__

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout) {
    std::string displayString = path.displayString();

    std::string program = path.toWtf8();
#ifdef _WINDOWS
    std::ranges::replace(program, '/', '\\'); // cmd.exe reads a forward slash in its own path as the start of a switch.
#endif

    std::vector<const char *> commandLine = {program.c_str()};
    for (const std::string &arg : args)
        commandLine.push_back(arg.c_str());
    commandLine.push_back(nullptr);

    subprocess_s process;
    int options = subprocess_option_inherit_environment | subprocess_option_no_window | subprocess_option_enable_async |
                  subprocess_option_enable_async_no_wait;
    if (int error = subprocess_create(commandLine.data(), options, &process))
        throw Exception("{}: couldn't start the process, subprocess.h error {}", displayString, error);
    MM_AT_SCOPE_EXIT(subprocess_destroy(&process));

    fclose(process.stdin_file); // The child reads end of file right away.
    process.stdin_file = nullptr;

    ProcessResult result;
    auto drain = [&] {
        char buffer[4096];
        while (unsigned size = subprocess_read_stdout(&process, buffer, sizeof(buffer)))
            result.stdOut.append(buffer, size);
        while (unsigned size = subprocess_read_stderr(&process, buffer, sizeof(buffer)))
            result.stdErr.append(buffer, size);
    };

    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (subprocess_alive(&process)) {
        drain();
        if (timeout != timeout.zero() && std::chrono::steady_clock::now() >= deadline) {
            subprocess_terminate(&process);
            result.timedOut = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (subprocess_join(&process, &result.exitCode) != 0)
        throw Exception("{}: couldn't get the exit code", displayString);
    drain();
    return result;
}

#endif // __ANDROID__
