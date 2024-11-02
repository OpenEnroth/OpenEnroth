#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"

/**
 * View over a directory on a file system.
 *
 * All methods will work as if the root directory of this `FileSystem` exists, even if it doesn't exist on the
 * underlying file system.
 *
 * Files outside of the root directory are not observable through this `FileSystem` - the methods will behave as if
 * these files don't exist.
 *
 * When it comes to permissions, this filesystem tries its best to provide a simple guarantee that `ls` for an existing
 * folder never throws, and is in sync with what `stat` / `exists` return. This means that some files or folders that
 * are observable through `ls` in bash won't be observable through this filesystem.
 */
class DirectoryFileSystem : public FileSystem {
 public:
    explicit DirectoryFileSystem(std::string_view root);
    virtual ~DirectoryFileSystem();

 private:
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

    std::filesystem::path makeBasePath(FileSystemPathView path) const;

 private:
    std::string _originalRoot;
    std::filesystem::path _root;
};
