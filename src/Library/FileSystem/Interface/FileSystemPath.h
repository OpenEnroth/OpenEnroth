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
        assert(normalizePath(path) == path);

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

    [[nodiscard]] bool isParentOf(FileSystemPathView child) const {
        return FileSystemPathView(*this).isParentOf(child);
    }

    [[nodiscard]] bool isChildOf(FileSystemPathView parent) const {
        return parent.isParentOf(*this);
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

    void append(std::string_view chunk) {
        assert(chunk.empty() || (chunk.find('\\') == std::string_view::npos && chunk.find('/') == std::string_view::npos && chunk != "." && chunk != ".."));

        if (!_path.empty() && !chunk.empty())
            _path += '/';
        _path += chunk;
    }

    void append(FileSystemPathView tail);

    // TODO(captainurist): name this one better, it takes a chunk, not a path that needs to be re-normalized.
    [[nodiscard]] FileSystemPath appended(std::string_view chunk) const {
        FileSystemPath result = *this;
        result.append(chunk);
        return result;
    }

    [[nodiscard]] FileSystemPath appended(FileSystemPathView tail) const {
        FileSystemPath result = *this;
        result.append(tail);
        return result;
    }

 private:
    [[nodiscard]] static std::string normalizePath(std::string_view path);

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
