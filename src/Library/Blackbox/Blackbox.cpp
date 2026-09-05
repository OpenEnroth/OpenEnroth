#include "Blackbox.h"

#include <cassert>
#include <cstdint>
#include <ctime>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#include <fcntl.h> // NOLINT: not a C++ system header.
#ifdef _WINDOWS
#   include <io.h> // NOLINT: not a C++ system header.
#   include <process.h> // NOLINT: not a C++ system header.
#   include <sys/stat.h> // NOLINT: not a C++ system header.
#else
#   include <unistd.h> // NOLINT: not a C++ system header.
#endif

#include "Library/BuildInfo/BuildInfo.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/String/Format.h"

// Not members, because the crash callback is a plain function pointer. No synchronization either - the
// constructor writes both before any other thread is spawned, and everything after that only reads.
static int crashLogFd = -1;
static CrashCallback chainedCallback = &printCrashChunk;

static void blackboxCrashCallback(std::string_view text, bool final) {
    // File first. The callback chained to may never return from the final chunk - a crash dialog, a console
    // wait - or die trying to show it, and the trace has to be on disk by then.
    if (crashLogFd >= 0)
        writeCrashChunk(crashLogFd, text);
    chainedCallback(text, final);
}

static std::string localTimestamp() {
    return fmt::format("{:%Y-%m-%dT%H:%M:%S}", fmt::localtime(std::time(nullptr)));
}

Blackbox::Blackbox(const NativePath &path) {
    static bool constructed = false;
    assert(!constructed); // The descriptor and the callback are process-wide, a second instance would fight the first over both.
    constructed = true;

    // Errors are swallowed throughout - the process has better things to do than complain about its crash log,
    // and without a file the previous callback simply stays in charge.
    std::error_code error;
    const std::filesystem::path &stdPath = path.toStdPath();
    std::filesystem::create_directories(stdPath.parent_path(), error);

    std::uintmax_t size = std::filesystem::file_size(stdPath, error);
    if (!error && size > 1024 * 1024) {
        std::filesystem::path oldPath = stdPath;
        oldPath += ".old";
        std::filesystem::rename(stdPath, oldPath, error); // Replaces the previous .old.
    }

#ifdef _WINDOWS
    int fd = _wopen(stdPath.c_str(), _O_WRONLY | _O_APPEND | _O_CREAT | _O_BINARY | _O_NOINHERIT, _S_IREAD | _S_IWRITE);
    int pid = _getpid();
#else
    int fd = open(stdPath.c_str(), O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
    int pid = getpid();
#endif
    if (fd < 0)
        return;
    crashLogFd = fd;

    writeCrashChunk(fd, fmt::format("--- started {} pid={} revision={} ---", localTimestamp(), pid, gitRevision()));

    // Registering comes before learning what was registered, so for a few instructions a crash would chain to
    // the default printer rather than to the callback inherited here. Stderr and the file still get written,
    // at worst a console wait is skipped.
    CrashCallback previous = initStackTraceOnCrash(&blackboxCrashCallback);
    if (previous != &blackboxCrashCallback)
        chainedCallback = previous; // A second instance would otherwise chain the callback to itself and recurse until the signal stack is gone.
}

bool Blackbox::isLogging() const {
    return crashLogFd >= 0;
}

Blackbox::~Blackbox() {
    if (crashLogFd < 0)
        return;

    // Runs during unwinding too, and that's not a clean exit.
    const char *how = std::uncaught_exceptions() > 0 ? "exiting with exception" : "clean exit";
    try {
        writeCrashChunk(crashLogFd, fmt::format("--- {} {} ---", how, localTimestamp()));
    } catch (...) {
        // Formatting allocates, and a destructor is noexcept. Losing the exit line beats terminating on the way out.
    }
}
