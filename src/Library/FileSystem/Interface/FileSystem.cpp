#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "Utility/System/PathView.h"

#include "FileSystemException.h"

// A path that this file system can act on: relative, so that it names something under the root rather than a
// location of its own, and non-escaping, so that it doesn't climb out. Normal form is what makes the check
// meaningful - "a/../.." is escaping and "a/../b" isn't, and only normalizing tells them apart cheaply.
static bool isAccessible(PathView path) {
    return !path.isAbsolute() && !path.isEscaping();
}

// Every public entry point starts here. The private _ methods take the result and assume it, so this is the one
// place the invariant is established.
static Path normalizedFor(PathView path) {
    return Path(path).normalized();
}

bool FileSystem::exists(std::string_view path) const {
    return exists(Path(path));
}

bool FileSystem::exists(PathView path) const {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        return true; // Root always exists.
    if (!isAccessible(normalPath))
        return false; // Inaccessible paths behave as if they don't exist.
    return _exists(normalPath);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(Path(path));
}

FileStat FileSystem::stat(PathView path) const {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (!isAccessible(normalPath))
        return FileStat();
    return _stat(normalPath);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(Path(path));
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    Path normalPath = normalizedFor(path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    _ls(normalPath, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(Path(path), entries);
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    Path normalPath = normalizedFor(path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(normalPath, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(Path(path));
}

Blob FileSystem::read(PathView path) const {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(normalPath);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(Path(path), data);
}

void FileSystem::write(PathView path, const Blob &data) {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(normalPath, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(Path(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(Path(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(normalPath);
}

bool FileSystem::remove(std::string_view path) {
    return remove(Path(path));
}

bool FileSystem::remove(PathView path) {
    Path normalPath = normalizedFor(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(normalPath);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(Path(path));
}

std::string FileSystem::displayPath(PathView path) const {
    return _displayPath(path);
}
