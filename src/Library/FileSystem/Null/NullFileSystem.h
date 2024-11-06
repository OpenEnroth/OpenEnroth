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
    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

    [[noreturn]] void reportReadError(FileSystemPathView path) const;
};
