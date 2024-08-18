#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(const FileSystemPath &path) const {
    return nonNullBase()->_exists(path);
}

FileStat ProxyFileSystem::_stat(const FileSystemPath &path) const {
    return nonNullBase()->_stat(path);
}

void ProxyFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    nonNullBase()->_ls(path, entries);
}

Blob ProxyFileSystem::_read(const FileSystemPath &path) const {
    return nonNullBase()->_read(path);
}

void ProxyFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    return nonNullBase()->_write(path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(const FileSystemPath &path) const {
    return nonNullBase()->_openForReading(path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(const FileSystemPath &path) {
    return nonNullBase()->_openForWriting(path);
}

void ProxyFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    return nonNullBase()->_rename(srcPath, dstPath);
}

bool ProxyFileSystem::_remove(const FileSystemPath &path) {
    return nonNullBase()->_remove(path);
}

std::string ProxyFileSystem::_displayPath(const FileSystemPath &path) const {
    return nonNullBase()->_displayPath(path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
