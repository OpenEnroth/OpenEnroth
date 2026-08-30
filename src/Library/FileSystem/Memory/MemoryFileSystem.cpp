#include "MemoryFileSystem.h"

#include <cassert>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

#include "MemoryFileSystemInputStream.h"
#include "MemoryFileSystemOutputStream.h"

MemoryFileSystem::MemoryFileSystem(std::string_view displayName) : _displayName(displayName) {}

void MemoryFileSystem::clear() {
    _trie.clear();
}

bool MemoryFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());
    return _trie.find(path) != nullptr;
}

FileStat MemoryFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    assert(!path.isEmpty());

    const Node *node = _trie.find(path);
    if (!node)
        return {};

    if (node->hasValue()) {
        return FileStat(FILE_REGULAR, node->value()->blob.size());
    } else {
        return FileStat(FILE_DIRECTORY, 0);
    }
}

void MemoryFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    const Node *node = _trie.find(path);
    if (!node)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->hasValue())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->hasValue() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob MemoryFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    // We mimic how Windows handles file mapping here - treating mapped files as if they are open for reading.
    std::shared_ptr<MemoryFileData> data = nodeForReading(path)->value();
    data->readerCount++;
    std::shared_ptr<void> guard(nullptr, [data](void *) { data->readerCount--; });
    return Blob::custom(data->blob.data(), data->blob.size(), std::move(guard)).withDisplayPath(displayPath(path));
}

void MemoryFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    nodeForWriting(path)->value()->blob = Blob::share(data).withDisplayPath(displayPath(path));
}

std::unique_ptr<InputStream> MemoryFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    return std::make_unique<detail::MemoryFileSystemInputStream>(nodeForReading(path)->value());
}

std::unique_ptr<OutputStream> MemoryFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    return std::make_unique<detail::MemoryFileSystemOutputStream>(nodeForWriting(path)->value(), displayPath(path));
}

bool MemoryFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    assert(!path.isEmpty());

    Node *node = _trie.find(path);
    if (!node)
        return false;

    return _trie.erase(node);
}

std::string MemoryFileSystem::_displayPath(PathView path) const {
    return join(_displayName, "://", path.displayString());
}

const MemoryFileSystem::Node *MemoryFileSystem::nodeForReading(PathView path) const {
    assert(!path.isEmpty());
    const Node *node = _trie.find(path);
    if (!node)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    if (!node->hasValue())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (node->value()->writerCount)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_READABLE, path);
    return node;
}

MemoryFileSystem::Node *MemoryFileSystem::nodeForWriting(PathView path) {
    assert(!path.isEmpty());

    PathView tail;
    Node *node = _trie.walk(path, &tail);

    if (!tail.isEmpty()) { // File doesn't exist.
        if (node->hasValue()) {
            FileSystemException::raise(this, FS_WRITE_FAILED_FILE_IN_PATH, path);
        } else {
            return _trie.insertOrAssign(node, tail, std::make_shared<MemoryFileData>(Blob()));
        }
    }

    if (!node->hasValue())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);

    if (node->value()->readerCount || node->value()->writerCount)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    return node;
}

