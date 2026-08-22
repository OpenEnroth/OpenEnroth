#include "DirectoryFileSystem.h"

#include <cassert>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Os.h"

DirectoryFileSystem::DirectoryFileSystem(const NativePath &root) {
    _root = os::absolute(root);
}

DirectoryFileSystem::~DirectoryFileSystem() = default;

bool DirectoryFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return os::exists(makeBasePath(path));
}

FileStat DirectoryFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return os::stat(makeBasePath(path));
}

void DirectoryFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    NativePath basePath = makeBasePath(path);

    // Handle the known errors first.
    FileType type = os::stat(basePath).type;
    if (path.isEmpty() && type != FILE_DIRECTORY)
        return; // ls("") should always work.
    if (type == FILE_REGULAR)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    auto oldSize = static_cast<std::ptrdiff_t>(entries->size());
    os::ls(basePath, entries);

    // Files with '\\' in filename are not observable through this interface.
    auto isUnobservable = [] (const DirectoryEntry &entry) { return entry.name.contains('\\'); };
    entries->erase(std::remove_if(entries->begin() + oldSize, entries->end(), isUnobservable), entries->end());
}

Blob DirectoryFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path));
}

void DirectoryFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    NativePath basePath = makeBasePath(path);
    os::mkdirs(basePath.parent());
    FileOutputStream stream(basePath);
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> DirectoryFileSystem::_openForReading(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(makeBasePath(path));
}

std::unique_ptr<OutputStream> DirectoryFileSystem::_openForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());
    NativePath basePath = makeBasePath(path);
    os::mkdirs(basePath.parent());
    return std::make_unique<FileOutputStream>(basePath);
}

bool DirectoryFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return os::remove(makeBasePath(path));
}

std::string DirectoryFileSystem::_displayPath(FileSystemPathView path) const {
    return makeBasePath(path).displayString();
}

NativePath DirectoryFileSystem::makeBasePath(FileSystemPathView path) const {
    return _root / NativePath::fromWtf8(path.string());
}
