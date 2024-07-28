#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

/**
 * Proxy read-only filesystem that supports `remove` operations by simply masking away the removed files and
 * directories.
 *
 * Note that other mutating operations (`write`, `openForWriting` and `rename`) will still throw.
 */
class MaskingFileSystem : public ReadOnlyFileSystem {
 public:
    explicit MaskingFileSystem(FileSystem *base);
    virtual ~MaskingFileSystem();

    void mask(std::string_view path);
    void mask(const FileSystemPath &path);
    void clearMasks();

 private:
    bool isMasked(const FileSystemPath &path) const;

    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual std::vector<DirectoryEntry> _ls(const FileSystemPath &path) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

 private:
    FileSystem *_base = nullptr;
    FileSystemTrie<bool> _masks;
};
