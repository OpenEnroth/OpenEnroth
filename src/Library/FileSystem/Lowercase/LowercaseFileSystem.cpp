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

bool LowercaseFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    const auto [basePath, node, tail] = walk(path);
    return tail.isEmpty();
}

FileStat LowercaseFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return FileStat();
    if (node->value().conflicting)
        return FileStat(FILE_REGULAR, 0); // Conflicts are reported as empty files.
    return statOf(_base, basePath);
}

void LowercaseFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    const auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type != FILE_DIRECTORY)
        FileSystemException::raise(this, FS_LS_FAILED_PATH_IS_FILE, path);

    cacheLs(node, basePath);

    for (const auto &[name, child] : node->children())
        entries->push_back(DirectoryEntry(name, child->value().type));
}

Blob LowercaseFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    return readOf(_base, locateForReading(path));
}

void LowercaseFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    const auto &[basePath, node, tail] = locateForWriting(path);
    writeOf(_base, basePath, data);
    cacheInsert(node, tail, FILE_REGULAR);
}

std::unique_ptr<InputStream> LowercaseFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    return openForReadingOf(_base, locateForReading(path));
}

std::unique_ptr<OutputStream> LowercaseFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    const auto &[basePath, node, tail] = locateForWriting(path);
    std::unique_ptr<OutputStream> result = openForWritingOf(_base, basePath);
    cacheInsert(node, tail, FILE_REGULAR);
    return result;
}

bool LowercaseFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    assert(!path.isEmpty());

    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        return false;

    if (node->value().conflicting)
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);

    try {
        // Return value doesn't matter here, from this file system's pov we are deleting an existing entry.
        removeOf(_base, basePath);
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

std::string LowercaseFileSystem::_displayPath(PathView path) const {
    auto [basePath, node, tail] = walk(path);
    return displayPathOf(_base, basePath / tail);
}

std::tuple<Path, LowercaseFileSystem::Node *, PathView> LowercaseFileSystem::walk(PathView path) const {
    Node *node = _trie.root();
    if (path.isEmpty())
        return {Path(), node, PathView()};

    Path basePath;
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

    return {std::move(basePath), node, PathView()};
}

void LowercaseFileSystem::cacheLs(Node *node, PathView basePath) const {
    assert(node->value().type == FILE_DIRECTORY);

    if (node->value().listed)
        return;

    std::vector<DirectoryEntry> entries;
    lsOf(_base, basePath, &entries);
    for (DirectoryEntry &entry : entries) {
        std::string lowerEntryName = ascii::toLower(entry.name);

        auto pos = node->children().find(lowerEntryName);
        if (pos != node->children().end()) {
            pos->second->value().type = FILE_REGULAR;
            pos->second->value().conflicting = true;
            continue;
        }

        _trie.insertOrAssign(node,
                             PathView::fromNormalized(lowerEntryName),
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

void LowercaseFileSystem::cacheInsert(Node *node, PathView tail, FileType type) const {
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
                         PathView::fromNormalized(firstChunk),
                         detail::LowercaseFileData(nodeType, std::string(firstChunk)));
}

Path LowercaseFileSystem::locateForReading(PathView path) const {
    auto [basePath, node, tail] = walk(path);
    if (!tail.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    if (node->value().type == FILE_DIRECTORY)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (node->value().conflicting)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_READABLE, path);
    return std::move(basePath);
}

std::tuple<Path, LowercaseFileSystem::Node *, PathView> LowercaseFileSystem::locateForWriting(PathView path) {
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
