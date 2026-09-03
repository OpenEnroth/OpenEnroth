#include "SubFileSystem.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "Utility/Exception.h"

SubFileSystem::SubFileSystem(PathView basePath, FileSystem *base)
    : _base(base), _basePath(Path(basePath).normalized()) {
    assert(_base);

    // The base path is prepended to everything this file system is asked for, so an escaping one escapes on every
    // call. Nothing downstream checks it - the base's public methods used to, but delegation goes through the
    // forwarders now, which skip validation by design. An assert wouldn't do: this is the only guard left, and it
    // has to hold in a release build too.
    if (_basePath.isAbsolute() || _basePath.isEscaping())
        throw Exception("Base path '{}' is not accessible", _basePath);
}

SubFileSystem::SubFileSystem(std::string_view basePath, FileSystem *base)
    : SubFileSystem(PathView(Path(basePath)), base) {
}

bool SubFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    return existsOf(_base, _basePath / path);
}

FileStat SubFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    return statOf(_base, _basePath / path);
}

void SubFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    // A root always exists, so ls("") has to work even if the base path doesn't, or isn't a directory.
    if (path.isEmpty() && statOf(_base, _basePath).type != FILE_DIRECTORY)
        return;

    lsOf(_base, _basePath / path, entries);
}

Blob SubFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    return readOf(_base, _basePath / path);
}

void SubFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    writeOf(_base, _basePath / path, data);
}

std::unique_ptr<InputStream> SubFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    return openForReadingOf(_base, _basePath / path);
}

std::unique_ptr<OutputStream> SubFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    return openForWritingOf(_base, _basePath / path);
}

bool SubFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    return removeOf(_base, _basePath / path);
}

std::string SubFileSystem::_displayPath(PathView path) const {
    return displayPathOf(_base, _basePath / path);
}
