#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Library/FileSystem/Interface/FileSystem.h"

/**
 * File system that provides a view into a subdirectory of another file system.
 *
 * All paths are prefixed with the base path. Files outside the base path can't be reached.
 *
 * `exists`, `stat` and `ls` of `"."` work as if the base path were a directory, even if it's missing or a file. So
 * `ls(".")` returns an empty list for such a base path instead of throwing.
 *
 * Usage:
 * ```
 * FileSystem *root = ...;
 * SubFileSystem shaders("shaders", root);
 * shaders.read("precision.vert");  // reads root/shaders/precision.vert
 * ```
 */
class SubFileSystem : public FileSystem {
 public:
    /**
     * @param basePath                  Path prefix to apply to all operations.
     * @param base                      The underlying file system.
     * @throws Exception                If `basePath` is not accessible, see `NormalPathView::isAccessible`.
     */
    SubFileSystem(PathView basePath, FileSystem *base);

 protected:
    virtual bool _exists(NormalPathView path) const override;
    virtual FileStat _stat(NormalPathView path) const override;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(NormalPathView path) const override;
    virtual void _write(NormalPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) override;
    virtual bool _remove(NormalPathView path) override;
    virtual std::string _displayPath(NormalPathView path) const override;

 private:
    [[nodiscard]] NormalPath basePath(NormalPathView path) const;

 private:
    FileSystem *_base = nullptr;
    NormalPath _basePath;
};
