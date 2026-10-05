#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(PathView path) const {
    return nonNullBase()->_exists(path);
}

FileStat ProxyFileSystem::_stat(PathView path) const {
    return nonNullBase()->_stat(path);
}

void ProxyFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    nonNullBase()->_ls(path, entries);
}

Blob ProxyFileSystem::_read(PathView path) const {
    return nonNullBase()->_read(path);
}

void ProxyFileSystem::_write(PathView path, const Blob &data) {
    return nonNullBase()->_write(path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(PathView path) const {
    return nonNullBase()->_openForReading(path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(PathView path) {
    return nonNullBase()->_openForWriting(path);
}

bool ProxyFileSystem::_remove(PathView path) {
    return nonNullBase()->_remove(path);
}

std::string ProxyFileSystem::_displayPath(PathView path) const {
    return nonNullBase()->_displayPath(path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
