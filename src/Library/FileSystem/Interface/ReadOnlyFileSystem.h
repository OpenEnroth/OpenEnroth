#pragma once

#include <memory>

#include "FileSystem.h"

/**
 * Base class for read-only file systems.
 */
class ReadOnlyFileSystem : public FileSystem {
 private:
    virtual void _write(PathView path, const Blob &data) override;
    virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) override;
    virtual bool _remove(PathView path) override;

    [[noreturn]] void reportWriteError(PathView path) const;
};
