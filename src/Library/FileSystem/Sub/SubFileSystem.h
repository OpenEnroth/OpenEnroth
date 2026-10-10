#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Utility/System/Path.h"

/**
 * File system that provides a view into a subdirectory of another file system.
 *
 * All paths are prefixed with the base path. This provides isolation - files outside the base path cannot be accessed.
 *
 * All methods will work as if the base path exists on the underlying file system, even if it doesn't. So `ls("")`
 * returns an empty list instead of throwing for a base path that doesn't exist, or that points to a file.
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
     * @throws Exception                If `basePath` has a root or escapes the root of `base`.
     */
    SubFileSystem(PathView basePath, FileSystem *base);
    SubFileSystem(std::string_view basePath, FileSystem *base);

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
