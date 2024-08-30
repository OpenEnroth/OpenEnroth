#include "LowercaseFileSystem.h"

#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <ranges>
#include <tuple>

#include "Library/FileSystem/Interface/FileSystemException.h"

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
    _trie.insertOrAssign(FileSystemPath(), detail::LowercaseFileData(FILE_DIRECTORY, ""));
}

bool LowercaseFileSystem::_exists(const FileSystemPath &path) const {
    const auto [basePath, node, tail] = walk(path);
    return tail.isEmpty();
}

FileStat LowercaseFileSystem::_stat(const FileSystemPath &path) const {
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return FileStat();
    if (node->value().type == FILE_INVALID)
        return FileStat(FILE_REGULAR, 0); // Conflict detected, report it as an empty file.
    return _base->stat(basePath);
}

void LowercaseFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);

    cacheLs(node, basePath);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->value().type == FILE_INVALID ? FILE_REGULAR : child->value().type));
}

Blob LowercaseFileSystem::_read(const FileSystemPath &path) const {
    return _base->read(locateForReading(path));
}

void LowercaseFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    _base->write(basePath, data);
    cacheInsert(node, tail, FILE_REGULAR);
}

std::unique_ptr<InputStream> LowercaseFileSystem::_openForReading(const FileSystemPath &path) const {
    return _base->openForReading(locateForReading(path));
}

std::unique_ptr<OutputStream> LowercaseFileSystem::_openForWriting(const FileSystemPath &path) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    std::unique_ptr<OutputStream> result = _base->openForWriting(basePath);
    cacheInsert(node, tail, FILE_REGULAR);
    return result;
}

void LowercaseFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    if (hasUpper(dstPath.string()))
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    auto [srcBasePath, srcNode, srcTail] = walk(srcPath);
    if (!srcTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);
    if (srcNode->value().type == FILE_INVALID)
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_NOT_WRITEABLE, srcPath, dstPath);

    auto [dstBasePath, dstNode, dstTail] = walk(dstPath);
    if (dstNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (dstNode->value().type == FILE_INVALID && dstTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath); // Conflict detected.
    if (srcNode->value().type == FILE_DIRECTORY && dstTail.isEmpty())
        FileSystemException::raise(this, FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);

    dstBasePath.append(dstTail);
    _base->rename(srcBasePath, dstBasePath);

    // If rename() above throws we'll just assume that nothing was renamed. This might not be the case - if we were
    // renaming a folder between different file systems, then some files might have been renamed. We don't try to be
    // smart here, it's up to the user to call refresh() in this case.
    cacheInsert(dstNode, dstTail, srcNode->value().type);
    cacheRemove(srcNode);
}

bool LowercaseFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());

    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return false;

    if (node->value().type == FILE_INVALID)
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path); // Conflict detected.

    // Return value doesn't matter here, from this file system's pov we are deleting an existing entry.
    _base->remove(basePath);

    // If remove() above throws we'll just assume that nothing was removed. This might not be the case - if we were
    // removing a folder, then some files might have been removed. We don't try to be smart here, it's up to the
    // user to call refresh() in this case.
    cacheRemove(node);
    return true;
}

std::string LowercaseFileSystem::_displayPath(const FileSystemPath &path) const {
    auto [basePath, node, tail] = walk(path);
    return _base->displayPath(basePath.appended(tail));
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPath> LowercaseFileSystem::walk(const FileSystemPath &path) const {
    Node *node = _trie.root();
    if (path.isEmpty())
        return {FileSystemPath(), node, FileSystemPath()};

    FileSystemPath basePath;
    for (std::string_view chunk : path.chunks()) {
        if (node->value().type != FILE_DIRECTORY)
            return {std::move(basePath), node, path.tailAt(chunk)};

        cacheLs(node, basePath);

        Node *child = node->child(chunk);
        if (!child)
            return {std::move(basePath), node, path.tailAt(chunk)};

        node = child;
        basePath.append(child->value().baseName);
    }

    return {std::move(basePath), node, FileSystemPath()};
}

void LowercaseFileSystem::cacheLs(Node *node, const FileSystemPath &basePath) const {
    assert(node->value().type == FILE_DIRECTORY);

    if (node->value().listed)
        return;

    std::vector<DirectoryEntry> entries = _base->ls(basePath);
    for (DirectoryEntry &entry : entries) {
        std::string lowerEntryName = ascii::toLower(entry.name);

        auto pos = node->children().find(lowerEntryName);
        if (pos != node->children().end()) {
            pos->second->value().type = FILE_INVALID; // Conflict detected.
            continue;
        }

        _trie.insertOrAssign(node,
                             FileSystemPath::fromNormalized(std::move(lowerEntryName)),
                             detail::LowercaseFileData(entry.type, std::move(entry.name)));
    }

    node->value().listed = true;
}

void LowercaseFileSystem::cacheRemove(Node *node) const {
    do {
        Node *parent = node->parent();
        _trie.erase(node);
        node = parent;
    } while (node && node->children().empty() && node != _trie.root());
}

void LowercaseFileSystem::cacheInsert(Node *node, const FileSystemPath &tail, FileType type) const {
    if (tail.isEmpty())
        return;

    assert(node->value().type == FILE_DIRECTORY);

    auto chunks = tail.chunks();
    auto pos = chunks.begin();
    auto end = chunks.end();

    std::string_view firstChunk = *pos;
    ++pos;
    assert(!node->children().contains(firstChunk));

    FileType nodeType = pos == end ? type : FILE_DIRECTORY;
    _trie.insertOrAssign(node,
                         FileSystemPath::fromNormalized(std::string(firstChunk)),
                         detail::LowercaseFileData(nodeType, std::string(firstChunk)));
}

FileSystemPath LowercaseFileSystem::locateForReading(const FileSystemPath &path) const {
    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (node->value().type == FILE_INVALID)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_READABLE, path); // Conflicting paths are not readable.
    return std::move(basePath);
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPath> LowercaseFileSystem::locateForWriting(const FileSystemPath &path) {
    if (hasUpper(path.string()))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    auto result = walk(path);
    auto &[basePath, node, tail] = result;

    if (tail.isEmpty() && node->value().type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!tail.isEmpty() && node->value().type == FILE_REGULAR)
        FileSystemException::raise(this, FS_WRITE_FAILED_FILE_IN_PATH, path);
    if (node->value().type == FILE_INVALID)
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path); // Conflicting paths are not writeable.

    basePath.append(tail);
    return result;
}
