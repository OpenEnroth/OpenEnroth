#include "FileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "FileSystemPath.h"

#include "FileSystemException.h"

static constexpr size_t COPY_BUFFER_SIZE = 1024 * 1024;

bool FileSystem::exists(std::string_view path) const {
    return exists(FileSystemPath(path));
}

bool FileSystem::exists(FileSystemPathView path) const {
    if (path.isEmpty())
        return true; // Root always exists.
    if (path.isEscaping())
        return false; // Escaping paths are not accessible through this interface.
    return _exists(path);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(FileSystemPath(path));
}

FileStat FileSystem::stat(FileSystemPathView path) const {
    if (path.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (path.isEscaping())
        return FileStat();
    return _stat(path);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(FileSystemPath(path));
}

std::vector<DirectoryEntry> FileSystem::ls(FileSystemPathView path) const {
    if (path.isEscaping())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    _ls(path, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(FileSystemPath(path), entries);
}

void FileSystem::ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEscaping())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(path, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(FileSystemPath(path));
}

Blob FileSystem::read(FileSystemPathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(path);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(FileSystemPath(path), data);
}

void FileSystem::write(FileSystemPathView path, const Blob &data) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(path, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(FileSystemPath(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(FileSystemPathView path) const {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(path);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(FileSystemPath(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(FileSystemPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(path);
}

void FileSystem::rename(std::string_view srcPath, std::string_view dstPath) {
    return rename(FileSystemPath(srcPath), FileSystemPath(dstPath));
}

void FileSystem::rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    if (srcPath.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);
    if (srcPath.isEscaping())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_NOT_ACCESSIBLE, srcPath, dstPath);
    if (dstPath.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
    if (dstPath.isEscaping())
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_ACCESSIBLE, srcPath, dstPath);
    if (srcPath.isPrefixOf(dstPath) && srcPath != dstPath)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_IS_PARENT_OF_DST, srcPath, dstPath);
    _rename(srcPath, dstPath);
}

bool FileSystem::remove(std::string_view path) {
    return remove(FileSystemPath(path));
}

bool FileSystem::remove(FileSystemPathView path) {
    if (path.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (path.isEscaping())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(path);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(FileSystemPath(path));
}

std::string FileSystem::displayPath(FileSystemPathView path) const {
    return _displayPath(path);
}

void FileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    FileStat srcStat = stat(srcPath);
    if (!srcStat)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);

    FileStat dstStat = stat(dstPath);
    if (dstStat.type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (dstStat.type == FILE_REGULAR && srcStat.type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    if (dstStat)
        remove(dstPath);

    std::unique_ptr<char[]> buffer;
    auto copyFile = [this, &buffer](FileSystemPathView srcPath, FileSystemPathView dstPath) -> void {
        std::unique_ptr<InputStream> input = openForReading(srcPath);
        std::unique_ptr<OutputStream> output = openForWriting(dstPath);

        if (!buffer)
            buffer = std::make_unique<char[]>(COPY_BUFFER_SIZE); // TODO(captainurist): C++23, use make_unique_for_overwrite.

        while (true) {
            size_t bytes = input->read(buffer.get(), COPY_BUFFER_SIZE);
            if (!bytes)
                break;
            output->write(buffer.get(), bytes);
        }
    };

    auto copyDir = [this] (FileSystemPathView srcPath, FileSystemPathView dstPath, const auto &copyAny) -> void {
        for (const DirectoryEntry &entry : ls(srcPath))
            copyAny(entry.type, srcPath / entry.name, dstPath / entry.name, copyAny);
    };

    auto copyAny = [this, &copyFile, &copyDir] (FileType type, FileSystemPathView srcPath, FileSystemPathView dstPath, const auto &copyAny) -> void {
        if (type == FILE_REGULAR) {
            copyFile(srcPath, dstPath);
        } else {
            assert(type == FILE_DIRECTORY);
            copyDir(srcPath, dstPath, copyAny);
        }
    };

    copyAny(srcStat.type, srcPath, dstPath, copyAny);
    remove(srcPath);
}
