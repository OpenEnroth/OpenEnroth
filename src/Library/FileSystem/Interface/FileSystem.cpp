#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "FileSystemPath.h"

#include "FileSystemException.h"

/**
 * @param path                          Path as a caller passed it.
 * @param error                         Error to raise if `path` doesn't start with a separator.
 * @return                              `path` in normal form.
 */
static FileSystemPath rootedPath(std::string_view path, FileSystemError error) {
    if (!FileSystemPath::isRooted(path))
        throw FileSystemException(error, path);
    return FileSystemPath::fromRooted(path);
}

bool FileSystem::exists(std::string_view path) const {
    return FileSystemPath::isRooted(path) && exists(FileSystemPath::fromRooted(path));
}

bool FileSystem::exists(FileSystemPathView path) const {
    if (path.isEmpty())
        return true; // Root always exists.
    if (path.isEscaping())
        return false; // Escaping paths are not accessible through this interface.
    return _exists(path);
}

FileStat FileSystem::stat(std::string_view path) const {
    return FileSystemPath::isRooted(path) ? stat(FileSystemPath::fromRooted(path)) : FileStat();
}

FileStat FileSystem::stat(FileSystemPathView path) const {
    if (path.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (path.isEscaping())
        return FileStat();
    return _stat(path);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(rootedPath(path, FS_LS_FAILED_PATH_NOT_ACCESSIBLE));
}

std::vector<DirectoryEntry> FileSystem::ls(FileSystemPathView path) const {
    if (path.isEscaping())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    _ls(path, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(rootedPath(path, FS_LS_FAILED_PATH_NOT_ACCESSIBLE), entries);
}

void FileSystem::ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEscaping())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(path, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(rootedPath(path, FS_READ_FAILED_PATH_NOT_ACCESSIBLE));
}

Blob FileSystem::read(FileSystemPathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(path);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(rootedPath(path, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE), data);
}

void FileSystem::write(FileSystemPathView path, const Blob &data) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(rootedPath(path, FS_READ_FAILED_PATH_NOT_ACCESSIBLE));
}

std::unique_ptr<InputStream> FileSystem::openForReading(FileSystemPathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(rootedPath(path, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(FileSystemPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(path);
}

bool FileSystem::remove(std::string_view path) {
    return remove(rootedPath(path, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE));
}

bool FileSystem::remove(FileSystemPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(path);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(FileSystemPath::isRooted(path) ? FileSystemPath::fromRooted(path) : FileSystemPath(path));
}

std::string FileSystem::displayPath(FileSystemPathView path) const {
    return _displayPath(path);
}
