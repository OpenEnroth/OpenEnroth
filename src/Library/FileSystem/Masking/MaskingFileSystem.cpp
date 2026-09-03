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
    mask(Path(path).normalized());
}

void MaskingFileSystem::mask(PathView path) {
    assert(path.isNormalized()); // The tries are walked with normal paths, so a non-normal key could never match.
    _masks.insertOrAssign(path, true);
}

bool MaskingFileSystem::unmask(std::string_view path) {
    return unmask(Path(path).normalized());
}

bool MaskingFileSystem::unmask(PathView path) {
    assert(path.isNormalized()); // The tries are walked with normal paths, so a non-normal key could never match.
    FileSystemTrieNode<bool> *node = _masks.find(path);
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

bool MaskingFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_exists(path);
}

FileStat MaskingFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    if (isMasked(path))
        return {};
    return ProxyFileSystem::_stat(path);
}

void MaskingFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
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

Blob MaskingFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_read(path);
}

void MaskingFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    ProxyFileSystem::_write(path, data);
}

std::unique_ptr<InputStream> MaskingFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    if (isMasked(path))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return ProxyFileSystem::_openForReading(path);
}

std::unique_ptr<OutputStream> MaskingFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    if (isMasked(path))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
    return ProxyFileSystem::_openForWriting(path);
}

bool MaskingFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    if (isMasked(path))
        return false;
    return ProxyFileSystem::_remove(path);
}

std::string MaskingFileSystem::_displayPath(PathView path) const {
    if (isMasked(path))
        return join("masked://", path.displayString());
    return ProxyFileSystem::_displayPath(path);
}
