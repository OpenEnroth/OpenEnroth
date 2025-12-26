#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

SubFileSystem::SubFileSystem(FileSystemPathView basePath, FileSystem *base)
    : _base(base), _basePath(basePath) {
    assert(_base);
}

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(FileSystemPathView(FileSystemPath(basePath)), base) {
}

bool SubFileSystem::_exists(FileSystemPathView path) const {
    return _base->exists(_basePath / path);
}

FileStat SubFileSystem::_stat(FileSystemPathView path) const {
    return _base->stat(_basePath / path);
}

void SubFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    _base->ls(_basePath / path, entries);
}

Blob SubFileSystem::_read(FileSystemPathView path) const {
    return _base->read(_basePath / path);
}

void SubFileSystem::_write(FileSystemPathView path, const Blob &data) {
    _base->write(_basePath / path, data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(FileSystemPathView path) const {
    return _base->openForReading(_basePath / path);
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(FileSystemPathView path) {
    return _base->openForWriting(_basePath / path);
}

void SubFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    _base->rename(_basePath / srcPath, _basePath / dstPath);
}

bool SubFileSystem::_remove(FileSystemPathView path) {
    return _base->remove(_basePath / path);
}

std::string SubFileSystem::_displayPath(FileSystemPathView path) const {
    return _base->displayPath(_basePath / path);
}
