#pragma once

#include <concepts>
#include <string_view>

#include "Utility/Attributes.h"
#include "Utility/System/Path.h"

class NormalPath;

/**
 * View over a path in the normal form the file system layer works in. It comes from a `NormalPath` or from a tail of
 * another `NormalPathView`, so a function that takes one can rely on it being normal. It can be inaccessible, see
 * `isAccessible`, and the public `FileSystem` methods check that. An empty view names the root.
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

    [[nodiscard]] bool isRoot() const {
        return _path.isEmpty();
    }

    /**
     * @return                          Whether a file system can reach this path. It can't if the path has a root, as
     *                                  `"/a"` does. On Windows it also can't if a segment starts with a drive, as in
     *                                  `"a/c:b"`, because a tail starting there reads as a root.
     */
    [[nodiscard]] bool isAccessible() const;

    [[nodiscard]] PathSplit split() const {
        return _path.split();
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`.
     * @return                          The rest of this path, starting at `segment`.
     */
    [[nodiscard]] NormalPathView tailAt(std::same_as<std::string_view> auto segment) const {
        return NormalPathView(_path.tailAt(segment));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view.
     * @return                          The rest of this path after `segment`, empty if there's nothing left, or all
     *                                  of it for an empty `segment`.
     */
    [[nodiscard]] NormalPathView tailAfter(std::same_as<std::string_view> auto segment) const {
        return NormalPathView(_path.tailAfter(segment));
    }

 private:
    friend class NormalPath;

    explicit NormalPathView(PathView path) : _path(path) {}

 private:
    PathView _path;
};

/**
 * @param head                          Path to join onto, must be accessible.
 * @param tail                          Path to append, must be accessible.
 * @return                              `head` and `tail` joined, in normal form.
 */
[[nodiscard]] NormalPath operator/(NormalPathView head, NormalPathView tail);

/**
 * A path inside a `FileSystem`, in the normal form the file system layer works in. A backslash is a separator on every
 * platform, and `.`, `..` and repeated separators are folded. The root is `""`, so `"."` and `"a/.."` normalize to
 * it, and `..` stops there, so `"../a"` is `"a"`. A path that `isAccessible` refuses stays representable, so that
 * `displayPath` can show it. Build one to join paths, or to use a path repeatedly without normalizing it again.
 */
class NormalPath {
 public:
    NormalPath() = default;
    explicit NormalPath(PathView path);

    operator NormalPathView() const MM_LIFETIMEBOUND { // NOLINT: intentionally implicit.
        return NormalPathView(_path);
    }

    [[nodiscard]] const Path &path() const MM_LIFETIMEBOUND {
        return _path;
    }

    [[nodiscard]] bool isAccessible() const {
        return NormalPathView(_path).isAccessible();
    }

    /**
     * Both this path and `tail` must be accessible.
     *
     * @param tail                      Path to append.
     * @return                          `*this`, still in normal form.
     */
    NormalPath &operator/=(NormalPathView tail);

    friend NormalPath operator/(NormalPathView head, NormalPathView tail);

 private:
    Path _path;
};

