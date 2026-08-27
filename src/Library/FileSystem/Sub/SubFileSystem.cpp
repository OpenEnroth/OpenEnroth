#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

SubFileSystem::SubFileSystem(PathView basePath, FileSystem *base)
    : _base(base), _basePath(basePath) {
    assert(_base);
}

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(PathView(Path(basePath)), base) {
}

bool SubFileSystem::_exists(PathView path) const {
    return _base->exists(_basePath / path);
}

FileStat SubFileSystem::_stat(PathView path) const {
    return _base->stat(_basePath / path);
}

void SubFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && _base->stat(_basePath).type != FILE_DIRECTORY)
        return;

    _base->ls(_basePath / path, entries);
}

Blob SubFileSystem::_read(PathView path) const {
    return _base->read(_basePath / path);
}

void SubFileSystem::_write(PathView path, const Blob &data) {
    _base->write(_basePath / path, data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(PathView path) const {
    return _base->openForReading(_basePath / path);
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(PathView path) {
    return _base->openForWriting(_basePath / path);
}

bool SubFileSystem::_remove(PathView path) {
    return _base->remove(_basePath / path);
}

std::string SubFileSystem::_displayPath(PathView path) const {
    return _base->displayPath(_basePath / path);
}
