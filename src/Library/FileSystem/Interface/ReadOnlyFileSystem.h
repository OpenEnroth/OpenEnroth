#pragma once

#include <memory>

#include "FileSystem.h"

/**
 * Base class for read-only file systems.
 */
class ReadOnlyFileSystem : public FileSystem {
 private:
    virtual void _write(FileSystemPathView path, const Blob &data) override;
    virtual std::unique_ptr<OutputStream> _openForWriting(FileSystemPathView path) override;
    virtual void _rename(FileSystemPathView srcPath, FileSystemPathView dstPath) override;
    virtual bool _remove(FileSystemPathView path) override;

    [[noreturn]] void reportWriteError(FileSystemPathView path) const;
};
