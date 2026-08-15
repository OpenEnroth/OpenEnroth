#include "NativeFileSystem.h"

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Os.h"

// Parent directory, keeping the trailing separator so that roots come out right - the parent of "C:/x" is "C:/".
static NativePath parentOf(const NativePath &path) {
    std::string_view wtf8 = path.toWtf8();
    size_t pos = wtf8.find_last_of('/');
    assert(pos != std::string_view::npos); // We're only called for absolute paths.
    return NativePath::fromWtf8(wtf8.substr(0, pos + 1));
}

NativeFileSystem::NativeFileSystem(const NativePath &root) {
    _root = os::absolute(root);
}

NativeFileSystem::~NativeFileSystem() = default;

std::pair<std::unique_ptr<NativeFileSystem>, FileSystemPath> NativeFileSystem::fromNativePath(const NativePath &path) {
    // lexically_normal collapses ".." even right after the root, so the tail can never be an escaping path.
    std::filesystem::path absolutePath = std::filesystem::path(os::absolute(path).native()).lexically_normal();

    // FileSystemPath normalizes - lexically_normal can leave a trailing '/' in there.
    return {std::make_unique<NativeFileSystem>(NativePath::fromNative(absolutePath.root_path().native())),
            FileSystemPath(NativePath::fromNative(absolutePath.relative_path().native()).toWtf8())};
}

bool NativeFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return os::exists(toNativePath(path));
}

FileStat NativeFileSystem::_stat(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return os::stat(toNativePath(path));
}

void NativeFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    NativePath basePath = toNativePath(path);

    // Handle the known errors first.
    FileType type = os::stat(basePath).type;
    if (path.isEmpty() && type != FILE_DIRECTORY)
        return; // ls("") should always work.
    if (type == FILE_REGULAR)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
    if (type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    for (DirectoryEntry &entry : os::ls(basePath)) {
        if (entry.name.find('\\') != std::string::npos)
            continue; // Files with '\\' in filename are not observable through this interface. Don't be a retard.
        entries->push_back(std::move(entry));
    }
}

Blob NativeFileSystem::_read(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(toNativePath(path));
}

void NativeFileSystem::_write(FileSystemPathView path, const Blob &data) {
    assert(!path.isEmpty());
    NativePath basePath = toNativePath(path);
    os::mkdirs(parentOf(basePath));
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
    NativePath basePath = toNativePath(path);
    os::mkdirs(parentOf(basePath));
    return std::make_unique<FileOutputStream>(basePath);
}

bool NativeFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());
    return os::remove(toNativePath(path));
}

std::string NativeFileSystem::_displayPath(FileSystemPathView path) const {
    return toNativePath(path).displayString();
}

NativePath NativeFileSystem::toNativePath(std::string_view path) const {
    return toNativePath(FileSystemPath(path));
}

NativePath NativeFileSystem::toNativePath(FileSystemPathView path) const {
    if (path.isEmpty())
        return _root; // `_root / ""` would add a trailing separator.

    return _root / NativePath::fromWtf8(path.string());
}
