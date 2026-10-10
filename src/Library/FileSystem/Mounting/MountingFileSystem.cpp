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
    mount(Path(path), fileSystem);
}

void MountingFileSystem::mount(PathView path, FileSystem *fileSystem) {
    NormalPath normalPath(path);
    assert(normalPath.isAccessible());
    _trie.insertOrAssign(normalPath.path(), fileSystem);
}

bool MountingFileSystem::unmount(std::string_view path) {
    return unmount(Path(path));
}

bool MountingFileSystem::unmount(PathView path) {
    NormalPath normalPath(path);
    if (!normalPath.isAccessible())
        return false; // mount refuses these, so nothing is mounted there.

    Node *node = _trie.find(normalPath.path());
    if (!node || !node->hasValue())
        return false; // Should be a real mount point, unmount("") is not equivalent to clearMounts().

    return _trie.erase(node);
}

void MountingFileSystem::clearMounts() {
    _trie.clear();
}

bool MountingFileSystem::_exists(NormalPathView path) const {
    assert(!path.isEmpty());

    auto [node, mount, tail] = walk(path);
    return node ? true : mount ? existsIn(mount, tail) : false;
}

FileStat MountingFileSystem::_stat(NormalPathView path) const {
    assert(!path.isEmpty());
    auto [node, mount, tail] = walk(path);
    return node ? FileStat(FILE_DIRECTORY, 0) : mount ? statIn(mount, tail) : FileStat();
}

void MountingFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    auto [node, mount, tail] = walk(path);

    if (!node && !mount)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    if (!node) {
        lsIn(mount, tail, entries);
        return;
    }

    if (!mount) {
        for (const auto &[name, _] : node->children())
            entries->push_back(DirectoryEntry(name, FILE_DIRECTORY));
        return;
    }

    // Need to merge in this case.
    lsIn(mount, tail, entries);
    std::ranges::sort(*entries);
    size_t originalSize = entries->size();
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

Blob MountingFileSystem::_read(NormalPathView path) const {
    auto [mount, tail] = walkForReading(path);
    return readIn(mount, tail);
}

void MountingFileSystem::_write(NormalPathView path, const Blob &data) {
    auto [mount, tail] = walkForWriting(path);
    writeIn(mount, tail, data);
}

std::unique_ptr<InputStream> MountingFileSystem::_openForReading(NormalPathView path) const {
    auto [mount, tail] = walkForReading(path);
    return openForReadingIn(mount, tail);
}

std::unique_ptr<OutputStream> MountingFileSystem::_openForWriting(NormalPathView path) {
    auto [mount, tail] = walkForWriting(path);
    return openForWritingIn(mount, tail);
}

bool MountingFileSystem::_remove(NormalPathView path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!mount)
        return false; // Nothing to remove.
    return removeIn(mount, tail);
}

std::string MountingFileSystem::_displayPath(NormalPathView path) const {
    // TODO(captainurist): this is not symmetric with that's done in read / openForReading / openForWriting.
    return join(_displayName, "://", txt::encodedToUtf8(path.str(), ENCODING_UTF8)); // Replaces invalid UTF8.
}

MountingFileSystem::WalkResult MountingFileSystem::walk(NormalPathView path) {
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
        return {node, mount, path.tailAfter(mountChunk)};
    } else {
        return {node, nullptr, {}};
    }
}

MountingFileSystem::ConstWalkResult MountingFileSystem::walk(NormalPathView path) const {
    return const_cast<MountingFileSystem *>(this)->walk(path);
}

std::pair<const FileSystem *, NormalPathView> MountingFileSystem::walkForReading(NormalPathView path) const {
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!mount)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return {mount, std::move(tail)};
}

std::pair<FileSystem *, NormalPathView> MountingFileSystem::walkForWriting(NormalPathView path) {
    auto [node, mount, tail] = walk(path);
    if (node)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!mount)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path); // No mount point => can't write.
    return {mount, std::move(tail)};
}
