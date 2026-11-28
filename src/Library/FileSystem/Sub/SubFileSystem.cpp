#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

SubFileSystem::SubFileSystem(PathView basePath, FileSystem *base) : _base(base), _basePath(basePath) {
    assert(_base);
    assert(_basePath.isAccessible());
}

bool SubFileSystem::_exists(NormalPathView path) const {
    return _base->exists(makeBasePath(path));
}

FileStat SubFileSystem::_stat(NormalPathView path) const {
    return _base->stat(makeBasePath(path));
}

void SubFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && _base->stat(_basePath).type != FILE_DIRECTORY)
        return;

    _base->ls(makeBasePath(path), entries);
}

Blob SubFileSystem::_read(NormalPathView path) const {
    return _base->read(makeBasePath(path));
}

void SubFileSystem::_write(NormalPathView path, const Blob &data) {
    _base->write(makeBasePath(path), data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(NormalPathView path) const {
    return _base->openForReading(makeBasePath(path));
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(NormalPathView path) {
    return _base->openForWriting(makeBasePath(path));
}

bool SubFileSystem::_remove(NormalPathView path) {
    return _base->remove(makeBasePath(path));
}

std::string SubFileSystem::_displayPath(NormalPathView path) const {
    return _base->displayPath(makeBasePath(path));
}

NormalPath SubFileSystem::makeBasePath(NormalPathView path) const {
    return _basePath / path;
}
