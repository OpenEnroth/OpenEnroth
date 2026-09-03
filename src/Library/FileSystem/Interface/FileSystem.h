#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <compare>

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/OutputStream.h"
#include "Utility/System/FileStat.h"

#include "Utility/System/PathView.h"
#include "FileSystemEnums.h"
#include "FileSystemFwd.h"

// TODO(captainurist): I still think most of FSs should inherit from ProxyFS.
//
// TODO(captainurist): Masking is for the portable mode. Mask out non-relevant parts from the corresponding FSs.
//
// TODO(captainurist): internal functions should NOT throw? Honestly, makes a lot of sense. Just throw in FileSystem impl!
//                     Is it OK for the underlying functions to throw? I think no. All exceptions should be `FileSystemException`s.
//                     Then I'll also be able to drop the exists() -> ls() paired calls that are inherently race-y.
//
// TODO(captainurist): _ls(vector*) should append, not overwrite.

/**
 * File system interface.
 *
 * All user-facing methods take paths as WTF-8 encoded `std::string_view`s, and users are expected to just use
 * `std::string`s to store paths. Note that this is for paths inside a `FileSystem` only - native paths are always
 * passed around as `Path` objects.
 *
 * Paths are normalized internally, and then processed by the implementation in a derived class. Both `".."` and `"."`
 * special dirs are supported, but peeking outside the root directory is not - passing paths that try to do this will
 * throw, `exists` will return `false`, and `stat` will return `FILE_INVALID`.
 *
 * Unlike a real file system, this interface doesn't have a concept of a "current directory." All methods take
 * paths relative to the root, and an absolute path is refused rather than reinterpreted - `"/foo/bar"` names a
 * location of its own, which is not something this file system can reach.
 *
 * Root folder of the file system always exists. Thus, `exists("")` always returns `true`, `stat("")` always returns
 * `FILE_DIRECTORY`, `ls("")` never throws, and `remove("")` always throws.
 *
 * @see ReadOnlyFileSystem
 */
class FileSystem {
 public:
    FileSystem() = default;
    virtual ~FileSystem() = default;

    /**
     * @param path                      Path to check.
     * @return                          Whether the given path exists.
     * @throws std::runtime_error       On error, e.g. if the current user doesn't have the necessary permissions.
     */
    [[nodiscard]] bool exists(std::string_view path) const;
    [[nodiscard]] bool exists(PathView path) const;

    /**
     * @param path                      Path to a file of a folder to get information for.
     * @return                          Information for a file or directory at `path`. `FileStat::type` will be set to
     *                                  `FILE_INVALID` if `path` doesn't exist.
     * @throws std::runtime_error       On error, e.g. if the current user doesn't have the necessary permissions.
     */
    [[nodiscard]] FileStat stat(std::string_view path) const;
    [[nodiscard]] FileStat stat(PathView path) const;

    /**
     * @param path                      Path to an existing directory to list.
     * @return                          List of directory entries.
     * @throws std::runtime_error       If `path` doesn't exist, or on any other error.
     */
    [[nodiscard]] std::vector<DirectoryEntry> ls(std::string_view path) const;
    [[nodiscard]] std::vector<DirectoryEntry> ls(PathView path) const;
    void ls(std::string_view path, std::vector<DirectoryEntry> *entries) const;
    void ls(PathView path, std::vector<DirectoryEntry> *entries) const;

    /**
     * @param path                      Path to an existing file to read or map into memory.
     * @return                          File contents. Implementations are encouraged to use memory mapping.
     * @throws std::runtime_error       If `path` doesn't exist, or on any other error.
     */
    [[nodiscard]] Blob read(std::string_view path) const;
    [[nodiscard]] Blob read(PathView path) const;

    /**
     * @param path                      Path to a file to write. If parent directory doesn't exist, it will be created.
     *                                  If a file with the provided name exists, it will be overwritten.
     * @param data                      File contents to write.
     * @throws std::runtime_error       On error, e.g. if the current user doesn't have the necessary permissions.
     */
    void write(std::string_view path, const Blob &data);
    void write(PathView path, const Blob &data);

    /**
     * @param path                      Path to an existing file to open for reading.
     * @return                          Input stream for reading from a file.
     * @throws std::runtime_error       If `path` doesn't exist, or on any other error.
     */
    [[nodiscard]] std::unique_ptr<InputStream> openForReading(std::string_view path) const;
    [[nodiscard]] std::unique_ptr<InputStream> openForReading(PathView path) const;

    /**
     * @param path                      Path to a file to write. If parent directory doesn't exist, it will be created.
     *                                  If a file with the provided name exists, it will be overwritten.
     * @return                          Output stream for writing into a file.
     * @throws std::runtime_error       On error, e.g. if the current user doesn't have the necessary permissions.
     */
    [[nodiscard]] std::unique_ptr<OutputStream> openForWriting(std::string_view path);
    [[nodiscard]] std::unique_ptr<OutputStream> openForWriting(PathView path);

    /**
     * @param path                      Path to a file or a directory to remove. A directory will be removed even if it
     *                                  is not empty. Must not be root.
     * @return                          `true` if the file or folder was deleted, `false` if it did not exist.
     * @throws std::runtime_error       On error, e.g. if the current user doesn't have the necessary permissions.
     */
    bool remove(std::string_view path);
    bool remove(PathView path);

    /**
     * Unlike every other method here, this one doesn't refuse an inaccessible path - error reporting formats the
     * path that was just rejected, so refusing one here would recurse. It still normalizes.
     *
     * @param path                      Path inside this file system. The passed path is not required to exist, or
     *                                  even to be accessible.
     * @return                          A path string that's suitable to be displayed to the user. E.g. an absolute path
     *                                  on the underlying OS file system. Always valid UTF-8, with everything that's
     *                                  not valid UTF-8 replaced with U+FFFD, so it might not map back to a real path.
     */
    [[nodiscard]] std::string displayPath(std::string_view path) const;
    [[nodiscard]] std::string displayPath(PathView path) const;

 protected:
    template<class T>
    using FileSystemTrieNode = detail::FileSystemTrieNode<T>;
    template<class T>
    using FileSystemTrie = detail::FileSystemTrie<T>;

    // A file system that delegates to another one reaches it through these, not through its public methods - the
    // argument already cleared the public boundary, so re-validating it would be wasted work. They have to be
    // static: [class.protected] only lets a derived class reach a protected member through its own type, so
    // `_base->_read(path)` doesn't compile while `readOf(_base, path)` does.
    // The empty path is the root, which every file system has and no backend implements - the public methods answer
    // for it before dispatching, so these two have to as well. The others don't: nothing reaches them with an empty
    // path, since every caller either prepends a non-empty prefix or has already refused the root.
    static bool existsOf(const FileSystem *fs, PathView path) { return path.isEmpty() || fs->_exists(path); }
    static FileStat statOf(const FileSystem *fs, PathView path) {
        return path.isEmpty() ? FileStat(FILE_DIRECTORY, 0) : fs->_stat(path);
    }
    static void lsOf(const FileSystem *fs, PathView path, std::vector<DirectoryEntry> *entries) { fs->_ls(path, entries); }
    static Blob readOf(const FileSystem *fs, PathView path) { return fs->_read(path); }
    static void writeOf(FileSystem *fs, PathView path, const Blob &data) { fs->_write(path, data); }
    static std::unique_ptr<InputStream> openForReadingOf(const FileSystem *fs, PathView path) { return fs->_openForReading(path); }
    static std::unique_ptr<OutputStream> openForWritingOf(FileSystem *fs, PathView path) { return fs->_openForWriting(path); }
    static bool removeOf(FileSystem *fs, PathView path) { return fs->_remove(path); }
    static std::string displayPathOf(const FileSystem *fs, PathView path) { return fs->_displayPath(path); }

 protected:
    [[nodiscard]] virtual bool _exists(PathView path) const = 0;
    [[nodiscard]] virtual FileStat _stat(PathView path) const = 0;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const = 0;
    [[nodiscard]] virtual Blob _read(PathView path) const = 0;
    virtual void _write(PathView path, const Blob &data) = 0;
    [[nodiscard]] virtual std::unique_ptr<InputStream> _openForReading(PathView path) const = 0;
    [[nodiscard]] virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) = 0;
    virtual bool _remove(PathView path) = 0;
    [[nodiscard]] virtual std::string _displayPath(PathView path) const = 0;
};


