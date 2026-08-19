#include "Os.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

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

        // The roundtrip through NativePath is a WTF8 conversion.
        result.emplace_back(NativePath::fromStdPath(entry.path().filename()).toWtf8(),
                            isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }

    return result;
}

bool os::remove(const NativePath &path) {
    return std::filesystem::remove_all(path.toStdPath()) > 0;
}

void os::mkdirs(const NativePath &path) {
    std::filesystem::create_directories(path.toStdPath());
}
