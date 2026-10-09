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
    virtual bool _exists(NormalPathView path) const override;
    virtual FileStat _stat(NormalPathView path) const override;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(NormalPathView path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const override;
    virtual std::string _displayPath(NormalPathView path) const override;

    [[noreturn]] void reportReadError(PathView path) const;
};
