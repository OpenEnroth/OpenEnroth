#include "Fs.h"

#include <cstdint>
#include <filesystem>
#include <system_error>
#include <vector>

static std::filesystem::path toStdPath(const NativePath &path) {
    if (path.isEmpty())
        throw std::filesystem::filesystem_error("Empty path", std::make_error_code(std::errc::invalid_argument));
    return std::filesystem::path(path.native()); // A wchar_t string on Windows, so no C locale conversion.
}

static NativePath fromStdPath(const std::filesystem::path &path) {
    return NativePath::fromNative(path.native());
}

bool fs::exists(const NativePath &path) {
    if (path.isEmpty())
        return false;

    std::error_code ec;
    return std::filesystem::exists(toStdPath(path), ec); // Returns false on error.
}

FileStat fs::stat(const NativePath &path) {
    if (path.isEmpty())
        return {};

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

std::vector<DirectoryEntry> fs::ls(const NativePath &path) {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void fs::ls(const NativePath &path, std::vector<DirectoryEntry> *entries) {
    std::filesystem::directory_iterator pos(toStdPath(path));
    std::filesystem::directory_iterator end;

    // Errors past this point are ignored. They're most likely permissions-related, and `stat` and `exists` ignore
    // them too. `operator++` is the throwing overload, so the loop calls `increment` with an `error_code`.
    std::error_code walkEc;
    std::error_code ec;
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

        entries->emplace_back(fromStdPath(entry.path().filename()).toWtf8(),
                              isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }
}

bool fs::remove(const NativePath &path) {
    if (path.isEmpty())
        return false;

    return std::filesystem::remove_all(toStdPath(path)) > 0;
}

void fs::mkdirs(const NativePath &path) {
    std::filesystem::create_directories(toStdPath(path));
}

NativePath fs::cwd() {
    return fromStdPath(std::filesystem::current_path());
}

NativePath fs::absolute(const NativePath &path) {
    return fromStdPath(std::filesystem::absolute(toStdPath(path)));
}

NativePath fs::tempDir() {
    return fromStdPath(std::filesystem::temp_directory_path());
}
