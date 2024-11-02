#pragma once

#include <cassert>
#include <compare>
#include <string>
#include <string_view>
#include <utility>

#include "FileSystemPathView.h"

/**
 * `FileSystemPath` does path normalization for `FileSystem`, transforming passed paths into a normal form of
 * `"foo/bar/baz"`:
 * - Windows-style path separators are replaced with unix-style ones.
 * - Repeated path separators are replaced with a single one.
 * - Leading & trailing path separators are removed.
 * - `"."` and `".."` are collapsed.
 * - Root path is stored as an empty string.
 *
 * Note that this means that `FileSystemPath` is different from `std::filesystem::path` in that there is no difference
 * between a file and a directory at the path level. `"foo/bar/."` is normalized into `"foo/bar"`, and can then be used
 * to open a file for reading.
 */
class FileSystemPath {
 public:
    explicit FileSystemPath(std::string_view path);

    explicit FileSystemPath(FileSystemPathView path) : _path(path.string()) {}

    static FileSystemPath fromNormalized(std::string path) {
        assert(FileSystemPath(path).string() == path);

        FileSystemPath result;
        result._path = std::move(path);
        return result;
    }

    /**
     * Constructs a root path.
     */
    FileSystemPath() = default;

    FileSystemPath(const FileSystemPath &) = default;
    FileSystemPath(FileSystemPath &&) = default;
    FileSystemPath &operator=(const FileSystemPath &) = default;
    FileSystemPath &operator=(FileSystemPath &&) = default;

    friend auto operator<=>(const FileSystemPath &l, const FileSystemPath &r) = default;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    [[nodiscard]] bool isPrefixOf(FileSystemPathView path) const {
        return FileSystemPathView(*this).isPrefixOf(path);
    }

    [[nodiscard]] bool isEscaping() const {
        return FileSystemPathView(*this).isEscaping();
    }

    [[nodiscard]] const std::string &string() const {
        return _path;
    }

    [[nodiscard]] auto split() const {
        return FileSystemPathView(*this).split();
    }

    [[nodiscard]] FileSystemPathComponents components() const {
        return FileSystemPathView(*this).components();
    }

    FileSystemPath &operator/=(std::string_view tail);
    FileSystemPath &operator/=(FileSystemPathView tail);

 private:
    std::string _path;
};

template<>
struct std::hash<FileSystemPath> : std::hash<std::string> {
    using base_type = std::hash<std::string>;

    auto operator()(const FileSystemPath &path) const {
        return base_type::operator()(path.string());
    }
};

inline FileSystemPathView::FileSystemPathView(const FileSystemPath &path) : _path(path.string()) {}

inline FileSystemPathView FileSystemPathView::fromNormalized(std::string_view path) {
    assert(FileSystemPath(path).string() == path); // Should be normalized.

    FileSystemPathView result;
    result._path = path;
    return result;
}

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
