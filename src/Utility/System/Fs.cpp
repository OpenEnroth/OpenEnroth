#include "Fs.h"

#include <cstdint>
#include <filesystem>
#include <system_error>
#include <vector>

bool fs::exists(const NativePath &path) {
    std::error_code ec;
    return std::filesystem::exists(path.toStdPath(), ec); // Returns false on error.
}

FileStat fs::stat(const NativePath &path) {
    std::filesystem::path stdPath = path.toStdPath();

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

std::vector<DirectoryEntry> fs::ls(const NativePath &path) {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void fs::ls(const NativePath &path, std::vector<DirectoryEntry> *entries) {
    // All errors are ignored. The ones we get here are most likely permissions-related, and `stat` and `exists`
    // ignore them too. Only the iterator's constructor and `increment` take an `error_code`, `operator++` throws.
    std::error_code walkEc;
    std::error_code ec;
    std::filesystem::directory_iterator pos(path.toStdPath(), walkEc);
    std::filesystem::directory_iterator end;
    for (; !walkEc && pos != end; pos.increment(walkEc)) {
        const std::filesystem::directory_entry &entry = *pos;

        // We can get a directory_entry for a dir that we don't have permissions for, and which won't be stat-able.
        // Then entry.is_directory() returns true while std::filesystem::exists(entry.path()) throws.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        entries->emplace_back(NativePath::fromStdPath(entry.path().filename()).toWtf8(),
                              isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }
}

bool fs::remove(const NativePath &path) {
    return std::filesystem::remove_all(path.toStdPath()) > 0;
}

void fs::mkdirs(const NativePath &path) {
    if (path.isEmpty())
        return; // The current directory, create_directories("") throws.

    std::filesystem::create_directories(path.toStdPath());
}
