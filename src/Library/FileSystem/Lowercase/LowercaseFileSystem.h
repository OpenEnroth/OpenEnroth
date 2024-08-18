#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <tuple>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

/**
 * Provides a lowercase view over another `FileSystem`:
 * - Contains only lowercase-named files.
 * - Effectively gives the user a case-insensitive view over a potentially case-sensitive filesystem.
 * - Caches the contents of the underlying filesystem in constructor. Call `refresh` to update the cache.
 * - Constructor and `refresh` throw on conflicts (e.g. if both "file.txt" and "FILE.txt" exist).
 * - Doesn't preserve empty folders because who needs them?
 *
 * Some notes on why it is designed the way it is designed. Previous iteration was a writeable FS that was always up
 * to date with the underlying FS (was checking timestamps for all parents in each call). The code was messy, it was
 * `O(N)` on each call, and even after writing an implementation that actually worked, I hated it. Mostly because of the 
 * `O(N)` - which led to a recursive implementation of `FileSystem::rename` becoming `O(N^2)`. It is possible to iron
 * this out, but that would require to redo the `FileSystem` interface & expose the `Directory` abstraction directly. 
 * Which is just not worth it.
 * 
 * If you've been paying attention, you might want to point out that recursive `FileSystem::rename` is `O(N^2)` anyway. 
 * Indeed, we'll be traversing the trie for each file, and that's `O(N)` per file. However, there is no amplification 
 * of the number of calls into the underlying file system, which is the important part. Each of the methods in 
 * `LowercaseFileSystem` calls into the underlying file system at most once.
 */
class LowercaseFileSystem : public FileSystem {
 public:
    explicit LowercaseFileSystem(FileSystem *base);
    virtual ~LowercaseFileSystem();

    void refresh();

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual void _ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

 private:
    using Node = FileSystemTrieNode<std::string>;

    void refresh(Node *node, const FileSystemPath &basePath);

    std::tuple<FileSystemPath, Node *, FileSystemPath> walk(const FileSystemPath &path);
    std::tuple<FileSystemPath, const Node *, FileSystemPath> walk(const FileSystemPath &path) const;
    Node *blaze(Node *node, const FileSystemPath &tail);
    void prune(Node *node);

    FileSystemPath locateForReading(const FileSystemPath &path) const;
    std::tuple<FileSystemPath, Node *, FileSystemPath> locateForWriting(const FileSystemPath &path);

 private:
    FileSystem *_base = nullptr;
    FileSystemTrie<std::string> _trie; // Stores names in the base filesystem, leaf nodes are files.
};
