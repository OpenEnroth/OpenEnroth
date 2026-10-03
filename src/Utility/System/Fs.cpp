#include "Fs.h"

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>

#include "Utility/Exception.h"

[[noreturn]] static void throwError(std::string_view action, const NativePath &path, std::error_code ec) {
    throw Exception("Couldn't {} '{}': {}", action, path.displayString(), ec.message());
}

static void checkNotEmpty(std::string_view action, const NativePath &path) {
    if (path.isEmpty())
        throwError(action, path, std::make_error_code(std::errc::invalid_argument));
}

static std::filesystem::path toStdPath(const NativePath &path) {
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
        return {};

    std::int64_t size = 0;
    if (isRegular) {
        size = entry.file_size(ec); // Cached in the entry on Windows, so no second syscall there.
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
    checkNotEmpty("list", path);

    std::error_code walkEc;
    std::filesystem::directory_iterator pos(toStdPath(path), walkEc);
    std::filesystem::directory_iterator end;
    if (walkEc) {
        if (stat(path).type == FILE_DIRECTORY)
            return; // Exists but can't be opened, e.g. no permissions.
        throwError("list", path, walkEc);
    }

    // Errors past this point are ignored. They're most likely permissions-related, and `stat` and `exists` ignore
    // them too. `operator++` is the throwing overload, so the loop calls `increment` with an `error_code`.
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

        entries->emplace_back(fromStdPath(entry.path().filename()).str(),
                              isRegular ? FILE_REGULAR : FILE_DIRECTORY);
    }
}

bool fs::remove(const NativePath &path) {
    if (path.isEmpty())
        return false;

    std::error_code ec;
    std::uintmax_t removed = std::filesystem::remove_all(toStdPath(path), ec);
    if (ec)
        throwError("remove", path, ec);
    return removed > 0;
}

void fs::mkdirs(const NativePath &path) {
    checkNotEmpty("create", path);

    std::error_code ec;
    std::filesystem::create_directories(toStdPath(path), ec);
    if (ec)
        throwError("create", path, ec);
}

NativePath fs::cwd() {
    std::error_code ec;
    std::filesystem::path result = std::filesystem::current_path(ec);
    if (ec)
        throw Exception("Couldn't get the current directory: {}", ec.message());
    return fromStdPath(result);
}

NativePath fs::tmp() {
    std::error_code ec;
    std::filesystem::path result = std::filesystem::temp_directory_path(ec);
    if (ec)
        throw Exception("Couldn't get the temp directory: {}", ec.message());
    return fromStdPath(result);
}

NativePath fs::absolute(const NativePath &path) {
    checkNotEmpty("resolve", path);

    std::error_code ec;
    std::filesystem::path result = std::filesystem::absolute(toStdPath(path), ec);
    if (ec)
        throwError("resolve", path, ec);
    return fromStdPath(result);
}
