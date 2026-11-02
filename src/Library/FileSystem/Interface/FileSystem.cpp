#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "FileSystemException.h"

bool FileSystem::exists(PathView path) const {
    return exists(NormalPath(path));
}

bool FileSystem::exists(NormalPathView path) const {
    return path.isAccessible() && (path.isRoot() || _exists(path)); // Root always exists.
}

FileStat FileSystem::stat(PathView path) const {
    return stat(NormalPath(path));
}

FileStat FileSystem::stat(NormalPathView path) const {
    if (!path.isAccessible())
        return {};
    return path.isRoot() ? FileStat(FILE_DIRECTORY, 0) : _stat(path);
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    return ls(NormalPath(path));
}

std::vector<DirectoryEntry> FileSystem::ls(NormalPathView path) const {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    ls(NormalPath(path), entries);
}

void FileSystem::ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(path, entries);
}

Blob FileSystem::read(PathView path) const {
    return read(NormalPath(path));
}

Blob FileSystem::read(NormalPathView path) const {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    if (path.isRoot())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    return _read(path);
}

void FileSystem::write(PathView path, const Blob &data) {
    write(NormalPath(path), data);
}

void FileSystem::write(NormalPathView path, const Blob &data) {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    if (path.isRoot())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    _write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    return openForReading(NormalPath(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(NormalPathView path) const {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    if (path.isRoot())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    return _openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    return openForWriting(NormalPath(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(NormalPathView path) {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    if (path.isRoot())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    return _openForWriting(path);
}

bool FileSystem::remove(PathView path) {
    return remove(NormalPath(path));
}

bool FileSystem::remove(NormalPathView path) {
    if (!path.isAccessible())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    if (path.isRoot())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    return _remove(path);
}

std::string FileSystem::displayPath(PathView path) const {
    return displayPath(NormalPath(path));
}

std::string FileSystem::displayPath(NormalPathView path) const {
    if (path.isAccessible())
        return _displayPath(path);

    std::string result = _displayPath(NormalPathView()); // The implementation never sees an inaccessible path.
    if (!detail::isPathSeparator(result.back()))
        result += '/';
    result += PathView(path).displayString();
    return result;
}
