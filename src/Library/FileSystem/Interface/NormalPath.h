#pragma once

#include <concepts>
#include <string_view>

#include "Utility/Attributes.h"
#include "Utility/System/Path.h"

class NormalPath;

/**
 * View over a path in the normal form the file system layer works in. It comes from a `NormalPath` or from a tail of
 * another `NormalPathView`, so a function that takes one can rely on it being normal. It can be inaccessible, see
 * `isAccessible`, and the public `FileSystem` methods check that. A default-constructed view names the root.
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
        return _path.str() == "/";
    }

    /**
     * @return                          Whether a file system can reach this path, which it can if the path starts
     *                                  with `/`.
     */
    [[nodiscard]] bool isAccessible() const {
        return _path.str().starts_with('/');
    }

    [[nodiscard]] PathSplit split() const {
        return _path.split();
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`.
     * @return                          The rest of this path as a path of its own, starting at the separator before
     *                                  `segment`.
     */
    [[nodiscard]] NormalPathView tailAt(std::same_as<std::string_view> auto segment) const {
        return NormalPathView(PathView(str().substr(segment.data() - 1 - str().data())));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view.
     * @return                          The rest of this path after `segment` as a path of its own, the root if
     *                                  there's nothing left, or all of it for an empty `segment`.
     */
    [[nodiscard]] NormalPathView tailAfter(std::same_as<std::string_view> auto segment) const {
        if (segment.empty())
            return *this;
        std::string_view tail = str().substr(segment.data() + segment.size() - str().data());
        return tail.empty() ? NormalPathView() : NormalPathView(PathView(tail));
    }

 private:
    friend class NormalPath;

    explicit NormalPathView(PathView path) : _path(path) {}

 private:
    PathView _path = "/";
};

/**
 * @param head                          Path to join onto, must be accessible.
 * @param tail                          Path to append, must be accessible.
 * @return                              `head` and `tail` joined, in normal form.
 */
[[nodiscard]] NormalPath operator/(NormalPathView head, NormalPathView tail);

/**
 * A path inside a `FileSystem`, in the normal form the file system layer works in. A backslash is a separator on every
 * platform, and `.`, `..` and repeated separators are folded. The root is `/`, and `..` stops there. A path that
 * doesn't start with `/` stays representable, so that `displayPath` can show it, but `isAccessible` refuses it. A
 * default-constructed path is the root. Build one to join paths, or to use a path repeatedly without normalizing it
 * again.
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
    Path _path = "/";
};

