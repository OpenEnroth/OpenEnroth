#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "NormalizedFileSystemPath.h"

#include "FileSystemException.h"

bool FileSystem::exists(std::string_view path) const {
    return exists(Path(path));
}

bool FileSystem::exists(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        return true; // Root always exists.
    if (!normalPath.isAccessible())
        return false;
    return _exists(normalPath);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(Path(path));
}

FileStat FileSystem::stat(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (!normalPath.isAccessible())
        return FileStat();
    return _stat(normalPath);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(Path(path));
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(Path(path), entries);
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(normalPath, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(Path(path));
}

Blob FileSystem::read(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(normalPath);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(Path(path), data);
}

void FileSystem::write(PathView path, const Blob &data) {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(normalPath, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(Path(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(Path(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(normalPath);
}

bool FileSystem::remove(std::string_view path) {
    return remove(Path(path));
}

bool FileSystem::remove(PathView path) {
    NormalizedFileSystemPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(normalPath);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(Path(path));
}

std::string FileSystem::displayPath(PathView path) const {
    return _displayPath(NormalizedFileSystemPath(path)); // Never refuses, raising an exception formats the path through here.
}
