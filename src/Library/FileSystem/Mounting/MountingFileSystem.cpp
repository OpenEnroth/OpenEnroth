#include "MountingFileSystem.h"

#include <vector>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

MountingFileSystem::MountingFileSystem(std::string_view displayName) : _displayName(displayName) {}
MountingFileSystem::~MountingFileSystem() = default;

void MountingFileSystem::mount(std::string_view path, FileSystem *fileSystem) {
    mount(FileSystemPath(path), fileSystem);
}

void MountingFileSystem::mount(const FileSystemPath &path, FileSystem *fileSystem) {
    _trie.insertOrAssign(path, fileSystem);
}

bool MountingFileSystem::unmount(std::string_view path) {
    return unmount(FileSystemPath(path));
}

bool MountingFileSystem::unmount(const FileSystemPath &path) {
    Node *node = _trie.find(path);
    if (!node || !node->hasValue())
        return false; // Should be a real mount point, unmount("") is not equivalent to clearMounts().

    return _trie.erase(node);
}

void MountingFileSystem::clearMounts() {
    _trie.clear();
}

bool MountingFileSystem::_exists(const FileSystemPath &path) const {
    assert(!path.isEmpty());

    auto [node, mount, tail] = walk(path);
    return node ? true : mount ? mount->exists(tail) : false;
}

FileStat MountingFileSystem::_stat(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    auto [node, mount, tail] = walk(path);
    return node ? FileStat(FILE_DIRECTORY, 0) : mount ? mount->stat(tail) : FileStat();
}

void MountingFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    auto [node, mount, tail] = walk(path);

    if (!node && !mount)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);

    if (!node) {
        mount->ls(tail, entries);
        return;
    }

    if (!mount) {
        for (const auto &[name, _] : node->children())
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        return;
    }

    // Need to merge in this case.
    mount->ls(tail, entries);
    std::ranges::sort(*entries);
    std::span<DirectoryEntry> searchable = *entries;
    bool cleanupNeeded = false;
    for (const auto &[name, _] : node->children()) {
        auto range = std::ranges::equal_range(searchable, name, std::ranges::less(), &DirectoryEntry::name);

        size_t size = range.size();

        if (size == 0) {
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        } else if (size == 1) {
            range[0].type = FILE_DIRECTORY;
        } else {
            assert(size == 2); // Schrodingermaxxed fs, still should not have more than two identical entries.
            range[0].type = FILE_DIRECTORY;
            range[1].type = FILE_INVALID;
            cleanupNeeded = true;
        }
    }
    if (cleanupNeeded)
        std::erase_if(*entries, [] (const DirectoryEntry &entry) { return entry.type == FILE_INVALID; });
}

Blob MountingFileSystem::_read(const FileSystemPath &path) const {
    auto [mount, tail] = walkForReading(path);
    return mount->read(tail);
}

void MountingFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    auto [mount, tail] = walkForWriting(path);
    return mount->write(tail, data);
}

std::unique_ptr<InputStream> MountingFileSystem::_openForReading(const FileSystemPath &path) const {
    auto [mount, tail] = walkForReading(path);
    return mount->openForReading(tail);
}

std::unique_ptr<OutputStream> MountingFileSystem::_openForWriting(const FileSystemPath &path) {
    auto [mount, tail] = walkForWriting(path);
    return mount->openForWriting(tail);
}

void MountingFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    auto [srcNode, srcMount, srcTail] = walk(srcPath);
    auto [dstNode, dstMount, dstTail] = walk(dstPath);

    if (srcNode)
        throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);
    if (dstNode)
        throw FileSystemException(FileSystemException::RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (!srcMount)
        throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (!dstMount)
        throw FileSystemException(FileSystemException::RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    if (srcMount == dstMount) {
        srcMount->rename(srcTail, dstTail);
    } else {
        // Just forward to recursive copy & remove. Every call will resolve the mount points again and again, so
        // suboptimal, but OK for now.
        FileSystem::_rename(srcPath, dstPath);
    }
}

bool MountingFileSystem::_remove(const FileSystemPath &path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        throw FileSystemException(FileSystemException::REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!mount)
        return false; // Nothing to remove.
    return mount->remove(tail);
}

std::string MountingFileSystem::_displayPath(const FileSystemPath &path) const {
    return _displayName + "://" + path.string();
}

MountingFileSystem::WalkResult MountingFileSystem::walk(const FileSystemPath &path) {
    Node *node = _trie.root();
    FileSystem *mount = node->hasValue() ? node->value() : nullptr;
    if (path.isEmpty())
        return {node, mount, FileSystemPath()};

    std::string_view mountChunk;
    for (std::string_view chunk : path.chunks()) {
        node = node->child(chunk);
        if (!node)
            break;
        if (node->hasValue()) {
            mount = node->value();
            mountChunk = chunk;
        }
    }

    if (mount) {
        return {node, mount, path.tailAfter(mountChunk)};
    } else {
        return {node, nullptr, FileSystemPath()};
    }
}

MountingFileSystem::ConstWalkResult MountingFileSystem::walk(const FileSystemPath &path) const {
    return const_cast<MountingFileSystem *>(this)->walk(path);
}

std::pair<const FileSystem *, FileSystemPath> MountingFileSystem::walkForReading(const FileSystemPath &path) const {
    auto [node, mount, tail] = walk(path);
    if (node)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_IS_DIR, path);
    if (!mount)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    return {mount, std::move(tail)};
}

std::pair<FileSystem *, FileSystemPath> MountingFileSystem::walkForWriting(const FileSystemPath &path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_IS_DIR, path);
    if (!mount)
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_NOT_WRITEABLE, path); // No mount point => can't write.
    return {mount, std::move(tail)};
}
