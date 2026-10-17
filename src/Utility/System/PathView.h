#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <string_view>

#include "Utility/String/Split.h"

class Path;

/**
 * Non-owning view over a `Path`, or over a slice of one.
 */
class PathView {
 public:
    PathView() = default;
    inline PathView(const Path &path); // NOLINT: intentionally implicit.

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
     * @return                          Lexical parent, see `Path::parent`.
     */
    [[nodiscard]] PathView parent() const;

    /**
     * @return                          The segments after the root. See `Path::split`.
     */
    [[nodiscard]] auto split() const {
        return ::split(_path.substr(root().size())).by('/').skip("");
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`. It has to point into this
     *                                  path.
     * @return                          The rest of this path, starting at `segment`. The root is not part of it.
     */
    [[nodiscard]] PathView tailAt(std::same_as<std::string_view> auto segment) const {
        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        return PathView(_path.substr(segment.data() - _path.data()));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view. It has to
     *                                  point into this path.
     * @return                          The rest of this path after `segment` and the separators that follow it, or
     *                                  all of it, root included, for an empty `segment`.
     */
    [[nodiscard]] PathView tailAfter(std::same_as<std::string_view> auto segment) const {
        if (segment.empty())
            return *this;

        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        size_t offset = _path.find_first_not_of('/', segment.data() + segment.size() - _path.data());
        if (offset == std::string_view::npos)
            return {};
        return PathView(_path.substr(offset));
    }

    friend auto operator<=>(PathView l, PathView r) = default;

 private:
    explicit PathView(std::string_view path) : _path(path) {}

 private:
    std::string_view _path;
};
