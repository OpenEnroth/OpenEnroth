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

    // We disable conversions with `std::same_as<std::string_view> auto` because the only valid value to pass into the
    // functions below is an element of `split()`, and that's always a `std::string_view`. Copying this value into a
    // separate `std::string` and then passing it in will blow up.

    /**
     * @param segment                   Segment of this path, as handed out by `split`.
     * @return                          The rest of this path, starting at `segment`.
     */
    [[nodiscard]] PathView tailAt(std::same_as<std::string_view> auto segment) const {
        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        return PathView(_path.substr(segment.data() - _path.data()));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view.
     * @return                          The rest of this path after `segment`, or all of it for an empty `segment`.
     */
    [[nodiscard]] PathView tailAfter(std::same_as<std::string_view> auto segment) const {
        if (segment.empty())
            return *this;

        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        if (segment.data() + segment.size() == _path.data() + _path.size())
            return {};
        return PathView(_path.substr(segment.data() + segment.size() - _path.data() + 1));
    }

    friend auto operator<=>(PathView l, PathView r) = default;

 private:
    explicit PathView(std::string_view path) : _path(path) {}

 private:
    std::string_view _path;
};
