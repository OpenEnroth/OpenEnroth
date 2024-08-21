#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

/**
 * Empty read-only filesystem.
 */
class NullFileSystem : public ReadOnlyFileSystem {
 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual void _ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

    [[noreturn]] void reportReadError(const FileSystemPath &path) const;
};
