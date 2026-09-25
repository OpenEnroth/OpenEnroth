#include "Process.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#ifndef __ANDROID__
#   include <subprocess.h>
#endif

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"
#include "Utility/String/Encoding.h"

#ifdef __ANDROID__

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &, std::chrono::milliseconds) {
    throw Exception("{}: can't start a process on Android", path.displayString()); // posix_spawn needs API level 28.
}

#else // __ANDROID__

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout) {
    std::string displayString = path.displayString();

    std::filesystem::path nativePath = path.toStdPath();
    nativePath.make_preferred(); // cmd.exe reads a forward slash in its own path as the start of a switch.
#ifdef _WINDOWS
    std::string program = txt::wideToWtf8(nativePath.native());
#else
    std::string program = nativePath.native();
#endif

    std::vector<const char *> commandLine = {program.c_str()};
    for (const std::string &arg : args)
        commandLine.push_back(arg.c_str());
    commandLine.push_back(nullptr);

    subprocess_s process;
    int options = subprocess_option_combined_stdout_stderr | subprocess_option_inherit_environment | subprocess_option_no_window |
                  subprocess_option_enable_async | subprocess_option_enable_async_no_wait;
    if (int error = subprocess_create(commandLine.data(), options, &process))
        throw Exception("{}: couldn't start the process, subprocess.h error {}", displayString, error);
    MM_AT_SCOPE_EXIT(subprocess_destroy(&process));

    fclose(process.stdin_file); // The child reads end of file right away.
    process.stdin_file = nullptr;

    ProcessResult result;
    auto drain = [&] {
        char buffer[4096];
        while (unsigned size = subprocess_read_stdout(&process, buffer, sizeof(buffer)))
            result.output.append(buffer, size);
    };

    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (subprocess_alive(&process)) {
        drain();
        if (std::chrono::steady_clock::now() >= deadline) {
            subprocess_terminate(&process);
            subprocess_join(&process, nullptr);
            drain();
            throw Exception("{}: didn't finish in {}ms, output was:\n{}", displayString, timeout.count(), result.output);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    drain();

    if (subprocess_join(&process, &result.exitCode) != 0)
        throw Exception("{}: couldn't get the exit code", displayString);
    return result;
}

#endif // __ANDROID__
