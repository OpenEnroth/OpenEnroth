#pragma once

#include <string_view>

#include "Utility/String/Split.h"

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

    [[nodiscard]] auto chunks() const {
        if (_path.empty()) {
            return detail::SplitView();
        } else {
            return split(_path, '/');
        }
    }

    [[nodiscard]] FileSystemPathView tailAt(std::string_view chunk) const {
        assert(chunk.data() >= _path.data() && chunk.data() + chunk.size() <= _path.data() + _path.size());
        size_t offset = chunk.data() - _path.data();
        return fromNormalized(_path.substr(offset));
    }

    [[nodiscard]] FileSystemPathView tailAfter(std::string_view chunk) const {
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

#include "FileSystemPath.h"
