#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/System/NativePath.h"

/**
 * View over a directory on the native file system of the host OS.
 *
 * All methods will work as if the root directory of this `FileSystem` exists, even if it doesn't exist on the
 * underlying file system.
 *
 * Files outside of the root directory are not observable through this `FileSystem` - the methods will behave as if
 * these files don't exist. That buys nothing for a file system rooted at the root of the native one, which is what
 * `fromNativePath` returns - everything is inside it, so `remove("home")` is `rm -rf /home`, and it is only the base
 * class that stops `remove("")`.
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
    explicit NativeFileSystem(const NativePath &root);
    virtual ~NativeFileSystem();

    /**
     * Splits a native path into a file system rooted at the root of the native file system, and a path inside it.
     *
     * Rooting at the very root keeps all of the path components, so that the result can be walked down looking for an
     * archive. On Windows the root is a drive or a UNC share. Not sandboxed, see above.
     *
     * @param path                      Native path, absolute or relative. Empty means the current working directory.
     * @return                          File system rooted at the native root, and `path` relative to it. Never
     *                                  escaping.
     * @throws Exception                If `path` couldn't be resolved.
     */
    [[nodiscard]] static std::pair<std::unique_ptr<NativeFileSystem>, FileSystemPath> fromNativePath(
        const NativePath &path);

 private:
    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual void _write(FileSystemPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(FileSystemPathView path) override;
    virtual bool _remove(FileSystemPathView path) override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

    NativePath makeBasePath(FileSystemPathView path) const;

 private:
    NativePath _root;
};
