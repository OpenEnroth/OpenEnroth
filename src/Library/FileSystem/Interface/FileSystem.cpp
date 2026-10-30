#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "FileSystemException.h"

bool FileSystem::exists(PathView path) const {
    NormalPath normalPath(path);
    return normalPath.isAccessible() && existsIn(this, normalPath);
}

FileStat FileSystem::stat(PathView path) const {
    NormalPath normalPath(path);
    return normalPath.isAccessible() ? statIn(this, normalPath) : FileStat();
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    std::vector<DirectoryEntry> result;
    ls(path, &result);
    return result;
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    lsIn(this, normalPath, entries);
}

Blob FileSystem::read(PathView path) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return readIn(this, normalPath);
}

void FileSystem::write(PathView path, const Blob &data) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    writeIn(this, normalPath, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return openForReadingIn(this, normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return openForWritingIn(this, normalPath);
}

bool FileSystem::remove(PathView path) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, normalPath.path());
    return removeIn(this, normalPath);
}

std::string FileSystem::displayPath(PathView path) const {
    NormalPath normalPath(path);
    if (normalPath.isAccessible())
        return displayPathIn(this, normalPath);

    std::string result = displayPathIn(this, NormalPathView()); // The implementation never sees an inaccessible path.
    if (normalPath.path().isEmpty())
        return result; // Exception messages about the root pass the layer's own spelling of it here.
    if (!result.ends_with('/'))
        result += '/';
    result += normalPath.path().displayString();
    return result;
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
