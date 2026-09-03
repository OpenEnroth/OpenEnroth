#include "MountingFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

MountingFileSystem::MountingFileSystem(std::string_view displayName) : _displayName(displayName) {}
MountingFileSystem::~MountingFileSystem() = default;

void MountingFileSystem::mount(std::string_view path, FileSystem *fileSystem) {
    mount(Path(path).normalized(), fileSystem);
}

void MountingFileSystem::mount(PathView path, FileSystem *fileSystem) {
    assert(path.isNormalized()); // The tries are walked with normal paths, so a non-normal key could never match.
    _trie.insertOrAssign(path, fileSystem);
}

bool MountingFileSystem::unmount(std::string_view path) {
    return unmount(Path(path).normalized());
}

bool MountingFileSystem::unmount(PathView path) {
    assert(path.isNormalized()); // The tries are walked with normal paths, so a non-normal key could never match.
    Node *node = _trie.find(path);
    if (!node || !node->hasValue())
        return false; // Should be a real mount point, unmount("") is not equivalent to clearMounts().

    return _trie.erase(node);
}

void MountingFileSystem::clearMounts() {
    _trie.clear();
}

bool MountingFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());

    auto [node, mount, tail] = walk(path);
    return node ? true : mount ? existsOf(mount, tail) : false;
}

FileStat MountingFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    auto [node, mount, tail] = walk(path);
    return node ? FileStat(FILE_DIRECTORY, 0) : mount ? statOf(mount, tail) : FileStat();
}

void MountingFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    auto [node, mount, tail] = walk(path);

    if (!node && !mount)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    if (!node) {
        lsOf(mount, tail, entries);
        return;
    }

    if (!mount) {
        for (const auto &[name, _] : node->children())
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        return;
    }

    // Need to merge in this case.
    lsOf(mount, tail, entries);
    std::ranges::sort(*entries);
    size_t originalSize = entries->size(); // lsOf appends, so the merge below works on what we added.
    bool cleanupNeeded = false;
    for (const auto &[name, _] : node->children()) {
        auto range = std::ranges::equal_range(
            entries->begin(), entries->begin() + originalSize, name, std::ranges::less(), &DirectoryEntry::name);

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

Blob MountingFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    auto [mount, tail] = walkForReading(path);
    return readOf(mount, tail);
}

void MountingFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    auto [mount, tail] = walkForWriting(path);
    return writeOf(mount, tail, data);
}

std::unique_ptr<InputStream> MountingFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    auto [mount, tail] = walkForReading(path);
    return openForReadingOf(mount, tail);
}

std::unique_ptr<OutputStream> MountingFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    auto [mount, tail] = walkForWriting(path);
    return openForWritingOf(mount, tail);
}

bool MountingFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!mount)
        return false; // Nothing to remove.
    return removeOf(mount, tail);
}

std::string MountingFileSystem::_displayPath(PathView path) const {
    // TODO(captainurist): this is not symmetric with that's done in read / openForReading / openForWriting.
    return join(_displayName, "://", path.displayString());
}

MountingFileSystem::WalkResult MountingFileSystem::walk(PathView path) {
    Node *node = _trie.root();
    FileSystem *mount = node->hasValue() ? node->value() : nullptr;
    if (path.isEmpty())
        return {node, mount, {}};

    std::string_view mountChunk;
    for (std::string_view chunk : path.split()) {
        node = node->child(chunk);
        if (!node)
            break;
        if (node->hasValue()) {
            mount = node->value();
            mountChunk = chunk;
        }
    }

    if (mount) {
        return {node, mount, path.split().tailAfter(mountChunk)};
    } else {
        return {node, nullptr, {}};
    }
}

MountingFileSystem::ConstWalkResult MountingFileSystem::walk(PathView path) const {
    return const_cast<MountingFileSystem *>(this)->walk(path);
}

std::pair<const FileSystem *, PathView> MountingFileSystem::walkForReading(PathView path) const {
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!mount)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return {mount, std::move(tail)};
}

std::pair<FileSystem *, PathView> MountingFileSystem::walkForWriting(PathView path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!mount)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path); // No mount point => can't write.
    return {mount, std::move(tail)};
}
