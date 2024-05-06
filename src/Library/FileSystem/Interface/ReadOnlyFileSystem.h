#pragma once

#include <memory>

#include "FileSystem.h"

/**
 * Base class for read-only file systems.
 */
class ReadOnlyFileSystem : public FileSystem {
 private:
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;

    [[noreturn]] void reportWriteError(const FileSystemPath &path) const;
};
