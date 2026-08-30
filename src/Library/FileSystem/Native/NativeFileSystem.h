#pragma once

#include <vector>
#include <memory>
#include <string>
#include <string_view>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/System/Path.h"

/**
 * View over a directory on the native file system of the host OS.
 *
 * All methods will work as if the root directory of this `FileSystem` exists, even if it doesn't exist on the
 * underlying file system.
 *
 * Files outside of the root directory are not observable through this `FileSystem` - the methods will behave as if
 * these files don't exist. That buys nothing for a file system rooted at the root of the native one - everything is
 * inside it, so `remove("home")` is `rm -rf /home`, and it is only the base class that stops `remove("")`.
 *
 * When it comes to permissions, this filesystem tries its best to provide a simple guarantee that `ls` for an existing
 * folder never throws, and is in sync with what `stat` / `exists` return. This means that some files or folders that
 * are observable through `ls` in bash won't be observable through this filesystem.
 *
 * On Windows file names are encoded as WTF8, so that names containing unpaired surrogates round-trip. A name is then
 * not necessarily valid UTF8, but that's also the case on *nix, where file names are just bytes.
 */
class NativeFileSystem : public FileSystem {
 public:
    /**
     * @param root                      Native path to the root directory, absolute or relative. A relative path is
     *                                  resolved against the current working directory, and an empty path means the
     *                                  current working directory itself.
     */
    explicit NativeFileSystem(const Path &root);
    virtual ~NativeFileSystem();

    /**
     * Maps a path in this file system back into a native path. The passed path is not required to exist.
     *
     * @param path                      Path in this file system.
     * @return                          Native path for `path`. Always absolute.
     */
    [[nodiscard]] Path toNativePath(std::string_view path) const;
    [[nodiscard]] Path toNativePath(PathView path) const;

 private:
    // Composes a path in this file system onto the root, without validating it. Callers inside this class have
    // already been through the public boundary, and _displayPath must work on a path that was just rejected.
    [[nodiscard]] Path basePath(PathView path) const;

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

 private:
    Path _root;
};
