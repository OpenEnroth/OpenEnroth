#include "Process.h"

#include <algorithm>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#ifdef _WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <fcntl.h>
#   include <poll.h>
#   include <signal.h> // NOLINT: not a C system header.
#   include <spawn.h>
#   include <sys/wait.h>
#   include <unistd.h>

#   include <cerrno>
#endif

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"
#include "Utility/String/Encoding.h"

constexpr std::chrono::milliseconds pollInterval(10);

std::string detail::windowsCommandLine(std::string_view program, const std::vector<std::string> &args) {
    std::string result;

    auto append = [&](std::string_view arg) {
        if (!result.empty())
            result += ' ';

        if (!arg.empty() && arg.find_first_of(" \t\n\v\"") == std::string_view::npos) {
            result += arg;
            return;
        }

        result += '"';
        size_t backslashes = 0;
        for (char c : arg) {
            if (c == '\\') {
                backslashes++;
                continue;
            }

            // Backslashes are literal unless a quote follows, then each one needs escaping, and so does the quote.
            result.append(c == '"' ? backslashes * 2 + 1 : backslashes, '\\');
            result += c;
            backslashes = 0;
        }
        result.append(backslashes * 2, '\\'); // These are followed by the closing quote.
        result += '"';
    };

    append(program);
    for (const std::string &arg : args)
        append(arg);
    return result;
}

#ifdef _WINDOWS

[[noreturn]] static void throwFromLastError(std::string_view arg) {
    throw Exception("{}: {}", arg, std::system_category().message(GetLastError()));
}

static void drain(HANDLE pipe, std::string *output) {
    DWORD available = 0;
    while (PeekNamedPipe(pipe, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
        char buffer[4096];
        DWORD size = 0;
        if (!ReadFile(pipe, buffer, std::min<DWORD>(available, sizeof(buffer)), &size, nullptr) || size == 0)
            return;
        output->append(buffer, size);
    }
}

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout) {
    std::string displayString = path.displayString();

    SECURITY_ATTRIBUTES inheritable = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

    HANDLE readEnd = nullptr, writeEnd = nullptr;
    if (!CreatePipe(&readEnd, &writeEnd, &inheritable, 0))
        throwFromLastError(displayString);
    MM_AT_SCOPE_EXIT(CloseHandle(readEnd));
    auto writeEndGuard = ScopeGuard([&] { if (writeEnd) CloseHandle(writeEnd); });
    if (!SetHandleInformation(readEnd, HANDLE_FLAG_INHERIT, 0))
        throwFromLastError(displayString);

    HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &inheritable, OPEN_EXISTING, 0, nullptr);
    if (nul == INVALID_HANDLE_VALUE)
        throwFromLastError(displayString);
    MM_AT_SCOPE_EXIT(CloseHandle(nul));

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = nul;
    startupInfo.hStdOutput = writeEnd;
    startupInfo.hStdError = writeEnd;

    std::wstring commandLine = wtf8ToWide(detail::windowsCommandLine(path.toWtf8(), args)); // CreateProcessW writes into it.
    PROCESS_INFORMATION processInfo = {};
    if (!CreateProcessW(path.toStdPath().c_str(), commandLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
        throwFromLastError(displayString);
    CloseHandle(processInfo.hThread);
    MM_AT_SCOPE_EXIT(CloseHandle(processInfo.hProcess));

    CloseHandle(writeEnd); // The child has its own copy now.
    writeEnd = nullptr;

    ProcessResult result;
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        DWORD waitResult = WaitForSingleObject(processInfo.hProcess, static_cast<DWORD>(pollInterval.count()));
        drain(readEnd, &result.output);
        if (waitResult == WAIT_OBJECT_0)
            break;
        if (waitResult == WAIT_FAILED)
            throwFromLastError(displayString);

        if (std::chrono::steady_clock::now() >= deadline) {
            TerminateProcess(processInfo.hProcess, 1);
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            drain(readEnd, &result.output);
            throw Exception("{}: didn't finish in {}ms, output was:\n{}", displayString, timeout.count(), result.output);
        }
    }

    DWORD exitCode = 0;
    if (!GetExitCodeProcess(processInfo.hProcess, &exitCode))
        throwFromLastError(displayString);
    result.exitCode = static_cast<int>(exitCode);
    return result;
}

#else // _WINDOWS

extern char **environ;

/**
 * @param fd                            Non-blocking descriptor to read from.
 * @param[out] output                   Where to append what was read.
 * @return                              Whether there might be more to read later, `false` on end of file.
 */
static bool drain(int fd, std::string *output) {
    while (true) {
        char buffer[4096];
        ssize_t size = read(fd, buffer, sizeof(buffer));
        if (size > 0) {
            output->append(buffer, size);
        } else if (size == 0) {
            return false;
        } else if (errno != EINTR) {
            return errno == EAGAIN || errno == EWOULDBLOCK;
        }
    }
}

ProcessResult runProcess(const NativePath &path, const std::vector<std::string> &args, std::chrono::milliseconds timeout) {
    std::string displayString = path.displayString();

    int fds[2];
    if (pipe(fds) != 0)
        Exception::throwFromErrno(displayString);
    int readEnd = fds[0], writeEnd = fds[1];
    MM_AT_SCOPE_EXIT(close(readEnd));
    auto writeEndGuard = ScopeGuard([&] { if (writeEnd != -1) close(writeEnd); });
    fcntl(readEnd, F_SETFD, FD_CLOEXEC);
    fcntl(writeEnd, F_SETFD, FD_CLOEXEC); // The copies that posix_spawn makes at 1 and 2 don't inherit the flag.
    fcntl(readEnd, F_SETFL, O_NONBLOCK);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    MM_AT_SCOPE_EXIT(posix_spawn_file_actions_destroy(&actions));
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    posix_spawn_file_actions_adddup2(&actions, writeEnd, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, writeEnd, STDERR_FILENO);

    std::string program = path.toStdPath().string();
    std::vector<std::string> argsCopy = args; // posix_spawn takes non-const pointers.
    std::vector<char *> argv = {program.data()};
    for (std::string &arg : argsCopy)
        argv.push_back(arg.data());
    argv.push_back(nullptr);

    pid_t pid = 0;
    if (int error = posix_spawn(&pid, program.c_str(), &actions, nullptr, argv.data(), environ))
        Exception::throwFromErrc(static_cast<std::errc>(error), displayString);

    close(writeEnd); // The child has its own copies now.
    writeEnd = -1;

    ProcessResult result;
    int status = 0;
    bool readable = true;
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        if (readable) {
            pollfd request = {readEnd, POLLIN, 0};
            poll(&request, 1, pollInterval.count());
            readable = drain(readEnd, &result.output);
        } else {
            std::this_thread::sleep_for(pollInterval); // The child has closed its output, and poll would return at once.
        }

        pid_t waitResult = waitpid(pid, &status, WNOHANG);
        if (waitResult == pid)
            break;
        if (waitResult == -1 && errno != EINTR)
            Exception::throwFromErrno(displayString);

        if (std::chrono::steady_clock::now() >= deadline) {
            kill(pid, SIGKILL);
            while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {}
            drain(readEnd, &result.output);
            throw Exception("{}: didn't finish in {}ms, output was:\n{}", displayString, timeout.count(), result.output);
        }
    }
    drain(readEnd, &result.output);

    result.exitCode = WIFSIGNALED(status) ? 128 + WTERMSIG(status) : WEXITSTATUS(status);
    return result;
}

#endif // _WINDOWS
