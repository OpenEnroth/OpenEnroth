#pragma once

#include <cassert>
#include <compare>
#include <string>
#include <string_view>

#include "Utility/String/Split.h"
#include "Utility/String/Format.h"

/**
 * `FileSystemPath` does path normalization for `FileSystem`, transforming passed paths into a normal form of
 * `"foo/bar/baz"`:
 * - Windows-style path separators are replaced with unix-style ones.
 * - Repeated path separators are replaced with a single one.
 * - Leading & trailing path separators are removed.
 * - `"."` and `".."` are collapsed.
 * - Leading `".."` are removed (since root is a parent of itself).
 * - Root path is stored as an empty string.
 *
 * Note that this means that `FileSystemPath` is different from `std::filesystem::path` in that there is no difference
 * between a file and a directory at the path level. `"foo/bar/."` is normalized into `"foo/bar"`, and can then be used
 * to open a file for reading.
 */
class FileSystemPath {
 public:
    explicit FileSystemPath(std::string_view path);

    static FileSystemPath fromNormalized(std::string path);

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

    [[nodiscard]] bool isParentOf(const FileSystemPath &child) const {
        if (isEmpty())
            return true; // Root is a parent of everything, including itself.
        return child._path.size() > _path.size() && child._path.starts_with(_path) && child._path[_path.size()] == '/';
    }

    [[nodiscard]] bool isChildOf(const FileSystemPath &parent) const {
        return parent.isParentOf(*this);
    }

    [[nodiscard]] const std::string &string() const {
        return _path;
    }

    [[nodiscard]] auto chunks() const {
        return split(_path, '/');
    }

    [[nodiscard]] FileSystemPath appended(std::string_view name) const {
        assert(name.find('\\') == std::string_view::npos && name.find('/') == std::string_view::npos && name != "." && name != "..");

        FileSystemPath result = *this;
        if (!result._path.empty())
            result._path += '/';
        result._path += name;

        return result;
    }

 private:
    [[nodiscard]] static std::string normalizePath(std::string_view path);

 private:
    std::string _path;
};

template<>
struct fmt::formatter<FileSystemPath> : fmt::formatter<std::string_view> {
    using base_type = fmt::formatter<std::string_view>;

    auto format(const FileSystemPath &path, format_context &ctx) const {
        return base_type::format(path.string(), ctx);
    }
};

template<>
struct std::hash<FileSystemPath> : std::hash<std::string> {
    using base_type = std::hash<std::string>;

    auto operator()(const FileSystemPath &path) const {
        return base_type::operator()(path.string());
    }
};
