#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "Utility/System/PathView.h"

#include "FileSystemException.h"

// A path in a file system is always relative to its root, so a leading separator is just noise - the class docs
// promise that "/foo" and "foo" name the same file. Stripping it before normalizing is also what keeps "/../.."
// escaping, rather than having it clamped to the root the way an absolute path would be.
static Path toRelativePath(std::string_view path) {
    while (path.starts_with('/'))
        path.remove_prefix(1);
    return Path(path);
}

// An escaping path points outside the root. An absolute one names a location of its own instead of something under
// the root - on Windows a path starting with a drive would otherwise replace the root outright once a backend
// composes the two, which is an escape.
static bool isAccessible(PathView path) {
    return !path.isEscaping() && !path.isAbsolute();
}

bool FileSystem::exists(std::string_view path) const {
    return exists(toRelativePath(path));
}

bool FileSystem::exists(PathView path) const {
    if (path.isEmpty())
        return true; // Root always exists.
    if (!isAccessible(path))
        return false; // Escaping paths are not accessible through this interface.
    return _exists(path);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(toRelativePath(path));
}

FileStat FileSystem::stat(PathView path) const {
    if (path.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (!isAccessible(path))
        return FileStat();
    return _stat(path);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(toRelativePath(path));
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    _ls(path, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(Path(path), entries);
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(path, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(toRelativePath(path));
}

Blob FileSystem::read(PathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(path);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(Path(path), data);
}

void FileSystem::write(PathView path, const Blob &data) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(toRelativePath(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(toRelativePath(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(path);
}

bool FileSystem::remove(std::string_view path) {
    return remove(toRelativePath(path));
}

bool FileSystem::remove(PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!isAccessible(path))
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(path);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(toRelativePath(path));
}

std::string FileSystem::displayPath(PathView path) const {
    return _displayPath(path);
}
