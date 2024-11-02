#pragma once

#include <string_view>

#include "Utility/String/Split.h"

#include "FileSystemPathSplit.h"

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

    [[nodiscard]] bool isParentOf(FileSystemPathView child) const;

    [[nodiscard]] bool isChildOf(FileSystemPathView parent) const {
        return parent.isParentOf(*this);
    }

    [[nodiscard]] bool isEscaping() const {
        return _path == ".." || _path.starts_with("../");
    }

    [[nodiscard]] std::string_view string() const {
        return _path;
    }

    [[nodiscard]] FileSystemPathSplit split() const {
        return FileSystemPathSplit(string());
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

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAt(std::string_view chunk) const {
    std::string_view path = string();
    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());
    size_t offset = chunk.data() - path.data();
    return FileSystemPathView::fromNormalized(path.substr(offset));
}

[[nodiscard]] inline FileSystemPathView FileSystemPathSplit::tailAfter(std::string_view chunk) const {
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

#include "FileSystemPath.h"
