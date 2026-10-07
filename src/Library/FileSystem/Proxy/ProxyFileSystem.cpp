#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(PathView path) const {
    return existsIn(nonNullBase(), path);
}

FileStat ProxyFileSystem::_stat(PathView path) const {
    return statIn(nonNullBase(), path);
}

void ProxyFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    lsIn(nonNullBase(), path, entries);
}

Blob ProxyFileSystem::_read(PathView path) const {
    return readIn(nonNullBase(), path);
}

void ProxyFileSystem::_write(PathView path, const Blob &data) {
    return writeIn(nonNullBase(), path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(PathView path) const {
    return openForReadingIn(nonNullBase(), path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(PathView path) {
    return openForWritingIn(nonNullBase(), path);
}

bool ProxyFileSystem::_remove(PathView path) {
    return removeIn(nonNullBase(), path);
}

std::string ProxyFileSystem::_displayPath(PathView path) const {
    return displayPathIn(nonNullBase(), path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
