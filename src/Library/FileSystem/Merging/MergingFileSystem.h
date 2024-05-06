#pragma once

#include <vector>
#include <string_view>
#include <span>
#include <memory>

#include "Library/FileSystem/Interface/FileSystem.h"

/**
 * Merges several filesystems into one:
 * - Read operations go one by one over the underlying filesystems. This effectively means that files on filesystem #0
 *   will hide the files on filesystem #1 that have the same names.
 * - Write operations are always writing into filesystem #0.
 * - Remove removes from all filesystems.
 * - Rename tries to forward the call to filesystem #0 if possible, then removes the source file from all filesystems.
 */
class MergingFileSystem : public FileSystem {
 public:
    explicit MergingFileSystem(std::span<FileSystem *> bases);
    virtual ~MergingFileSystem();

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

    FileSystem *locateForReading(const FileSystemPath &path, bool shouldExist) const;

 private:
    std::vector<FileSystem *> _bases;
};
