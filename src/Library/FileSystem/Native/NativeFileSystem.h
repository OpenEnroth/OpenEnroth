#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/System/Path.h"

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
class NativeFileSystem : public FileSystem {
 public:
    explicit NativeFileSystem(const Path &root);
    virtual ~NativeFileSystem();

 private:
    virtual bool _exists(PathView path) const override;
    virtual FileStat _stat(PathView path) const override;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(PathView path) const override;
    virtual void _write(PathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(PathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) override;
    virtual bool _remove(PathView path) override;
    virtual std::string _displayPath(PathView path) const override;

    Path makeBasePath(PathView path) const;

 private:
    Path _root;
};
