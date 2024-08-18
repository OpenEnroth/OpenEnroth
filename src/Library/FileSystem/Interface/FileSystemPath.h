#pragma once

#include <cassert>
#include <compare>
#include <string>
#include <string_view>
#include <utility>

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

    [[nodiscard]] auto chunks() const { // TODO(captainurist): fix usages now that we're not returning chunks
        if (_path.empty()) {
            return detail::SplitView();
        } else {
            return split(_path, '/');
        }
    }

    [[nodiscard]] FileSystemPath tailAt(std::string_view chunk) const {
        assert(chunk.data() >= _path.data() && chunk.data() + chunk.size() <= _path.data() + _path.size());
        size_t offset = chunk.data() - _path.data();
        return fromNormalized(_path.substr(offset));
    }

    [[nodiscard]] FileSystemPath tailAfter(std::string_view chunk) const {
        if (chunk.empty())
            return *this;

        assert(chunk.data() >= _path.data() && chunk.data() + chunk.size() <= _path.data() + _path.size());

        if (chunk.data() + chunk.size() == _path.data() + _path.size()) {
            return {};
        } else {
            size_t offset = chunk.data() + chunk.size() - _path.data() + 1;
            return fromNormalized(_path.substr(offset));
        }
    }

    void append(std::string_view chunk) {
        assert(chunk.empty() || (chunk.find('\\') == std::string_view::npos && chunk.find('/') == std::string_view::npos && chunk != "." && chunk != ".."));

        if (!_path.empty() && !chunk.empty())
            _path += '/';
        _path += chunk;
    }

    void append(const FileSystemPath &tail) {
        if (!_path.empty() && !tail.isEmpty())
            _path += '/';
        _path += tail._path;
    }

    // TODO(captainurist): name this one better, it takes a chunk, not a path that needs to be re-normalized.
    [[nodiscard]] FileSystemPath appended(std::string_view chunk) const {
        FileSystemPath result = *this;
        result.append(chunk);
        return result;
    }

    [[nodiscard]] FileSystemPath appended(const FileSystemPath &tail) const {
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
