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
 */
class DirectoryFileSystem : public FileSystem {
 public:
    explicit DirectoryFileSystem(std::string_view root);
    virtual ~DirectoryFileSystem();

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual void _ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

    std::filesystem::path makeBasePath(const FileSystemPath &path) const;

 private:
    std::string _originalRoot;
    std::filesystem::path _root;
};
