#include "LowercaseFileSystem.h"

#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <ranges>
#include <tuple>

#include "Library/FileSystem/Interface/FileSystemException.h"
#include "Library/FileSystem/Proxy/ProxyFileSystem.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

static bool hasUpper(std::string_view s) {
    return std::ranges::any_of(s, &ascii::isUpper);
}

LowercaseFileSystem::LowercaseFileSystem(FileSystem *base): _base(base) {
    assert(_base);
    refresh();
}

LowercaseFileSystem::~LowercaseFileSystem() = default;

void LowercaseFileSystem::refresh() {
    _trie.clear();
    _trie.insertOrAssign({}, detail::LowercaseFileData(FILE_DIRECTORY, ""));
}

bool LowercaseFileSystem::_exists(FileSystemPathView path) const {
    const auto [basePath, node, tail] = walk(path);
    return tail.isEmpty();
}

FileStat LowercaseFileSystem::_stat(FileSystemPathView path) const {
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return FileStat();
    if (node->value().conflicting)
        return FileStat(FILE_REGULAR, 0); // Conflicts are reported as empty files.
    return _base->stat(basePath);
}

void LowercaseFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);

    cacheLs(node, basePath);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->value().type));
}

Blob LowercaseFileSystem::_read(FileSystemPathView path) const {
    return _base->read(locateForReading(path));
}

void LowercaseFileSystem::_write(FileSystemPathView path, const Blob &data) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    _base->write(basePath, data);
    cacheInsert(node, tail, FILE_REGULAR);
}

std::unique_ptr<InputStream> LowercaseFileSystem::_openForReading(FileSystemPathView path) const {
    return _base->openForReading(locateForReading(path));
}

std::unique_ptr<OutputStream> LowercaseFileSystem::_openForWriting(FileSystemPathView path) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    std::unique_ptr<OutputStream> result = _base->openForWriting(basePath);
    cacheInsert(node, tail, FILE_REGULAR);
    return result;
}

void LowercaseFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    if (hasUpper(dstPath.string()))
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    auto [srcBasePath, srcNode, srcTail] = walk(srcPath);
    if (!srcTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (srcNode->value().conflicting)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);

    auto [dstBasePath, dstNode, dstTail] = walk(dstPath);
    if (dstNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (srcNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);
    if (dstNode->value().conflicting)
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    dstBasePath /= dstTail;
    try {
        _base->rename(srcBasePath, dstBasePath);
    } catch (...) {
        // We have no idea about the state of the underlying FS now. Don't bother checking, just invalidate the caches.
        invalidateLs(srcNode->parent());
        invalidateLs(dstTail.isEmpty() ? dstNode->parent() : dstNode);
        throw;
    }

    cacheInsert(dstNode, dstTail, srcNode->value().type);
    cacheRemove(srcNode);
}

bool LowercaseFileSystem::_remove(FileSystemPathView path) {
    assert(!path.isEmpty());

    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return false;

    if (node->value().conflicting)
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);

    try {
        // Return value doesn't matter here, from this file system's pov we are deleting an existing entry.
        _base->remove(basePath);
    } catch (...) {
        // Exception should mean that the file/folder wasn't removed. However, if it's a folder then some of the files
        // might have been removed, so we need to invalidate the caches in this case.
        if (node->value().type == FILE_DIRECTORY)
            invalidateLs(node);
        throw;
    }

    cacheRemove(node);
    return true;
}

std::string LowercaseFileSystem::_displayPath(FileSystemPathView path) const {
    auto [basePath, node, tail] = walk(path);
    return _base->displayPath(basePath / tail);
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPathView> LowercaseFileSystem::walk(FileSystemPathView path) const {
    Node *node = _trie.root();
    if (path.isEmpty())
        return {FileSystemPath(), node, FileSystemPathView()};

    FileSystemPath basePath;
    for (std::string_view chunk : path.split()) {
        if (node->value().type != FILE_DIRECTORY)
            return {std::move(basePath), node, path.split().tailAt(chunk)};

        cacheLs(node, basePath);

        Node *child = node->child(chunk);
        if (!child)
            return {std::move(basePath), node, path.split().tailAt(chunk)};

        node = child;
        basePath /= child->value().baseName;
    }

    return {std::move(basePath), node, FileSystemPathView()};
}

void LowercaseFileSystem::cacheLs(Node *node, FileSystemPathView basePath) const {
    assert(node->value().type == FILE_DIRECTORY);

    if (node->value().listed)
        return;

    std::vector<DirectoryEntry> entries = _base->ls(basePath);
    for (DirectoryEntry &entry : entries) {
        std::string lowerEntryName = ascii::toLower(entry.name);

        auto pos = node->children().find(lowerEntryName);
        if (pos != node->children().end()) {
            pos->second->value().type = FILE_REGULAR;
            pos->second->value().conflicting = true;
            continue;
        }

        _trie.insertOrAssign(node,
                             FileSystemPathView::fromNormalized(lowerEntryName),
                             detail::LowercaseFileData(entry.type, std::move(entry.name)));
    }

    node->value().listed = true;
}

void LowercaseFileSystem::invalidateLs(Node *node) const {
    assert(node->value().type == FILE_DIRECTORY);

    node->value().listed = false;
    _trie.chop(node);
}

void LowercaseFileSystem::cacheRemove(Node *node) const {
    Node *prev = node;
    Node *next = node->parent();

    while (next->children().size() == 1 && next != _trie.root()) {
        prev = next;
        next = next->parent();
    }

    if (prev == node) {
        _trie.erase(node);
    } else {
        // We don't know if the underlying FS keeps empty folders or not, so we just invalidate the caches. We might drop
        // more than we really should, but the alternative approach here is to call ProxyFileSystem::exists, and we need
        // to construct a base path for that... just not worth it.
        invalidateLs(next);
    }
}

void LowercaseFileSystem::cacheInsert(Node *node, FileSystemPathView tail, FileType type) const {
    if (tail.isEmpty())
        return;

    assert(node->value().type == FILE_DIRECTORY);

    auto chunks = tail.split();
    auto pos = chunks.begin();
    auto end = chunks.end();

    std::string_view firstChunk = *pos;
    ++pos;
    assert(!node->children().contains(firstChunk));

    FileType nodeType = pos == end ? type : FILE_DIRECTORY;
    _trie.insertOrAssign(node,
                         FileSystemPathView::fromNormalized(firstChunk),
                         detail::LowercaseFileData(nodeType, std::string(firstChunk)));
}

FileSystemPath LowercaseFileSystem::locateForReading(FileSystemPathView path) const {
    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (node->value().conflicting)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_READABLE, path);
    return std::move(basePath);
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPathView> LowercaseFileSystem::locateForWriting(FileSystemPathView path) {
    if (hasUpper(path.string()))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    auto result = walk(path);
    auto &[basePath, node, tail] = result;

    if (tail.isEmpty() && node->value().type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!tail.isEmpty() && node->value().type == FILE_REGULAR)
        FileSystemException::raise(this, FS_WRITE_FAILED_FILE_IN_PATH, path);
    if (node->value().conflicting)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    basePath /= tail;
    return result;
}
