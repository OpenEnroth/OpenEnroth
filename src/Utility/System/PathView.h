#pragma once

#include <cassert>
#include <string_view>

#include "PathSplit.h"

class Path;

/**
 * Non-owning view over a `Path`. Carries the same normal-form invariant, which is why it can't be built from an
 * arbitrary string - construct a `Path` for that.
 */
class PathView {
 public:
    inline PathView(const Path &path); // NOLINT: intentionally implicit.

    /**
     * @param path                      Path string that's already in normal form, e.g. a slice of another path.
     * @return                          View over the given string.
     */
    [[nodiscard]] static PathView fromNormalized(std::string_view path) {
        PathView result;
        result._path = path;
        return result;
    }

    PathView() = default;

    friend auto operator<=>(PathView l, PathView r) = default;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    [[nodiscard]] bool isEscaping() const {
        return _path == ".." || _path.starts_with("../");
    }

    [[nodiscard]] std::string_view string() const {
        return _path;
    }

    /**
     * @return                          The root this path is relative to, empty for a relative path.
     */
    [[nodiscard]] std::string_view root() const;

    [[nodiscard]] PathSplit split() const;

 private:
    std::string_view _path;
};

template<>
struct std::hash<PathView> : std::hash<std::string_view> {
    using base_type = std::hash<std::string_view>;

    auto operator()(PathView path) const {
        return base_type::operator()(path.string()); // NOLINT: not std::string.
    }
};

[[nodiscard]] inline PathView PathSplit::tailAt(std::same_as<std::string_view> auto chunk) const {
    std::string_view path = str();
    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());
    size_t offset = chunk.data() - path.data();
    return PathView::fromNormalized(path.substr(offset));
}

[[nodiscard]] inline PathView PathSplit::tailAfter(std::same_as<std::string_view> auto chunk) const {
    std::string_view path = str(); // NOLINT: not std::string.

    if (chunk.empty())
        return PathView::fromNormalized(path);

    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());

    if (chunk.data() + chunk.size() == path.data() + path.size()) {
        return {};
    } else {
        size_t offset = chunk.data() + chunk.size() - path.data() + 1;
        return PathView::fromNormalized(path.substr(offset));
    }
}

#include "Path.h"

inline PathView::PathView(const Path &path) : _path(path.string()) {}

inline std::string_view PathView::root() const {
    return Path::rootOf(_path);
}

inline PathSplit PathView::split() const {
    return PathSplit(_path.substr(root().size()));
}
