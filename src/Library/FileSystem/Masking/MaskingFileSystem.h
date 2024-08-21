#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

// TODO(captainurist): This was originally designed as a workaround to make rename() work on top of a mergingfs looking
//                     into two different filesystems, one of which is readonly. This made little sense, so mergingfs
//                     was changed to be readonly, thus the original use case is no more. This class needs to be redone
//                     with just mask / unmask / clearMasks, masking out parts of the underlying FS makes sense in some
//                     cases.

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
    virtual void _ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

 private:
    FileSystem *_base = nullptr;
    FileSystemTrie<bool> _masks;
};
