#include "MemoryFileSystem.h"

#include <vector>
#include <memory>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "MemoryFileSystemInputStream.h"
#include "MemoryFileSystemOutputStream.h"

void MemoryFileSystem::clear() {
    _trie.clear();
}

bool MemoryFileSystem::_exists(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    return _trie.find(path) != nullptr;
}

FileStat MemoryFileSystem::_stat(const FileSystemPath &path) const {
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

std::vector<DirectoryEntry> MemoryFileSystem::_ls(const FileSystemPath &path) const {
    const Node *node = _trie.find(path);
    if (!node)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->hasValue())
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_IS_FILE, path);

    std::vector<DirectoryEntry> result;
    for (const auto &[name, child] : node->children())
        result.push_back(DirectoryEntry(name, child->hasValue() ? FILE_REGULAR : FILE_DIRECTORY));
    return result;
}

Blob MemoryFileSystem::_read(const FileSystemPath &path) const {
    return Blob::share(nodeForReading(path)->value()->blob);
}

void MemoryFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    nodeForWriting(path)->value()->blob = Blob::share(data);
}

std::unique_ptr<InputStream> MemoryFileSystem::_openForReading(const FileSystemPath &path) const {
    return std::make_unique<detail::MemoryFileSystemInputStream>(nodeForReading(path)->value());
}

std::unique_ptr<OutputStream> MemoryFileSystem::_openForWriting(const FileSystemPath &path) {
    return std::make_unique<detail::MemoryFileSystemOutputStream>(nodeForWriting(path)->value());
}

void MemoryFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    assert(!srcPath.isEmpty());
    assert(!dstPath.isEmpty());

    Node *srcNode = _trie.find(srcPath);
    if (!srcNode)
        throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);

    FileSystemPath dstTail;
    Node *dstNode = _trie.walk(dstPath, &dstTail);
    if (dstTail.isEmpty()) { // dstPath exists.
        if (!dstNode->hasValue())
            throw FileSystemException(FileSystemException::RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
        if (!srcNode->hasValue())
            throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    }

    _trie.insertOrAssign(dstNode, dstTail, _trie.extract(srcNode));
}

bool MemoryFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());

    Node *node = _trie.find(path);
    if (!node)
        return false;

    return _trie.erase(node);
}

const MemoryFileSystem::Node *MemoryFileSystem::nodeForReading(const FileSystemPath &path) const {
    assert(!path.isEmpty());
    const Node *node = _trie.find(path);
    if (!node)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    if (!node->hasValue())
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_IS_DIR, path);
    if (node->value()->writerCount)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_NOT_READABLE, path);
    return node;
}

MemoryFileSystem::Node *MemoryFileSystem::nodeForWriting(const FileSystemPath &path) {
    assert(!path.isEmpty());

    FileSystemPath tail;
    Node *node = _trie.walk(path, &tail);

    if (!tail.isEmpty()) { // File doesn't exist.
        if (node->hasValue()) { // But we have an existing file as parent.
            throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_NOT_WRITEABLE, path);
        } else {
            return _trie.insertOrAssign(node, tail, std::make_shared<MemoryFileData>(Blob()));
        }
    }

    if (!node->hasValue())
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_IS_DIR, path);

    if (node->value()->readerCount || node->value()->writerCount)
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    return node;
}

