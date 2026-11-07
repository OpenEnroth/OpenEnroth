#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "Utility/Exception.h"

SubFileSystem::SubFileSystem(PathView basePath, FileSystem *base) : _base(base), _basePath(basePath) {
    assert(_base);

    if (!_basePath.isAccessible())
        throw Exception("Base path '{}' of a sub file system is not accessible in '{}'", basePath.str(), _base->displayPath(""));
}

bool SubFileSystem::_exists(NormalPathView path) const {
    return _base->exists(basePath(path));
}

FileStat SubFileSystem::_stat(NormalPathView path) const {
    return _base->stat(basePath(path));
}

void SubFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isRoot() && _base->stat(_basePath).type != FILE_DIRECTORY)
        return;

    _base->ls(basePath(path), entries);
}

Blob SubFileSystem::_read(NormalPathView path) const {
    return _base->read(basePath(path));
}

void SubFileSystem::_write(NormalPathView path, const Blob &data) {
    _base->write(basePath(path), data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(NormalPathView path) const {
    return _base->openForReading(basePath(path));
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(NormalPathView path) {
    return _base->openForWriting(basePath(path));
}

bool SubFileSystem::_remove(NormalPathView path) {
    return _base->remove(basePath(path));
}

std::string SubFileSystem::_displayPath(NormalPathView path) const {
    return _base->displayPath(basePath(path));
}

NormalPath SubFileSystem::basePath(NormalPathView path) const {
    return _basePath / path;
}
