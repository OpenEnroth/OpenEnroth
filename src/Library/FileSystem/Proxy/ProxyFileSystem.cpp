#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(NormalPathView path) const {
    return existsIn(nonNullBase(), path);
}

FileStat ProxyFileSystem::_stat(NormalPathView path) const {
    return statIn(nonNullBase(), path);
}

void ProxyFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    lsIn(nonNullBase(), path, entries);
}

Blob ProxyFileSystem::_read(NormalPathView path) const {
    return readIn(nonNullBase(), path);
}

void ProxyFileSystem::_write(NormalPathView path, const Blob &data) {
    return writeIn(nonNullBase(), path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(NormalPathView path) const {
    return openForReadingIn(nonNullBase(), path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(NormalPathView path) {
    return openForWritingIn(nonNullBase(), path);
}

bool ProxyFileSystem::_remove(NormalPathView path) {
    return removeIn(nonNullBase(), path);
}

std::string ProxyFileSystem::_displayPath(NormalPathView path) const {
    return displayPathIn(nonNullBase(), path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
