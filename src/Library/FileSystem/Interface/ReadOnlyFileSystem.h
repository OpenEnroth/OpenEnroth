#pragma once

#include <memory>

#include "FileSystem.h"

/**
 * Base class for read-only file systems.
 */
class ReadOnlyFileSystem : public FileSystem {
 private:
    virtual void _write(NormalPathView path, const Blob &data) override;
    virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) override;
    virtual bool _remove(NormalPathView path) override;

    [[noreturn]] void reportWriteError(PathView path) const;
};
