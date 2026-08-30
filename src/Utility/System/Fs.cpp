#include "Fs.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "Utility/Exception.h"

static std::filesystem::path toStdPath(const Path &path) {
    // The native() string is charset-safe to construct a path from - it's wchar_t on Windows.
    return std::filesystem::path(path.native());
}

static std::string toPathString(const std::filesystem::path &path) {
    return Path::fromNative(path.native()).string();
}

bool fs::exists(const Path &path) {
    std::error_code ec;
    return std::filesystem::exists(toStdPath(path), ec); // Returns false on error.
}

FileStat fs::stat(const Path &path) {
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

std::vector<DirectoryEntry> fs::ls(const Path &path) {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void fs::ls(const Path &path, std::vector<DirectoryEntry> *entries) {
    // We just ignore all errors here. The errors we'll get are most likely permissions-related, and we're ignoring
    // them in `stat` and `exists` too.
    // Driven by hand rather than with a range-for, because only the constructor takes an error_code - operator++ is
    // the throwing overload, and a mid-iteration EIO on a network mount would escape this function. The walk gets
    // its own error_code, so that a per-entry failure below skips that entry instead of truncating the listing.
    std::error_code walkEc;
    std::error_code ec;
    std::filesystem::directory_iterator pos(toStdPath(path), walkEc);
    std::filesystem::directory_iterator end;
    for (; !walkEc && pos != end; pos.increment(walkEc)) {
        const std::filesystem::directory_entry &entry = *pos;

        // Unfortunately, std::filesystem is broken here. We can get a directory_entry for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        // The roundtrip through Path is a WTF-8 conversion on Windows.
        entries->emplace_back(toPathString(entry.path().filename()), isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }
}

bool fs::remove(const Path &path) {
    return std::filesystem::remove_all(toStdPath(path)) > 0;
}

void fs::mkdirs(const Path &path) {
    if (path.isEmpty())
        return; // The current directory, which necessarily exists - create_directories("") would throw.

    std::filesystem::create_directories(toStdPath(path));
}

Path fs::cwd() {
    std::error_code ec;
    std::filesystem::path result = std::filesystem::current_path(ec);
    if (ec)
        throw Exception("Couldn't get the current directory: {}", ec.message());
    return Path::fromNative(result.native());
}

Path fs::absolute(const Path &path) {
    // An explicit isEmpty() check b/c libstdc++ std::filesystem::absolute chokes on an empty path.
    std::error_code ec;
    std::filesystem::path result =
        path.isEmpty() ? std::filesystem::current_path(ec) : std::filesystem::absolute(toStdPath(path.normalized()), ec);
    if (ec)
        throw Exception("Couldn't resolve native path '{}': {}", path, ec.message());
    return Path::fromNative(result.native());
}
