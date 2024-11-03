#include "MaskingFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

MaskingFileSystem::MaskingFileSystem(FileSystem *base) : ProxyFileSystem(base) {}

MaskingFileSystem::~MaskingFileSystem() = default;

void MaskingFileSystem::mask(std::string_view path) {
    mask(FileSystemPath(path));
}

void MaskingFileSystem::mask(const FileSystemPath &path) {
    _masks.insertOrAssign(path, true);
}

bool MaskingFileSystem::unmask(std::string_view path) {
    return unmask(FileSystemPath(path));
}

bool MaskingFileSystem::unmask(const FileSystemPath &path) {
    FileSystemTrieNode<bool> *node = _masks.find(path);
    if (!node || !node->hasValue() || !node->value())
        return false; // Can only unmask what was previously masked.
    node->value() = false;
    return true;
}

void MaskingFileSystem::clearMasks() {
    _masks.clear();
}

bool MaskingFileSystem::isMasked(const FileSystemPath &path) const {
    const FileSystemTrieNode<bool> *node = _masks.root();

    for (std::string_view chunk : path.chunks()) {
        if (node->hasValue() && node->value())
            return true;

        node = node->child(chunk);
        if (!node)
            return false;
    }

    return node->hasValue() && node->value();
}

bool MaskingFileSystem::_exists(const FileSystemPath &path) const {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_exists(path);
}

FileStat MaskingFileSystem::_stat(const FileSystemPath &path) const {
    if (isMasked(path))
        return {};
    return ProxyFileSystem::_stat(path);
}

void MaskingFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
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

Blob MaskingFileSystem::_read(const FileSystemPath &path) const {
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_read(path);
}

void MaskingFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    ProxyFileSystem::_write(path, data);
}

std::unique_ptr<InputStream> MaskingFileSystem::_openForReading(const FileSystemPath &path) const {
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_openForReading(path);
}

std::unique_ptr<OutputStream> MaskingFileSystem::_openForWriting(const FileSystemPath &path) {
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    return ProxyFileSystem::_openForWriting(path);
}

void MaskingFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    if (isMasked(srcPath))
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (isMasked(dstPath))
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
    return ProxyFileSystem::_rename(srcPath, dstPath);
}

bool MaskingFileSystem::_remove(const FileSystemPath &path) {
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_remove(path);
}

std::string MaskingFileSystem::_displayPath(const FileSystemPath &path) const {
    if (isMasked(path))
        return "masked://" + path.string();
    return ProxyFileSystem::_displayPath(path);
}
