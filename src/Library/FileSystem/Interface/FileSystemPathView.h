#pragma once

#include <cassert>
#include <string_view>

#include "Utility/String/Split.h"

#include "FileSystemPathSplit.h"

class FileSystemPath;

class FileSystemPathView {
 public:
    inline FileSystemPathView(const FileSystemPath &path); // NOLINT: intentionally implicit.

    /**
     * Constructs a view of the root path.
     */
    FileSystemPathView() = default;

    FileSystemPathView(const FileSystemPathView &) = default;
    FileSystemPathView(FileSystemPathView &&) = default;
    FileSystemPathView &operator=(const FileSystemPathView &) = default;
    FileSystemPathView &operator=(FileSystemPathView &&) = default;

    friend auto operator<=>(FileSystemPathView l, FileSystemPathView r) = default;

    [[nodiscard]] bool isRoot() const {
        return _path == "/";
    }

    [[nodiscard]] std::string_view string() const {
        return _path;
    }

    [[nodiscard]] FileSystemPathSplit split() const {
        return FileSystemPathSplit(_path);
    }

 private:
    friend class FileSystemPathSplit;

    explicit FileSystemPathView(std::string_view path) : _path(path) {
        assert(_path.starts_with('/'));
    }

 private:
    std::string_view _path = "/";
};

template<>
struct std::hash<FileSystemPathView> : std::hash<std::string_view> {
    using base_type = std::hash<std::string_view>;

    auto operator()(FileSystemPathView path) const {
        return base_type::operator()(path.string()); // NOLINT: not std::string.
    }
};

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAt(std::same_as<std::string_view> auto chunk) const {
    assert(chunk.data() > _path.data() && chunk.data() + chunk.size() <= _path.data() + _path.size());
    size_t offset = chunk.data() - 1 - _path.data(); // The separator before the chunk, normal form has exactly one.
    return FileSystemPathView(_path.substr(offset));
}

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAfter(std::same_as<std::string_view> auto chunk) const {
    if (chunk.empty())
        return _path.empty() ? FileSystemPathView() : FileSystemPathView(_path);

    assert(chunk.data() > _path.data() && chunk.data() + chunk.size() <= _path.data() + _path.size());
    size_t offset = chunk.data() + chunk.size() - _path.data();
    if (offset == _path.size())
        return {};
    return FileSystemPathView(_path.substr(offset)); // A suffix of a normal path is normal.
}

#include "FileSystemPath.h"
