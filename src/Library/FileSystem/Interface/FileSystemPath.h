#pragma once

#include <cassert>
#include <compare>
#include <string>
#include <string_view>
#include <utility>

#include "FileSystemPathView.h"

/**
 * `FileSystemPath` does path normalization for `FileSystem`, transforming passed paths into a normal form of
 * `"/foo/bar/baz"`:
 * - The path is absolute, starting with `/`, which is the root of the file system. A relative path is taken
 *   relative to the root. The root itself is `"/"`.
 * - Windows-style path separators are replaced with unix-style ones.
 * - Repeated path separators are replaced with a single one, and a trailing one is removed.
 * - `"."` and `".."` are collapsed, and `".."` stops at the root, so `"/../a"` is `"/a"`.
 *
 * Note that this means that `FileSystemPath` is different from `std::filesystem::path` in that there is no difference
 * between a file and a directory at the path level. `"/foo/bar/."` is normalized into `"/foo/bar"`, and can then be
 * used to open a file for reading.
 */
class FileSystemPath {
 public:
    /**
     * @param path                      Path to normalize. A relative one is taken relative to the root.
     */
    explicit FileSystemPath(std::string_view path);

    explicit FileSystemPath(FileSystemPathView path) : _path(path.string()) {}

    /**
     * Constructs a root path.
     */
    FileSystemPath() = default;

    FileSystemPath(const FileSystemPath &) = default;
    FileSystemPath(FileSystemPath &&) = default;
    FileSystemPath &operator=(const FileSystemPath &) = default;
    FileSystemPath &operator=(FileSystemPath &&) = default;

    friend auto operator<=>(const FileSystemPath &l, const FileSystemPath &r) = default;

    [[nodiscard]] bool isRoot() const {
        return _path == "/";
    }

    [[nodiscard]] const std::string &string() const {
        return _path;
    }

    [[nodiscard]] auto split() const {
        return FileSystemPathView(*this).split();
    }

    /**
     * @return                          The last component, empty for the root. `"/a/b.txt"` gives `"b.txt"`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @return                          The extension of the file name, with the leading dot, empty if there is none.
     *                                  A leading dot doesn't start one, so `".bashrc"` has no extension, and only the
     *                                  last one counts, so `"a.tar.gz"` gives `".gz"`. Same as `std::filesystem`,
     *                                  which means `"..."` decomposes into a stem of `".."` and an extension of `"."`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view extension() const;

    /**
     * @return                          The file name without its extension. `"/a/b.tar.gz"` gives `"b.tar"`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view stem() const;

    /**
     * @return                          Parent path, or the root for the root.
     */
    [[nodiscard]] FileSystemPath parent() const;

    /**
     * @param tail                      Path to append, taken relative to this path whether or not it starts with a
     *                                  separator.
     */
    FileSystemPath &operator/=(std::string_view tail);
    FileSystemPath &operator/=(FileSystemPathView tail);

 private:
    std::string _path = "/";
};

template<>
struct std::hash<FileSystemPath> : std::hash<std::string> {
    using base_type = std::hash<std::string>;

    auto operator()(const FileSystemPath &path) const {
        return base_type::operator()(path.string());
    }
};

inline FileSystemPathView::FileSystemPathView(const FileSystemPath &path) : _path(path.string()) {}

inline FileSystemPath operator/(FileSystemPathView l, FileSystemPathView r) {
    FileSystemPath result(l);
    result /= r;
    return result;
}

inline FileSystemPath operator/(FileSystemPathView l, std::string_view r) {
    FileSystemPath result(l);
    result /= r;
    return result;
}
