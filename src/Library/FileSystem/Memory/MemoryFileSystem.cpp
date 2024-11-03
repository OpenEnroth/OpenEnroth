#include "MemoryFileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Join.h"

#include "MemoryFileSystemInputStream.h"
#include "MemoryFileSystemOutputStream.h"

MemoryFileSystem::MemoryFileSystem(std::string_view displayName) : _displayName(displayName) {}

void MemoryFileSystem::clear() {
    _trie.clear();
}

bool MemoryFileSystem::_exists(FileSystemPathView path) const {
    assert(!path.isEmpty());
    return _trie.find(path) != nullptr;
}

FileStat MemoryFileSystem::_stat(FileSystemPathView path) const {
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

void MemoryFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    const Node *node = _trie.find(path);
    if (!node)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->hasValue())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->hasValue() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob MemoryFileSystem::_read(FileSystemPathView path) const {
    return Blob::share(nodeForReading(path)->value()->blob);
}

void MemoryFileSystem::_write(FileSystemPathView path, const Blob &data) {
    nodeForWriting(path)->value()->blob = Blob::share(data).withDisplayPath(displayPath(path));
}

std::unique_ptr<InputStream> MemoryFileSystem::_openForReading(FileSystemPathView path) const {
    return std::make_unique<detail::MemoryFileSystemInputStream>(nodeForReading(path)->value());
}

std::unique_ptr<OutputStream> MemoryFileSystem::_openForWriting(FileSystemPathView path) {
    return std::make_unique<detail::MemoryFileSystemOutputStream>(nodeForWriting(path)->value(), displayPath(path));
}

void MemoryFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    Node *srcNode = _trie.find(srcPath);
    if (!srcNode)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);

    FileSystemPathView dstTail;
    Node *dstNode = _trie.walk(dstPath, &dstTail);
    if (dstTail.isEmpty()) { // dstPath exists.
        if (!dstNode->hasValue())
            FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
        if (!srcNode->hasValue())
            FileSystemException::raise(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    }

    _trie.insertOrAssign(dstNode, dstTail, _trie.extract(srcNode));
}

bool MemoryFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());

    Node *node = _trie.find(path);
    if (!node)
        return false;

    return _trie.erase(node);
}

std::string MemoryFileSystem::_displayPath(FileSystemPathView path) const {
    return join(_displayName, "://", path.string());
}

const MemoryFileSystem::Node *MemoryFileSystem::nodeForReading(FileSystemPathView path) const {
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

MemoryFileSystem::Node *MemoryFileSystem::nodeForWriting(FileSystemPathView path) {
    assert(!path.isEmpty());

    FileSystemPathView tail;
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

