#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(NormalPathView path) const {
    return nonNullBase()->exists(path);
}

FileStat ProxyFileSystem::_stat(NormalPathView path) const {
    return nonNullBase()->stat(path);
}

void ProxyFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    nonNullBase()->ls(path, entries);
}

Blob ProxyFileSystem::_read(NormalPathView path) const {
    return nonNullBase()->read(path);
}

void ProxyFileSystem::_write(NormalPathView path, const Blob &data) {
    return nonNullBase()->write(path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(NormalPathView path) const {
    return nonNullBase()->openForReading(path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(NormalPathView path) {
    return nonNullBase()->openForWriting(path);
}

bool ProxyFileSystem::_remove(NormalPathView path) {
    return nonNullBase()->remove(path);
}

std::string ProxyFileSystem::_displayPath(NormalPathView path) const {
    return nonNullBase()->displayPath(path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
