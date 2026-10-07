#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "Library/FileSystem/Interface/NormalizedFileSystemPath.h"

#include "Utility/Exception.h"

SubFileSystem::SubFileSystem(PathView basePath, FileSystem *base) : _base(base) {
    assert(_base);

    NormalizedFileSystemPath normalPath(basePath);
    if (!normalPath.isAccessible())
        throw Exception("Base path '{}' of a sub file system is not accessible in '{}'", basePath.str(), _base->displayPath(""));
    _basePath = Path(PathView(normalPath));
}

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(Path(basePath), base) {
}

bool SubFileSystem::_exists(PathView path) const {
    return existsIn(_base, basePath(path));
}

FileStat SubFileSystem::_stat(PathView path) const {
    return statIn(_base, basePath(path));
}

void SubFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && statIn(_base, _basePath).type != FILE_DIRECTORY)
        return;

    lsIn(_base, basePath(path), entries);
}

Blob SubFileSystem::_read(PathView path) const {
    return readIn(_base, basePath(path));
}

void SubFileSystem::_write(PathView path, const Blob &data) {
    writeIn(_base, basePath(path), data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(PathView path) const {
    return openForReadingIn(_base, basePath(path));
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(PathView path) {
    return openForWritingIn(_base, basePath(path));
}

bool SubFileSystem::_remove(PathView path) {
    return removeIn(_base, basePath(path));
}

std::string SubFileSystem::_displayPath(PathView path) const {
    return displayPathIn(_base, basePath(path));
}

Path SubFileSystem::basePath(PathView path) const {
    return path.isEmpty() ? _basePath : _basePath / Path(path);
}
