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
    return normalPath.isAccessible() && existsOf(this, normalPath);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(Path(path));
}

FileStat FileSystem::stat(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    return normalPath.isAccessible() ? statOf(this, normalPath) : FileStat();
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
    lsOf(this, normalPath, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(Path(path));
}

Blob FileSystem::read(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return readOf(this, normalPath);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(Path(path), data);
}

void FileSystem::write(PathView path, const Blob &data) {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    writeOf(this, normalPath, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(Path(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return openForReadingOf(this, normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(Path(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return openForWritingOf(this, normalPath);
}

bool FileSystem::remove(std::string_view path) {
    return remove(Path(path));
}

bool FileSystem::remove(PathView path) {
    NormalizedFileSystemPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return removeOf(this, normalPath);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(Path(path));
}

std::string FileSystem::displayPath(PathView path) const {
    return displayPathOf(this, NormalizedFileSystemPath(path)); // Never refuses, raising an exception formats the path through here.
}

bool FileSystem::existsOf(const FileSystem *fs, PathView path) {
    return path.isEmpty() || fs->_exists(path); // Root always exists.
}

FileStat FileSystem::statOf(const FileSystem *fs, PathView path) {
    return path.isEmpty() ? FileStat(FILE_DIRECTORY, 0) : fs->_stat(path);
}

void FileSystem::lsOf(const FileSystem *fs, PathView path, std::vector<DirectoryEntry> *entries) {
    entries->clear();
    fs->_ls(path, entries);
}

Blob FileSystem::readOf(const FileSystem *fs, PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_READ_FAILED_PATH_IS_DIR, path);
    return fs->_read(path);
}

void FileSystem::writeOf(FileSystem *fs, PathView path, const Blob &data) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_WRITE_FAILED_PATH_IS_DIR, path);
    fs->_write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReadingOf(const FileSystem *fs, PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_READ_FAILED_PATH_IS_DIR, path);
    return fs->_openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWritingOf(FileSystem *fs, PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_WRITE_FAILED_PATH_IS_DIR, path);
    return fs->_openForWriting(path);
}

bool FileSystem::removeOf(FileSystem *fs, PathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    return fs->_remove(path);
}

std::string FileSystem::displayPathOf(const FileSystem *fs, PathView path) {
    return fs->_displayPath(path);
}
