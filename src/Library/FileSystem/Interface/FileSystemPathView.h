#pragma once

#include <string_view>

#include "Utility/String/Split.h"

#include "FileSystemPathSplit.h"
#include "FileSystemPathComponents.h"

class FileSystemPath;

class FileSystemPathView {
 public:
    inline FileSystemPathView(const FileSystemPath &path); // NOLINT: intentionally implicit.

    static inline FileSystemPathView fromNormalized(std::string_view path);

    FileSystemPathView() = default;

    FileSystemPathView(const FileSystemPathView &) = default;
    FileSystemPathView(FileSystemPathView &&) = default;
    FileSystemPathView &operator=(const FileSystemPathView &) = default;
    FileSystemPathView &operator=(FileSystemPathView &&) = default;

    friend auto operator<=>(FileSystemPathView l, FileSystemPathView r) = default;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @param path                      Path to check against.
     * @return                          Whether this path is a starting subpath of `path`. Note that this is different
     *                                  from just checking whether this path is a lexicographical prefix of `path`.
     *                                  Every path is always a starting subpath of itself.
     */
    [[nodiscard]] bool isPrefixOf(FileSystemPathView path) const {
        return
            path._path.size() >= _path.size() &&
            path._path.starts_with(_path) &&
            (path._path.size() == _path.size() || _path.empty() || path._path[_path.size()] == '/');
    }

    [[nodiscard]] bool isEscaping() const {
        return _path == ".." || _path.starts_with("../");
    }

    [[nodiscard]] std::string_view string() const {
        return _path;
    }

    [[nodiscard]] FileSystemPathSplit split() const {
        return FileSystemPathSplit(_path);
    }

    [[nodiscard]] FileSystemPathComponents components() const {
        return FileSystemPathComponents(_path);
    }

 private:
    std::string_view _path;
};

template<>
struct std::hash<FileSystemPathView> : std::hash<std::string_view> {
    using base_type = std::hash<std::string_view>;

    auto operator()(FileSystemPathView path) const {
        return base_type::operator()(path.string()); // NOLINT: not std::string.
    }
};

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAt(std::same_as<std::string_view> auto chunk) const {
    std::string_view path = string();
    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());
    size_t offset = chunk.data() - path.data();
    return FileSystemPathView::fromNormalized(path.substr(offset));
}

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAfter(std::same_as<std::string_view> auto chunk) const {
    std::string_view path = string(); // NOLINT: not std::string.

    if (chunk.empty())
        return FileSystemPathView::fromNormalized(path);

    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());

    if (chunk.data() + chunk.size() == path.data() + path.size()) {
        return {};
    } else {
        size_t offset = chunk.data() + chunk.size() - path.data() + 1;
        return FileSystemPathView::fromNormalized(path.substr(offset));
    }
}

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAt(detail::SplitViewIterator pos) const {
    return pos == detail::SplitViewSentinel() ? FileSystemPathView() : tailAt(*pos);
}

inline FileSystemPathView FileSystemPathComponents::prefix() const {
    return FileSystemPathView::fromNormalized(_path.substr(0, _prefixEnd));
}

#include "FileSystemPath.h"
