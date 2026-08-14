#include "NativeFileSystem.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Exception.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"

NativeFileSystem::NativeFileSystem(const NativePath &root) {
    _root = root.absolute();
}

NativeFileSystem::~NativeFileSystem() = default;

std::pair<std::unique_ptr<NativeFileSystem>, FileSystemPath> NativeFileSystem::fromNativePath(const NativePath &path) {
    // An explicit isEmpty() check b/c libstdc++ std::filesystem::absolute chokes on an empty path.
    std::error_code ec;
    std::filesystem::path absolutePath =
        path.isEmpty() ? std::filesystem::current_path(ec) : std::filesystem::absolute(path.toStdPath(), ec);
    if (ec)
        throw Exception("Couldn't resolve native path '{}': {}", path, ec.message());

    // lexically_normal collapses ".." even right after the root, so the tail can never be an escaping path.
    absolutePath = absolutePath.lexically_normal();

    // FileSystemPath normalizes - lexically_normal can leave a trailing '/' in there.
    return {std::make_unique<NativeFileSystem>(NativePath::fromStdPath(absolutePath.root_path())),
            FileSystemPath(NativePath::fromStdPath(absolutePath.relative_path()).toWtf8())};
}

bool NativeFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::error_code ec;
    return std::filesystem::exists(makeBasePath(path).toStdPath(), ec); // Returns false on error.
}

FileStat NativeFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::filesystem::path basePath = makeBasePath(path).toStdPath();

    std::error_code ec;
    std::filesystem::directory_entry entry(basePath, ec);
    bool isRegular = entry.is_regular_file(ec);
    bool isDirectory = !isRegular && entry.is_directory(ec);
    if (!isRegular && !isDirectory)
        return {}; // Return an empty stat on error or if it's not a file / directory.

    std::int64_t size = 0;
    if (isRegular) {
        size = std::filesystem::file_size(basePath, ec);
        if (ec)
            return {};
    }

    FileStat result;
    result.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    result.size = size;
    return result;
}

void NativeFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    std::filesystem::path basePath = makeBasePath(path).toStdPath();

    // Handle the known errors first.
    std::error_code ec;
    std::filesystem::directory_entry parent(basePath, ec);
    bool isParentRegular = parent.is_regular_file(ec);
    bool isParentDirectory = !isParentRegular && parent.is_directory(ec);
    if (path.isEmpty() && !isParentDirectory)
        return; // ls("") should always work.
    if (isParentRegular)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (!isParentDirectory)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    // Then we do the regular ls and just ignore all errors. The errors we'll get here are most likely
    // permissions-related, and we're ignoring them in stat() and exists().
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(basePath, ec)) {
        // Unfortunately, std::filesystem is retarded. We can get a directory_entry here for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        std::string name = NativePath::fromStdPath(entry.path().filename()).toWtf8(); // The roundtrip is a WTF8 conversion.
        if (name.find('\\') != std::string::npos)
            continue; // Files with '\\' in filename are not observable through this interface. Don't be a retard.

        DirectoryEntry &resultEntry = entries->emplace_back();
        resultEntry.name = std::move(name);
        resultEntry.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    }
}

Blob NativeFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path));
}

void NativeFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    NativePath basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.toStdPath().parent_path());
    FileOutputStream stream(basePath);
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> NativeFileSystem::_openForReading(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(makeBasePath(path));
}

std::unique_ptr<OutputStream> NativeFileSystem::_openForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());
    NativePath basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.toStdPath().parent_path());
    return std::make_unique<FileOutputStream>(basePath);
}

bool NativeFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return std::filesystem::remove_all(makeBasePath(path).toStdPath()) > 0;
}

std::string NativeFileSystem::_displayPath(FileSystemPathView path) const {
    return makeBasePath(path).displayString();
}

NativePath NativeFileSystem::makeBasePath(FileSystemPathView path) const {
    if (path.isEmpty())
        return _root; // `_root / ""` would add a trailing separator.

    return _root / NativePath::fromWtf8(path.string());
}
