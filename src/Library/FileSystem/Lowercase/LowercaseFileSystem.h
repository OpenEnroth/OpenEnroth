#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <utility>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

namespace detail {
struct LowercaseFileData {
    FileType type = FILE_INVALID;
    bool listed = false; // Only for `FILE_DIRECTORY`, means that `ls()` call was cached.
    bool conflicting = false; // Was there a conflict in the underlying FS? `type` should be set to `FILE_REGULAR`.
    std::string baseName;

    LowercaseFileData(FileType type, std::string baseName) : type(type), baseName(std::move(baseName)) {}
};
} // namespace detail

/**
 * Provides a lowercase view over another `FileSystem`:
 * - Contains only lowercase-named files.
 * - Effectively gives the user a case-insensitive view over a potentially case-sensitive filesystem.
 * - Caches the contents of the underlying filesystem. Call `refresh` to clear cache.
 * - Preserves empty folders that exist on the underlying FS, but also supports file systems that automatically prune
 *   empty folders.
 * - Conflicts are visible as empty files but are not readable / writeable / removeable (conflict is when both
 *   "file.txt" and "FILE.txt" exist).
 * - Is not thread-safe. Even const methods update the underlying cache.
 *
 * Some notes on why it is designed the way it is designed.
 *
 * Previous iteration was a writeable FS that was always up to date with the underlying FS (was checking timestamps for
 * all parents in each call). The code was messy, it was `O(N)` on each call, and even after writing an implementation
 * that actually worked, I hated it. Mostly because of the `O(N)` - which makes anything that walks a tree `O(N^2)`.
 * It is possible to iron this out, but that would require to redo the `FileSystem` interface & expose the `Directory`
 * abstraction directly. Which is just not worth it.
 *
 * Next iteration was caching the whole file tree in constructor, but this was blowing up spectacularly when constructed
 * for `/`. Thus, we're now caching lazily.
 *
 * The important point about the implementation is that there is no amplification of the number of calls into the
 * underlying file system. Each of the methods in `LowercaseFileSystem` calls into the underlying file system at most
 * once.
 */
class LowercaseFileSystem : public FileSystem {
 public:
    explicit LowercaseFileSystem(FileSystem *base);
    virtual ~LowercaseFileSystem();

    void refresh();

 private:
    virtual bool _exists(PathView path) const override;
    virtual FileStat _stat(PathView path) const override;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(PathView path) const override;
    virtual void _write(PathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(PathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) override;
    virtual bool _remove(PathView path) override;
    virtual std::string _displayPath(PathView path) const override;

 private:
    using Node = FileSystemTrieNode<detail::LowercaseFileData>;

    std::tuple<Path, Node *, PathView> walk(PathView path) const;
    void cacheLs(Node *node, PathView basePath) const;
    void invalidateLs(Node *node) const;
    void cacheRemove(Node *node) const;
    void cacheInsert(Node *node, PathView tail, FileType type) const;

    Path locateForReading(PathView path) const;
    std::tuple<Path, Node *, PathView> locateForWriting(PathView path);

 private:
    FileSystem *_base = nullptr;
    mutable FileSystemTrie<detail::LowercaseFileData> _trie; // Stores names in the base filesystem, leaf nodes are files.
};
