#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "FileSystemException.h"

bool FileSystem::exists(std::string_view path) const {
    NormalPath normalPath(path);
    return normalPath.isAccessible() && existsIn(this, normalPath);
}

bool FileSystem::exists(PathView path) const {
    return exists(path.str());
}

FileStat FileSystem::stat(std::string_view path) const {
    NormalPath normalPath(path);
    return normalPath.isAccessible() ? statIn(this, normalPath) : FileStat();
}

FileStat FileSystem::stat(PathView path) const {
    return stat(path.str());
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    return ls(path.str());
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    lsIn(this, normalPath, entries);
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    ls(path.str(), entries);
}

Blob FileSystem::read(std::string_view path) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return readIn(this, normalPath);
}

Blob FileSystem::read(PathView path) const {
    return read(path.str());
}

void FileSystem::write(std::string_view path, const Blob &data) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    writeIn(this, normalPath, data);
}

void FileSystem::write(PathView path, const Blob &data) {
    write(path.str(), data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return openForReadingIn(this, normalPath);
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    return openForReading(path.str());
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return openForWritingIn(this, normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    return openForWriting(path.str());
}

bool FileSystem::remove(std::string_view path) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return removeIn(this, normalPath);
}

bool FileSystem::remove(PathView path) {
    return remove(path.str());
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPathIn(this, NormalPath(path)); // Never refuses, raising an exception formats the path through here.
}

std::string FileSystem::displayPath(PathView path) const {
    return displayPath(path.str());
}

bool FileSystem::existsIn(const FileSystem *fs, NormalPathView path) {
    return path.isEmpty() || fs->_exists(path); // Root always exists.
}

FileStat FileSystem::statIn(const FileSystem *fs, NormalPathView path) {
    return path.isEmpty() ? FileStat(FILE_DIRECTORY, 0) : fs->_stat(path);
}

void FileSystem::lsIn(const FileSystem *fs, NormalPathView path, std::vector<DirectoryEntry> *entries) {
    entries->clear();
    fs->_ls(path, entries);
}

Blob FileSystem::readIn(const FileSystem *fs, NormalPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_READ_FAILED_PATH_IS_DIR, path);
    return fs->_read(path);
}

void FileSystem::writeIn(FileSystem *fs, NormalPathView path, const Blob &data) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_WRITE_FAILED_PATH_IS_DIR, path);
    fs->_write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReadingIn(const FileSystem *fs, NormalPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_READ_FAILED_PATH_IS_DIR, path);
    return fs->_openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWritingIn(FileSystem *fs, NormalPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_WRITE_FAILED_PATH_IS_DIR, path);
    return fs->_openForWriting(path);
}

bool FileSystem::removeIn(FileSystem *fs, NormalPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(fs, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    return fs->_remove(path);
}

std::string FileSystem::displayPathIn(const FileSystem *fs, NormalPathView path) {
    return fs->_displayPath(path);
}
