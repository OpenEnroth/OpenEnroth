#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <utility>

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
 * Note that `rename` and `remove` only work on regular files, and will fail on virtual directories. If you want to
 * tweak the virtual directory tree, use `mount` and `unmount`.
 */
class MountingFileSystem : public FileSystem {
 public:
    MountingFileSystem();
    virtual ~MountingFileSystem();

    void mount(std::string_view path, FileSystem *fileSystem);
    void mount(const FileSystemPath &path, FileSystem *fileSystem);
    bool unmount(std::string_view path);
    bool unmount(const FileSystemPath &path);

    void clearMounts();

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual std::vector<DirectoryEntry> _ls(const FileSystemPath &path) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;

 private:
    using Node = FileSystemTrieNode<FileSystem *>;
    using WalkResult = std::tuple<Node *, FileSystem *, FileSystemPath>;
    using ConstWalkResult = std::tuple<const Node *, const FileSystem *, FileSystemPath>;

    WalkResult walk(const FileSystemPath &path);
    ConstWalkResult walk(const FileSystemPath &path) const;

    std::pair<const FileSystem *, FileSystemPath> walkForReading(const FileSystemPath &path) const;
    std::pair<FileSystem *, FileSystemPath> walkForWriting(const FileSystemPath &path);

 private:
    FileSystemTrie<FileSystem *> _trie;
};
