#include "MaskingFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

MaskingFileSystem::MaskingFileSystem(FileSystem *base) : ProxyFileSystem(base) {}

MaskingFileSystem::~MaskingFileSystem() = default;

void MaskingFileSystem::mask(std::string_view path) {
    mask(Path(path));
}

void MaskingFileSystem::mask(PathView path) {
    NormalPath normalPath(path);
    assert(normalPath.isAccessible());
    _masks.insertOrAssign(normalPath.path(), true);
}

bool MaskingFileSystem::unmask(std::string_view path) {
    return unmask(Path(path));
}

bool MaskingFileSystem::unmask(PathView path) {
    FileSystemTrieNode<bool> *node = _masks.find(NormalPath(path).path());
    if (!node || !node->hasValue() || !node->value())
        return false; // Can only unmask what was previously masked.
    node->value() = false;
    return true;
}

void MaskingFileSystem::clearMasks() {
    _masks.clear();
}

bool MaskingFileSystem::isMasked(PathView path) const {
    const FileSystemTrieNode<bool> *node = _masks.root();

    for (std::string_view chunk : path.split()) {
        if (node->hasValue() && node->value())
            return true;

        node = node->child(chunk);
        if (!node)
            return false;
    }

    return node->hasValue() && node->value();
}

bool MaskingFileSystem::_exists(NormalPathView path) const {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_exists(path);
}

FileStat MaskingFileSystem::_stat(NormalPathView path) const {
    if (isMasked(path))
        return {};
    return ProxyFileSystem::_stat(path);
}

void MaskingFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    if (isMasked(path)) {
        if (path.isEmpty()) {
            return; // Pretend root exists even if it was masked.
        } else {
            FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
        }
    }

    ProxyFileSystem::_ls(path, entries);

    if (const FileSystemTrieNode<bool> *node = _masks.find(path)) {
        std::erase_if(*entries, [node] (const DirectoryEntry &entry) {
            if (FileSystemTrieNode<bool> *child = node->child(entry.name)) {
                return child->hasValue() && child->value();
            } else {
                return false;
            }
        });
    }
}

Blob MaskingFileSystem::_read(NormalPathView path) const {
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_read(path);
}

void MaskingFileSystem::_write(NormalPathView path, const Blob &data) {
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    ProxyFileSystem::_write(path, data);
}

std::unique_ptr<InputStream> MaskingFileSystem::_openForReading(NormalPathView path) const {
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_openForReading(path);
}

std::unique_ptr<OutputStream> MaskingFileSystem::_openForWriting(NormalPathView path) {
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    return ProxyFileSystem::_openForWriting(path);
}

bool MaskingFileSystem::_remove(NormalPathView path) {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_remove(path);
}

std::string MaskingFileSystem::_displayPath(NormalPathView path) const {
    if (isMasked(path))
        return join("masked://", txt::encodedToUtf8(path.str(), ENCODING_UTF8)); // Replaces invalid UTF8.
    return ProxyFileSystem::_displayPath(path);
}
