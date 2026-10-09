#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/OutputStream.h"
#include "Utility/System/FileStat.h"
#include "Utility/System/Path.h"

#include "FileSystemEnums.h"
#include "NormalPath.h"
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
 * All user-facing methods take paths as UTF-8 encoded `std::string_view`s, and users are expected to just use
 * `std::string`s to store paths.
 *
 * Paths are normalized internally, and then processed by the implementation in a derived class. A backslash is a
 * separator on every platform. Both `".."` and `"."` special dirs are supported, but peeking outside the root directory
 * is not. Passing such a path will throw, `exists` will return `false`, and `stat` will return `FILE_INVALID`.
 *
 * Unlike a real file system, this interface doesn't have a concept of a "current directory." All methods take paths
 * relative to the root, and a path with a root of its own, like `"/foo/bar"` or `"C:/foo"` on Windows, is refused the
 * same way.
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
     * @param path                      Path inside this file system. The passed path is not required to exist.
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

    /**
     * Calls into another file system, for file systems that delegate to one. These do what the public methods do,
     * answering for the root included, but skip normalizing and validating the path. `NormalPathView` takes care of
     * the normal form, and the caller has to pass an accessible path, like the one its own `_` method got or a tail
     * or a join of it. They are static because a derived class can only reach a protected member through its own
     * type.
     */
    static bool existsIn(const FileSystem *fs, NormalPathView path);
    static FileStat statIn(const FileSystem *fs, NormalPathView path);
    static void lsIn(const FileSystem *fs, NormalPathView path, std::vector<DirectoryEntry> *entries);
    static Blob readIn(const FileSystem *fs, NormalPathView path);
    static void writeIn(FileSystem *fs, NormalPathView path, const Blob &data);
    static std::unique_ptr<InputStream> openForReadingIn(const FileSystem *fs, NormalPathView path);
    static std::unique_ptr<OutputStream> openForWritingIn(FileSystem *fs, NormalPathView path);
    static bool removeIn(FileSystem *fs, NormalPathView path);
    static std::string displayPathIn(const FileSystem *fs, NormalPathView path);

 protected:
    [[nodiscard]] virtual bool _exists(NormalPathView path) const = 0;
    [[nodiscard]] virtual FileStat _stat(NormalPathView path) const = 0;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const = 0;
    [[nodiscard]] virtual Blob _read(NormalPathView path) const = 0;
    virtual void _write(NormalPathView path, const Blob &data) = 0;
    [[nodiscard]] virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const = 0;
    [[nodiscard]] virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) = 0;
    virtual bool _remove(NormalPathView path) = 0;
    [[nodiscard]] virtual std::string _displayPath(NormalPathView path) const = 0;
};


