#include "Os.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "Utility/Exception.h"

bool os::exists(const NativePath &path) {
    std::error_code ec;
    return std::filesystem::exists(path.toStdPath(), ec); // Returns false on error.
}

FileStat os::stat(const NativePath &path) {
    const std::filesystem::path &stdPath = path.toStdPath();

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
    ls(path, &result);
    return result;
}

void os::ls(const NativePath &path, std::vector<DirectoryEntry> *entries) {
    // We just ignore all errors here. The errors we'll get are most likely permissions-related, and we're ignoring
    // them in `stat` and `exists` too.
    std::error_code ec;
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(path.toStdPath(), ec)) {
        // Unfortunately, std::filesystem is broken here. We can get a directory_entry for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        // The roundtrip through NativePath is a WTF-8 conversion on Windows.
        entries->emplace_back(NativePath::fromStdPath(entry.path().filename()).toWtf8(),
                              isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }
}

bool os::remove(const NativePath &path) {
    return std::filesystem::remove_all(path.toStdPath()) > 0;
}

void os::mkdirs(const NativePath &path) {
    std::filesystem::create_directories(path.toStdPath());
}

NativePath os::cwd() {
    return NativePath::fromStdPath(std::filesystem::current_path());
}

NativePath os::absolute(const NativePath &path) {
    // An explicit isEmpty() check b/c libstdc++ std::filesystem::absolute chokes on an empty path.
    std::error_code ec;
    std::filesystem::path result =
        path.isEmpty() ? std::filesystem::current_path(ec) : std::filesystem::absolute(path.toStdPath(), ec);
    if (ec)
        throw Exception("Couldn't resolve native path '{}': {}", path, ec.message());
    return NativePath::fromStdPath(std::move(result));
}
