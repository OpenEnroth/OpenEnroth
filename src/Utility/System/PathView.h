#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <string_view>

#include "PathSplit.h"

class Path;

/**
 * Non-owning view over a `Path`, or over a slice of one.
 */
class PathView {
 public:
    PathView() = default;
    inline PathView(const Path &path); // NOLINT: intentionally implicit.

    /**
     * @param path                      Path string in normal form, e.g. a slice of a normalized `Path`. It has to
     *                                  outlive the view.
     * @return                          View over the given string.
     */
    [[nodiscard]] static PathView fromNormalized(std::string_view path) {
        PathView result;
        result._path = path;
        assert(result.isNormalized());
        return result;
    }

    [[nodiscard]] std::string_view str() const {
        return _path;
    }

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @return                          The root name and root directory this path starts with, empty if there are
     *                                  none. See `Path::root`.
     */
    [[nodiscard]] std::string_view root() const;

    /**
     * @return                          Whether this path points above its starting point. See `Path::isEscaping`.
     */
    [[nodiscard]] bool isEscaping() const;

    /**
     * @return                          Whether this path is in normal form. See `Path::normalized`.
     */
    [[nodiscard]] bool isNormalized() const;

    /**
     * @return                          The segments after the root. See `Path::split`.
     */
    [[nodiscard]] PathSplit split() const;

    friend auto operator<=>(PathView l, PathView r) = default;

 private:
    std::string_view _path;
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
