#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/FileSystem/Interface/FileSystemPath.h"

/**
 * File system that provides a view into a subdirectory of another file system.
 *
 * All paths are prefixed with the base path. This provides isolation - files outside the base path cannot be accessed.
 *
 * Usage:
 * ```
 * FileSystem *root = ...;
 * SubFileSystem shaders(root, "shaders/");
 * shaders.read("precision.vert");  // reads root/shaders/precision.vert
 * ```
 */
class SubFileSystem : public FileSystem {
 public:
    /**
     * @param basePath          Path prefix to apply to all operations.
     * @param base              The underlying file system.
     */
    SubFileSystem(FileSystemPathView basePath, FileSystem *base);
    SubFileSystem(std::string_view basePath, FileSystem *base);

 protected:
    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual void _write(FileSystemPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(FileSystemPathView path) override;
    virtual void _rename(FileSystemPathView srcPath, FileSystemPathView dstPath) override;
    virtual bool _remove(FileSystemPathView path) override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

 private:
    FileSystem *_base;
    FileSystemPath _basePath;
};
