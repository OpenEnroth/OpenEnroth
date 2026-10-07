#include "ProxyFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

bool ProxyFileSystem::_exists(PathView path) const {
    return existsOf(nonNullBase(), path);
}

FileStat ProxyFileSystem::_stat(PathView path) const {
    return statOf(nonNullBase(), path);
}

void ProxyFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    lsOf(nonNullBase(), path, entries);
}

Blob ProxyFileSystem::_read(PathView path) const {
    return readOf(nonNullBase(), path);
}

void ProxyFileSystem::_write(PathView path, const Blob &data) {
    return writeOf(nonNullBase(), path, data);
}

std::unique_ptr<InputStream> ProxyFileSystem::_openForReading(PathView path) const {
    return openForReadingOf(nonNullBase(), path);
}

std::unique_ptr<OutputStream> ProxyFileSystem::_openForWriting(PathView path) {
    return openForWritingOf(nonNullBase(), path);
}

bool ProxyFileSystem::_remove(PathView path) {
    return removeOf(nonNullBase(), path);
}

std::string ProxyFileSystem::_displayPath(PathView path) const {
    return displayPathOf(nonNullBase(), path);
}

FileSystem *ProxyFileSystem::nonNullBase() const {
    assert(_base);
    return _base;
}
