#include "NativeFileSystem.h"

#include <cassert>
#include <algorithm>
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

bool NativeFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return fs::exists(toNativePath(path));
}

FileStat NativeFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return fs::stat(toNativePath(path));
}

void NativeFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    Path basePath = toNativePath(path);

    // Handle the known errors first.
    FileType type = fs::stat(basePath).type;
    if (path.isEmpty() && type != FILE_DIRECTORY)
        return; // ls("") should always work.
    if (type == FILE_REGULAR)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    auto oldSize = static_cast<std::ptrdiff_t>(entries->size());
    fs::ls(basePath, entries);

    // Files with '\\' in filename are not observable through this interface.
    auto isUnobservable = [] (const DirectoryEntry &entry) { return entry.name.contains('\\'); };
    entries->erase(std::remove_if(entries->begin() + oldSize, entries->end(), isUnobservable), entries->end());
}

Blob NativeFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(toNativePath(path));
}

void NativeFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    Path basePath = toNativePath(path);
    fs::mkdirs(basePath.parent());
    FileOutputStream stream(basePath);
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> NativeFileSystem::_openForReading(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(toNativePath(path));
}

std::unique_ptr<OutputStream> NativeFileSystem::_openForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());
    Path basePath = toNativePath(path);
    fs::mkdirs(basePath.parent());
    return std::make_unique<FileOutputStream>(basePath);
}

bool NativeFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return fs::remove(toNativePath(path));
}

std::string NativeFileSystem::_displayPath(FileSystemPathView path) const {
    return toNativePath(path).displayString();
}

Path NativeFileSystem::toNativePath(std::string_view path) const {
    return toNativePath(FileSystemPath(path));
}

Path NativeFileSystem::toNativePath(FileSystemPathView path) const {
    if (path.isEmpty())
        return _root; // `_root / ""` would add a trailing separator.

    return _root / Path(path.string());
}
