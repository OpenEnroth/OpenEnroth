#include <chrono>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Exception.h"
#include "Utility/System/Process.h"

using namespace std::chrono_literals; // NOLINT

static ProcessResult runShell(std::string command, std::chrono::milliseconds timeout = 30s) {
#ifdef _WINDOWS
    return runProcess(NativePath::fromWtf8(std::getenv("ComSpec")), {"/c", std::move(command)}, timeout);
#else
    return runProcess("/bin/sh", {"-c", std::move(command)}, timeout);
#endif
}

UNIT_TEST(Process, ExitCode) {
    EXPECT_EQ(runShell("exit 0").exitCode, 0);
    EXPECT_EQ(runShell("exit 3").exitCode, 3);
}

UNIT_TEST(Process, Output) {
    ProcessResult result = runShell("echo hello");
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.output.starts_with("hello")) << result.output; // The line ending is platform-specific.
}

UNIT_TEST(Process, StandardErrorIsCaptured) {
    EXPECT_TRUE(runShell("echo oops 1>&2").output.starts_with("oops"));
}

UNIT_TEST(Process, LargeOutput) {
    // Way more than a pipe buffer holds, so this hangs if the output isn't read while the process is running.
#ifdef _WINDOWS
    ProcessResult result = runShell("for /L %i in (1,1,20000) do @echo 0123456789");
#else
    ProcessResult result = runShell("i=0; while [ $i -lt 20000 ]; do echo 0123456789; i=$((i+1)); done");
#endif
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_GE(result.output.size(), 20000 * 11);
}

UNIT_TEST(Process, Timeout) {
#ifdef _WINDOWS
    std::string command = "ping -n 6 127.0.0.1"; // There is no sleep, and timeout.exe refuses to run without a console.
#else
    std::string command = "sleep 5";
#endif
    auto start = std::chrono::steady_clock::now();
    EXPECT_THROW((void) runShell(command, 200ms), Exception);
    EXPECT_LT(std::chrono::steady_clock::now() - start, 10s);
}

UNIT_TEST(Process, MissingExecutable) {
    EXPECT_THROW((void) runProcess("no_such_executable_here", {}, 30s), Exception);
}

UNIT_TEST(Process, WindowsCommandLine) {
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {}), "a.exe");
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {"--flag", "value"}), "a.exe --flag value");
    EXPECT_EQ(detail::windowsCommandLine("C:/Program Files/a.exe", {""}), "\"C:/Program Files/a.exe\" \"\"");
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {"C:\\dir\\"}), "a.exe C:\\dir\\"); // No quotes, so backslashes are literal.
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {"C:\\my dir\\"}), "a.exe \"C:\\my dir\\\\\""); // Or they'd escape the closing quote.
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {"say \"hi\""}), "a.exe \"say \\\"hi\\\"\"");
    EXPECT_EQ(detail::windowsCommandLine("a.exe", {"a\\\"b"}), "a.exe \"a\\\\\\\"b\"");
}
