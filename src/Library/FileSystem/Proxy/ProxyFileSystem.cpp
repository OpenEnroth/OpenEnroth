#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(FileSystemPathView path) const {
    return nonNullBase()->_exists(path);
}

FileStat ProxyFileSystem::_stat(FileSystemPathView path) const {
    return nonNullBase()->_stat(path);
}

void ProxyFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    nonNullBase()->_ls(path, entries);
}

Blob ProxyFileSystem::_read(FileSystemPathView path) const {
    return nonNullBase()->_read(path);
}

void ProxyFileSystem::_write(FileSystemPathView path, const Blob &data) {
    return nonNullBase()->_write(path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(FileSystemPathView path) const {
    return nonNullBase()->_openForReading(path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(FileSystemPathView path) {
    return nonNullBase()->_openForWriting(path);
}

void ProxyFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    return nonNullBase()->_rename(srcPath, dstPath);
}

bool ProxyFileSystem::_remove(FileSystemPathView path) {
    return nonNullBase()->_remove(path);
}

std::string ProxyFileSystem::_displayPath(FileSystemPathView path) const {
    return nonNullBase()->_displayPath(path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
