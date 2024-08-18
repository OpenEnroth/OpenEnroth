#include "MaskingFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

MaskingFileSystem::MaskingFileSystem(FileSystem *base) : _base(base) {
    assert(_base);
}

MaskingFileSystem::~MaskingFileSystem() = default;

void MaskingFileSystem::mask(std::string_view path) {
    mask(FileSystemPath(path));
}

void MaskingFileSystem::mask(const FileSystemPath &path) {
    // Chop drops all children for the newly inserted node. This is needed for the implementation of `isMasked` to work.
    _masks.chop(_masks.insertOrAssign(path, true));
}

void MaskingFileSystem::clearMasks() {
    _masks.clear();
}

bool MaskingFileSystem::isMasked(const FileSystemPath &path) const {
    return _masks.walk(path)->hasValue();
}

bool MaskingFileSystem::_exists(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        return false;
    return _base->exists(path);
}

FileStat MaskingFileSystem::_stat(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        return {};
    return _base->stat(path);
}

void MaskingFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    if (isMasked(path)) {
        if (path.isEmpty()) {
            return; // Pretend root exists even if it was masked.
        } else {
            throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
        }
    }

    _base->ls(path, entries);

    if (const FileSystemTrieNode<bool> *node = _masks.find(path)) {
        std::erase_if(*entries, [node] (const DirectoryEntry &entry) {
            if (FileSystemTrieNode<bool> *child = node->child(entry.name)) {
                return child->hasValue();
            } else {
                return false;
            }
        });
    }
}

Blob MaskingFileSystem::_read(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    return _base->read(path);
}

std::unique_ptr<InputStream> MaskingFileSystem::_openForReading(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    if (isMasked(path))
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    return _base->openForReading(path);
}

bool MaskingFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());

    if (isMasked(path))
        return false;

    FileStat stat = _base->stat(path);
    if (!stat)
        return false;

    mask(path);
    return true;
}

std::string MaskingFileSystem::_displayPath(const FileSystemPath &path) const {
    // TODO(captainurist): gotta tag masked paths somehow, otherwise error messages might get confusing.
    return _base->displayPath(path);
}
