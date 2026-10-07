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
    return existsOf(_base, basePath(path));
}

FileStat SubFileSystem::_stat(PathView path) const {
    return statOf(_base, basePath(path));
}

void SubFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && statOf(_base, _basePath).type != FILE_DIRECTORY)
        return;

    lsOf(_base, basePath(path), entries);
}

Blob SubFileSystem::_read(PathView path) const {
    return readOf(_base, basePath(path));
}

void SubFileSystem::_write(PathView path, const Blob &data) {
    writeOf(_base, basePath(path), data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(PathView path) const {
    return openForReadingOf(_base, basePath(path));
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(PathView path) {
    return openForWritingOf(_base, basePath(path));
}

bool SubFileSystem::_remove(PathView path) {
    return removeOf(_base, basePath(path));
}

std::string SubFileSystem::_displayPath(PathView path) const {
    return displayPathOf(_base, basePath(path));
}

Path SubFileSystem::basePath(PathView path) const {
    return path.isEmpty() ? _basePath : _basePath / Path(path);
}
