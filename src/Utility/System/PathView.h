#pragma once

#include <cassert>
#include <string>
#include <string_view>

#include "PathSplit.h"

class Path;

/**
 * Non-owning view over a `Path`. It can't be built from an arbitrary string, because the bytes have to be owned by
 * something - construct a `Path` for that.
 */
class PathView {
 public:
    inline PathView(const Path &path); // NOLINT: intentionally implicit.

    /**
     * @param path                      Path string owned by something that outlives this view, e.g. a slice of
     *                                  another path.
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

    [[nodiscard]] bool isEscaping() const;

    [[nodiscard]] bool isNormalized() const;

    [[nodiscard]] std::string_view string() const {
        return _path;
    }

    /**
     * @return                          The root this path is relative to, empty for a relative path.
     */
    [[nodiscard]] std::string_view root() const;

    [[nodiscard]] bool isAbsolute() const {
        return !root().empty();
    }

    [[nodiscard]] bool isRelative() const {
        return !isAbsolute();
    }

    [[nodiscard]] PathSplit split() const;

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD. Unlike `string`, it might not
     *                                  round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

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
    if (_isEmpty)
        return {}; // No buffer to point into, so str() is not usable here.

    std::string_view path = str();
    assert(chunk.data() >= path.data() && chunk.data() + chunk.size() <= path.data() + path.size());
    size_t offset = chunk.data() - path.data();
    return PathView::fromNormalized(path.substr(offset));
}

[[nodiscard]] inline PathView PathSplit::tailAfter(std::same_as<std::string_view> auto chunk) const {
    if (_isEmpty)
        return {}; // No buffer to point into, so str() is not usable here.

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

inline Path::Path(PathView path) : _path(path.string()) {}

inline Path Path::fromNormalized(PathView path) {
    return fromNormalized(std::string(path.string()));
}

inline bool PathView::isEscaping() const {
    return Path::isEscapingImpl(_path);
}

inline std::string PathView::displayString() const {
    return Path::fromNormalized(*this).displayString();
}

inline bool PathView::isNormalized() const {
    return Path::isNormalizedImpl(_path);
}

inline std::string_view PathView::root() const {
    return Path::rootOf(_path);
}

inline Path &Path::operator/=(PathView tail) {
    return *this = *this / Path::fromNormalized(tail);
}

[[nodiscard]] inline Path operator/(PathView head, PathView tail) {
    return Path::fromNormalized(head) / Path::fromNormalized(tail);
}

inline PathSplit PathView::split() const {
    return PathSplit(_path.substr(root().size()));
}
