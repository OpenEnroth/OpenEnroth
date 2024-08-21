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

bool DirectoryFileSystem::_exists(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    return std::filesystem::exists(makeBasePath(path));
}

FileStat DirectoryFileSystem::_stat(const FileSystemPath &path) const {
    assert(!path.isEmpty());

    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::file_status status = std::filesystem::status(basePath);
    bool isRegular = std::filesystem::is_regular_file(status);
    bool isDirectory = !isRegular && std::filesystem::is_directory(status);
    if (!isRegular && !isDirectory)
        return {}; // Non-files and non-folders are not visible through this interface.

    FileStat result;
    result.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    result.size = isRegular ? std::filesystem::file_size(basePath) : 0;
    return result;
}

void DirectoryFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    std::error_code ec;
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(makeBasePath(path), ec)) {
        bool isRegular = entry.is_regular_file();
        bool isDirectory = !isRegular && entry.is_directory();
        if (!isRegular && !isDirectory)
            continue;

        std::string name = entry.path().filename().string();
        if (name.find('\\') != std::string::npos)
            continue; // Files with '\\' in filename are not observable through this interface. Don't be a retard.

        DirectoryEntry &resultEntry = entries->emplace_back();
        resultEntry.name = std::move(name);
        resultEntry.type = isRegular ? FILE_REGULAR : FILE_DIRECTORY;
    }

    if (ec) {
        if (path.isEmpty()) {
            return; // Always pretend the root exists & is accessible.
        } else {
            throw std::system_error(ec, path.string());
        }
    }
}

Blob DirectoryFileSystem::_read(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path).generic_string());
}

void DirectoryFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    FileOutputStream stream(basePath.generic_string());
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> DirectoryFileSystem::_openForReading(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(makeBasePath(path).generic_string());
}

std::unique_ptr<OutputStream> DirectoryFileSystem::_openForWriting(const FileSystemPath &path) {
    assert(!path.isEmpty());
    std::filesystem::path basePath = makeBasePath(path);
    std::filesystem::create_directories(basePath.parent_path());
    return std::make_unique<FileOutputStream>(basePath.generic_string());
}

void DirectoryFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    std::filesystem::path srcBasePath = makeBasePath(srcPath);
    std::filesystem::path dstBasePath = makeBasePath(dstPath);
    if (std::filesystem::is_directory(dstBasePath))
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);

    // This call will copy the file if POSIX rename() fails, so if it throws then we can't really do anything either.
    std::filesystem::rename(srcBasePath, dstBasePath);
}

bool DirectoryFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());
    return std::filesystem::remove_all(makeBasePath(path)) > 0;
}

std::string DirectoryFileSystem::_displayPath(const FileSystemPath &path) const {
    return makeBasePath(path).generic_string();
}

std::filesystem::path DirectoryFileSystem::makeBasePath(const FileSystemPath &path) const {
    return _root / path.string();
}
