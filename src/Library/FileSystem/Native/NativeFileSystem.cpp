#include "NativeFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Exception.h"
#include "Utility/System/Fs.h"

NativeFileSystem::NativeFileSystem(const Path &root) {
    _root = fs::absolute(root);
}

NativeFileSystem::~NativeFileSystem() = default;

bool NativeFileSystem::_exists(PathView path) const {
    assert(!path.isEmpty());
    return fs::exists(makeBasePath(path));
}

FileStat NativeFileSystem::_stat(PathView path) const {
    assert(!path.isEmpty());
    return fs::stat(makeBasePath(path));
}

void NativeFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    Path basePath = makeBasePath(path);

    try {
        fs::ls(basePath, entries);
    } catch (const Exception &) {
        if (path.isEmpty())
            return; // ls("") always works.

        if (fs::stat(basePath).type == FILE_REGULAR)
            FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    }

    // Files with '\\' in filename are not observable through this interface.
    std::erase_if(*entries, [](const DirectoryEntry &entry) { return entry.name.find('\\') != std::string::npos; });
}

Blob NativeFileSystem::_read(PathView path) const {
    assert(!path.isEmpty());
    return Blob::fromFile(makeBasePath(path));
}

void NativeFileSystem::_write(PathView path, const Blob &data) {
    assert(!path.isEmpty());
    Path basePath = makeBasePath(path);
    fs::mkdirs(makeBasePath(Path(path).parent()));
    FileOutputStream stream(basePath);
    stream.write(data.data(), data.size());
    stream.close();
}

std::unique_ptr<InputStream> NativeFileSystem::_openForReading(PathView path) const {
    assert(!path.isEmpty());
    return std::make_unique<FileInputStream>(makeBasePath(path));
}

std::unique_ptr<OutputStream> NativeFileSystem::_openForWriting(PathView path) {
    assert(!path.isEmpty());
    Path basePath = makeBasePath(path);
    fs::mkdirs(makeBasePath(Path(path).parent()));
    return std::make_unique<FileOutputStream>(basePath);
}

bool NativeFileSystem::_remove(PathView path) {
    assert(!path.isEmpty());
    return fs::remove(makeBasePath(path));
}

std::string NativeFileSystem::_displayPath(PathView path) const {
    return makeBasePath(path).displayString();
}

Path NativeFileSystem::makeBasePath(PathView path) const {
    return _root / Path(path);
}
