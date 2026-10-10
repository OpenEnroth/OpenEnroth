#pragma once

#include <compare>
#include <concepts>
#include <string_view>

#include "Utility/System/Path.h"

class NormalPath;

/**
 * View over a path that a file system can act on, in the normal form the file system layer works in. It can only come
 * from a `NormalPath` that the caller checked with `isAccessible`, or be a tail of another `NormalPathView`, so a
 * function that takes one can rely on both without checking.
 */
class NormalPathView {
 public:
    NormalPathView() = default;

    operator PathView() const { // NOLINT: intentionally implicit.
        return _path;
    }

    [[nodiscard]] std::string_view str() const {
        return _path.str();
    }

    [[nodiscard]] bool isEmpty() const {
        return _path.isEmpty();
    }

    [[nodiscard]] PathSplit split() const {
        return _path.split();
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`.
     * @return                          The rest of this path, starting at `segment`.
     */
    [[nodiscard]] NormalPathView tailAt(std::same_as<std::string_view> auto segment) const {
        return NormalPathView(_path.split().tailAt(segment));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view.
     * @return                          The rest of this path after `segment`, or all of it for an empty `segment`.
     */
    [[nodiscard]] NormalPathView tailAfter(std::same_as<std::string_view> auto segment) const {
        return NormalPathView(_path.split().tailAfter(segment));
    }

 private:
    friend class NormalPath;

    explicit NormalPathView(PathView path) : _path(path) {}

 private:
    PathView _path;
};

/**
 * A path inside a `FileSystem`, in the normal form the file system layer works in. A backslash is a separator on every
 * platform, and `.`, `..` and repeated separators are folded. A path that has a root or escapes stays representable,
 * so that `displayPath` can show it, and `isAccessible` says whether a file system can act on it. Only an accessible
 * one may be passed on as a `NormalPathView`.
 */
class NormalPath {
 public:
    NormalPath() = default;
    explicit NormalPath(std::string_view path);
    explicit NormalPath(PathView path) : NormalPath(path.str()) {}

    operator NormalPathView() const { // NOLINT: intentionally implicit.
        return NormalPathView(_path);
    }

    [[nodiscard]] const Path &path() const {
        return _path;
    }

    /**
     * @return                          Whether a file system can reach this path. It can't if the path has a root,
     *                                  as `"/a"` does, or escapes the file system's root. On Windows it can't if a
     *                                  segment starts with a drive, as in `"a/c:b"`, because a tail that starts at
     *                                  that segment reads as a root.
     */
    [[nodiscard]] bool isAccessible() const;

    /**
     * @param tail                      Path to append. Joining two accessible paths keeps the result normal.
     */
    NormalPath &operator/=(NormalPathView tail);

    /**
     * @return                          `head` and `tail` joined, built in one allocation. Joining two accessible paths
     *                                  keeps the result normal.
     */
    [[nodiscard]] friend NormalPath operator/(NormalPathView head, NormalPathView tail);

 private:
    Path _path;
};

