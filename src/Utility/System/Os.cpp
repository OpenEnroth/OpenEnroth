#include "Os.h"

#include <cstdint>
#include <filesystem>
#include <system_error>
#include <utility>
#include <vector>

#include "Utility/Exception.h"

static std::filesystem::path toStdPath(const NativePath &path) {
    // The native() string is charset-safe to construct a path from - it's wchar_t on Windows.
    return std::filesystem::path(path.native());
}

static std::string toWtf8(const std::filesystem::path &path) {
    return NativePath::fromNative(path.native()).toWtf8();
}

bool os::exists(const NativePath &path) {
    std::error_code ec;
    return std::filesystem::exists(toStdPath(path), ec); // Returns false on error.
}

FileStat os::stat(const NativePath &path) {
    std::filesystem::path stdPath = toStdPath(path);

    std::error_code ec;
    std::filesystem::directory_entry entry(stdPath, ec);
    bool isRegular = entry.is_regular_file(ec);
    bool isDirectory = !isRegular && entry.is_directory(ec);
    if (!isRegular && !isDirectory)
        return {}; // Return an empty stat on error or if it's not a file / directory.

    std::int64_t size = 0;
    if (isRegular) {
        size = std::filesystem::file_size(stdPath, ec);
        if (ec)
            return {};
    }

    return FileStat(isRegular ? FILE_REGULAR : FILE_DIRECTORY, size);
}

std::vector<DirectoryEntry> os::ls(const NativePath &path) {
    std::vector<DirectoryEntry> result;

    // We just ignore all errors here. The errors we'll get are most likely permissions-related, and we're ignoring
    // them in `stat` and `exists` too.
    std::error_code ec;
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(toStdPath(path), ec)) {
        // Unfortunately, std::filesystem is retarded. We can get a directory_entry here for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        result.emplace_back(toWtf8(entry.path().filename()), isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }

    return result;
}

bool os::remove(const NativePath &path) {
    return std::filesystem::remove_all(toStdPath(path)) > 0;
}

void os::mkdirs(const NativePath &path) {
    std::filesystem::create_directories(toStdPath(path));
}

NativePath os::cwd() {
    return NativePath::fromNative(std::filesystem::current_path().native());
}

NativePath os::absolute(const NativePath &path) {
    // An explicit isEmpty() check b/c libstdc++ std::filesystem::absolute chokes on an empty path.
    std::error_code ec;
    std::filesystem::path result =
        path.isEmpty() ? std::filesystem::current_path(ec) : std::filesystem::absolute(toStdPath(path), ec);
    if (ec)
        throw Exception("Couldn't resolve native path '{}': {}", path, ec.message());
    return NativePath::fromNative(result.native());
}
