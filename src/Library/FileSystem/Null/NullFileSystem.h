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
    virtual bool _exists(PathView path) const override;
    virtual FileStat _stat(PathView path) const override;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(PathView path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(PathView path) const override;
    virtual std::string _displayPath(PathView path) const override;

    [[noreturn]] void reportReadError(PathView path) const;
};
