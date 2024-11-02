#include "DirectoryFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/UnicodeCrt.h"

DirectoryFileSystem::DirectoryFileSystem(std::string_view root) {
    assert(UnicodeCrt::isInitialized()); // Otherwise std::filesystem will choke on Unicode paths.

    // We need to explicitly check for empty() b/c libstdc++ std::filesystem::absolute chokes on empty path.
    _root = root.empty() ? std::filesystem::current_path() : std::filesystem::absolute(root).lexically_normal();
    _originalRoot = root;
}

DirectoryFileSystem::~DirectoryFileSystem() = default;

bool DirectoryFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::error_code ec;
    return std::filesystem::exists(makeBasePath(path), ec); // Returns false on error.
}

FileStat DirectoryFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());

    std::filesystem::path basePath = makeBasePath(path);

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
    return result;}

void DirectoryFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    std::filesystem::path basePath = makeBasePath(path);

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
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(makeBasePath(path), ec)) {
        // Unfortunately, std::filesystem is retarded. We can get a directory_entry here for a dir that we don't have
        // permissions for, and which won't be stat-able. Seriously, entry.is_directory() returns true while
        // std::filesystem::exists(entry.path()) just throws. So we need to check for that.
        if (!std::filesystem::exists(entry.path(), ec))
            continue;

        bool isRegular = entry.is_regular_file(ec);
        bool isDirectory = !isRegular && entry.is_directory(ec);
        if (!isRegular && !isDirectory)
            continue;

        std::string name = entry.path().filename().string();
        if (name.find('\\') != std::string::npos)
            continue; // Files with '\\' in filename are not observable through this interface. Don't be a retard.

        DirectoryEntry &resultEntry = entries->emplace_back();
        resultEntry.name = std::move(name);
        resultEntry.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    }
}

Blob DirectoryFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path).generic_string());
}

void DirectoryFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    FileOutputStream stream(basePath.generic_string());
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> DirectoryFileSystem::_openForReading(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(makeBasePath(path).generic_string());
}

std::unique_ptr<OutputStream> DirectoryFileSystem::_openForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    return std::make_unique<FileOutputStream>(basePath.generic_string());
}

void DirectoryFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    std::filesystem::path srcBasePath = makeBasePath(srcPath);
    std::filesystem::path dstBasePath = makeBasePath(dstPath);

    std::error_code ec;
    if (std::filesystem::is_directory(dstBasePath, ec))
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);

    // This call will copy the file if POSIX rename() fails, so if it throws then we can't really do anything either.
    std::filesystem::rename(srcBasePath, dstBasePath);
}

bool DirectoryFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return std::filesystem::remove_all(makeBasePath(path)) > 0;
}

std::string DirectoryFileSystem::_displayPath(FileSystemPathView path) const {
    return makeBasePath(path).generic_string();
}

std::filesystem::path DirectoryFileSystem::makeBasePath(FileSystemPathView path) const {
    return _root / path.string();
}
