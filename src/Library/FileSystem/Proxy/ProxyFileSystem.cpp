#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    return existsOf(nonNullBase(), path);
}

FileStat ProxyFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    return statOf(nonNullBase(), path);
}

void ProxyFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    lsOf(nonNullBase(), path, entries);
}

Blob ProxyFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    return readOf(nonNullBase(), path);
}

void ProxyFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    return writeOf(nonNullBase(), path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    return openForReadingOf(nonNullBase(), path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    return openForWritingOf(nonNullBase(), path);
}

bool ProxyFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    return removeOf(nonNullBase(), path);
}

std::string ProxyFileSystem::_displayPath(PathView path) const {
    return displayPathOf(nonNullBase(), path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
