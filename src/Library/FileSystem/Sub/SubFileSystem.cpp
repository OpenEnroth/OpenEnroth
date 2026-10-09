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

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(Path(basePath), base) {
}

bool SubFileSystem::_exists(NormalPathView path) const {
    return existsIn(_base, basePath(path));
}

FileStat SubFileSystem::_stat(NormalPathView path) const {
    return statIn(_base, basePath(path));
}

void SubFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && statIn(_base, _basePath).type != FILE_DIRECTORY)
        return;

    lsIn(_base, basePath(path), entries);
}

Blob SubFileSystem::_read(NormalPathView path) const {
    return readIn(_base, basePath(path));
}

void SubFileSystem::_write(NormalPathView path, const Blob &data) {
    writeIn(_base, basePath(path), data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(NormalPathView path) const {
    return openForReadingIn(_base, basePath(path));
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(NormalPathView path) {
    return openForWritingIn(_base, basePath(path));
}

bool SubFileSystem::_remove(NormalPathView path) {
    return removeIn(_base, basePath(path));
}

std::string SubFileSystem::_displayPath(NormalPathView path) const {
    return displayPathIn(_base, basePath(path));
}

NormalPath SubFileSystem::basePath(NormalPathView path) const {
    return _basePath / path;
}
