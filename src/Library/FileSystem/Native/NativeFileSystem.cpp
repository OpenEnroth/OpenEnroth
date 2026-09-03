#include "NativeFileSystem.h"

#include <cassert>
#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"

NativeFileSystem::NativeFileSystem(const Path &root) {
    _root = fs::absolute(root);
}

NativeFileSystem::~NativeFileSystem() = default;

bool NativeFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return fs::exists(basePath(path));
}

FileStat NativeFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return fs::stat(basePath(path));
}

void NativeFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    Path nativePath = basePath(path);

    // Handle the known errors first.
    FileType type = fs::stat(nativePath).type;
    if (path.isEmpty() && type != FILE_DIRECTORY)
        return; // ls("") should always work.
    if (type == FILE_REGULAR)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    fs::ls(nativePath, entries);
}

Blob NativeFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return Blob::fromFile(basePath(path));
}

void NativeFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    Path nativePath = basePath(path);
    fs::mkdirs(nativePath.normalized().parent());
    FileOutputStream stream(nativePath);
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> NativeFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(basePath(path));
}

std::unique_ptr<OutputStream> NativeFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    Path nativePath = basePath(path);
    fs::mkdirs(nativePath.normalized().parent());
    return std::make_unique<FileOutputStream>(nativePath);
}

bool NativeFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return fs::remove(basePath(path));
}

std::string NativeFileSystem::_displayPath(PathView path) const {
    return basePath(path).displayString();
}

Path NativeFileSystem::toNativePath(std::string_view path) const {
    return toNativePath(Path(path));
}

Path NativeFileSystem::toNativePath(PathView path) const {
    // Public, and it takes a path in this file system's namespace, so it validates like every other public method.
    // Without that it would hand back a path outside the root for an absolute argument, since a rooted tail
    // replaces the head - while displayPath, given the same input, would say something else entirely.
    Path normalPath = Path(path.string()).normalized();
    if (normalPath.isAbsolute() || normalPath.isEscaping())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);

    return basePath(normalPath);
}

Path NativeFileSystem::basePath(PathView path) const {
    // No validation - the private _ methods are only reachable through the public boundary, which has already done
    // it, and _displayPath has to work on a path that was just rejected, or raising an error would recurse. What it
    // does guarantee is that the result is under the root: a rooted tail would otherwise replace it outright, so
    // the root of the argument is dropped rather than honoured.
    std::string_view tail = path.string();
    tail.remove_prefix(path.root().size());

    if (tail.empty())
        return _root; // `_root / ""` would add a trailing separator.

    return _root / Path(tail);
}
