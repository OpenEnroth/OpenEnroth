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
    try {
        _trie.clear();
        refresh(_trie.root(), FileSystemPath());
    } catch (...) {
        // refresh() failed - just clear everything & rethrow. This can happen because of conflicts, or if the
        // underlying file system is acting up.
        _trie.clear();
        throw;
    }
}

bool LowercaseFileSystem::_exists(const FileSystemPath &path) const {
    return _trie.find(path) != nullptr;
}

FileStat LowercaseFileSystem::_stat(const FileSystemPath &path) const {
    const auto [basePath, node, tail] = walk(path);
    return !tail.isEmpty() ? FileStat() : _base->stat(basePath);
}

std::vector<DirectoryEntry> LowercaseFileSystem::_ls(const FileSystemPath &path) const {
    const Node *node = _trie.find(path);
    if (!node)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->children().empty() && node != _trie.root())
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_IS_FILE, path);

    std::vector<DirectoryEntry> result;
    for (const auto &[name, child] : node->children())
        result.push_back(DirectoryEntry(name, child->children().empty() ? FILE_REGULAR : FILE_DIRECTORY));
    return result;
}

Blob LowercaseFileSystem::_read(const FileSystemPath &path) const {
    return _base->read(locateForReading(path));
}

void LowercaseFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    _base->write(basePath, data);
    blaze(node, tail); // If write() above throws then we won't store anything in cache, as intended.
}

std::unique_ptr<InputStream> LowercaseFileSystem::_openForReading(const FileSystemPath &path) const {
    return _base->openForReading(locateForReading(path));
}

std::unique_ptr<OutputStream> LowercaseFileSystem::_openForWriting(const FileSystemPath &path) {
    const auto &[basePath, node, tail] = locateForWriting(path);
    std::unique_ptr<OutputStream> result = _base->openForWriting(basePath);
    blaze(node, tail); // If openForWriting() above throws then we won't store anything in cache, as intended.
    return result;
}

void LowercaseFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    if (hasUpper(dstPath.string()))
        throw FileSystemException(FileSystemException::RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);

    auto [srcBasePath, srcNode, srcTail] = walk(srcPath);
    if (!srcTail.isEmpty())
        throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_DOESNT_EXIST, srcPath, dstPath);

    auto [dstBasePath, dstNode, dstTail] = walk(dstPath);
    if (!dstNode->children().empty() && dstTail.isEmpty())
        throw FileSystemException(FileSystemException::RENAME_FAILED_DST_IS_DIR, srcPath, dstPath);
    if (!srcNode->children().empty() && dstTail.isEmpty())
        throw FileSystemException(FileSystemException::RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE, srcPath, dstPath);

    dstBasePath.append(dstTail);
    _base->rename(srcBasePath, dstBasePath);

    // If rename() above throws we'll just assume that nothing was renamed. This might not be the case - if we were
    // renaming a folder between different file systems, then some files might have been renamed. We don't try to be
    // smart here, it's up to the user to call refresh() in this case.
    Node *srcParent = srcNode->parent();
    assert(srcParent);
    dstNode = blaze(dstNode, dstTail);
    srcNode->value() = dstNode->value();
    _trie.insertOrAssign(dstNode, FileSystemPath(), _trie.extract(srcNode));
    prune(srcParent);
}

bool LowercaseFileSystem::_remove(const FileSystemPath &path) {
    assert(!path.isEmpty());

    Node *node = _trie.find(path);
    if (!node)
        return false;

    // Return value doesn't matter here, from this file system's pov we are deleting an existing entry.
    _base->remove(node->value());

    // If remove() above throws we'll just assume that nothing was removed. This might not be the case - if we were
    // removing a folder, then some files might have been removed. We don't try to be smart here, it's up to the
    // user to call refresh() in this case.
    Node *parent = node->parent();
    assert(parent);
    _trie.erase(node);
    prune(parent);

    return true;
}

std::string LowercaseFileSystem::_displayPath(const FileSystemPath &path) const {
    auto [basePath, node, tail] = walk(path);
    return _base->displayPath(basePath.appended(tail));
}

void LowercaseFileSystem::refresh(Node *node, const FileSystemPath &basePath) {
    std::vector<DirectoryEntry> entries = _base->ls(basePath);

    for (DirectoryEntry &entry : entries) {
        std::string lowerEntryName = ascii::toLower(entry.name);

        if (node->children().contains(lowerEntryName))
            throw Exception("Can't refresh a lowercase filesystem because paths '{}' and '{}' are conflicting.",
                            basePath.appended(entry.name),
                            basePath.appended(node->child(lowerEntryName)->value()));

        Node *child = _trie.insertOrAssign(node, FileSystemPath::fromNormalized(lowerEntryName), entry.name);

        if (entry.type == FILE_DIRECTORY) {
            refresh(child, basePath.appended(entry.name));

            // We don't preserve empty folders, and the implementation relies on the fact that all leaf nodes are files.
            // So we need to maintain this invariant.
            //
            // Coincidentally, this means that we won't throw on two conflicting empty dirs. Well, who needs them
            // anyway?
            if (child->children().empty())
                _trie.erase(child);
        }
    }
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPath> LowercaseFileSystem::walk(const FileSystemPath &path) {
    Node *node = _trie.root();
    if (path.isEmpty())
        return {FileSystemPath(), node, FileSystemPath()};

    FileSystemPath basePath;
    for (std::string_view chunk : path.chunks()) {
        Node *child = node->child(chunk);
        if (!child)
            return {std::move(basePath), node, path.tailAt(chunk)};

        node = child;
        basePath.append(child->value());
    }

    return {std::move(basePath), node, FileSystemPath()};
}

std::tuple<FileSystemPath, const LowercaseFileSystem::Node *, FileSystemPath> LowercaseFileSystem::walk(const FileSystemPath &path) const {
    return const_cast<LowercaseFileSystem *>(this)->walk(path);
}

LowercaseFileSystem::Node *LowercaseFileSystem::blaze(Node *node, const FileSystemPath &tail) { // Trailblazers lol.
    for (std::string_view chunk : tail.chunks())
        node = _trie.insertOrAssign(node, FileSystemPath::fromNormalized(std::string(chunk)), std::string(chunk));
    return node;
}

void LowercaseFileSystem::prune(Node *node) {
    while (node && node->children().empty()) {
        Node *parent = node->parent();
        _trie.erase(node);
        node = parent;
    }
}

FileSystemPath LowercaseFileSystem::locateForReading(const FileSystemPath &path) const {
    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    if (!node->children().empty())
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_IS_DIR, path);
    return std::move(basePath);
}

std::tuple<FileSystemPath, LowercaseFileSystem::Node *, FileSystemPath> LowercaseFileSystem::locateForWriting(const FileSystemPath &path) {
    if (hasUpper(path.string()))
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_NOT_WRITEABLE, path);

    auto result = walk(path);
    auto &[basePath, node, tail] = result;

    if (tail.isEmpty() && !node->children().empty())
        throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_IS_DIR, path);
    if (!tail.isEmpty() && node->children().empty())
        throw FileSystemException(FileSystemException::WRITE_FAILED_FILE_IN_PATH, path);

    basePath.append(tail);
    return result;
}





