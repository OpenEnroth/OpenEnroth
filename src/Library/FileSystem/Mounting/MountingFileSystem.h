#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <utility>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

/**
 * File system that makes it possible to mount other file systems at different points of the file tree.
 *
 * This can be handy if, for example, you need to override one of the folders of some other file system:
 * ```
 * SomeFileSystem fs1;
 * MemoryFileSystem fs2;
 *
 * MountingFileSystem fs;
 * fs.mount("", &fs1);
 * fs.mount("data/images", &fs2); // Now "data/images" is pointing into `fs2`, whatever was there in `fs1` is ignored.
 * ```
 *
 * In the example above, repeated calls to `mount` create a hierarchy of virtual directories. These virtual directories
 * override everything with the same name that existed in the underlying file systems.
 *
 * Note that `remove` only works on regular files, and will fail on virtual directories. If you want to tweak the
 * virtual directory tree, use `mount` and `unmount`.
 */
class MountingFileSystem : public FileSystem {
 public:
    explicit MountingFileSystem(std::string_view displayName);
    virtual ~MountingFileSystem();

    void mount(std::string_view path, FileSystem *fileSystem);
    void mount(PathView path, FileSystem *fileSystem);
    bool unmount(std::string_view path);
    bool unmount(PathView path);

    void clearMounts();

 private:
    virtual bool _exists(NormalPathView path) const override;
    virtual FileStat _stat(NormalPathView path) const override;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(NormalPathView path) const override;
    virtual void _write(NormalPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) override;
    virtual bool _remove(NormalPathView path) override;
    virtual std::string _displayPath(NormalPathView path) const override;

 private:
    using Node = FileSystemTrieNode<FileSystem *>;
    using WalkResult = std::tuple<Node *, FileSystem *, NormalPathView>;
    using ConstWalkResult = std::tuple<const Node *, const FileSystem *, NormalPathView>;

    WalkResult walk(NormalPathView path);
    ConstWalkResult walk(NormalPathView path) const;

    std::pair<const FileSystem *, NormalPathView> walkForReading(NormalPathView path) const;
    std::pair<FileSystem *, NormalPathView> walkForWriting(NormalPathView path);

 private:
    std::string _displayName;
    FileSystemTrie<FileSystem *> _trie;
};
