#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <string_view>

#include "Utility/View.h"

namespace detail {

/**
 * @param c                             Character to check.
 * @return                              Whether `c` separates path components. Both slashes do on Windows, only the
 *                                      forward one on POSIX.
 */
[[nodiscard]] constexpr bool isPathSeparator(char c) {
#ifdef _WINDOWS
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

/**
 * @param path                          Path to search.
 * @param from                          Offset to start at.
 * @return                              Offset of the first separator at or after `from`, or `npos` if there is none.
 */
[[nodiscard]] inline size_t findPathSeparator(std::string_view path, size_t from) {
#ifdef _WINDOWS
    return std::min(path.find('/', from), path.find('\\', from)); // Two memchr scans, find_first_of checks every char.
#else
    return path.find('/', from);
#endif
}

/**
 * @param path                          Path to search.
 * @return                              Offset of the last separator, or `npos` if there is none.
 */
[[nodiscard]] inline size_t findLastPathSeparator(std::string_view path) {
#ifdef _WINDOWS
    size_t slash = path.rfind('/');
    size_t backslash = path.rfind('\\');
    if (slash == std::string_view::npos || backslash == std::string_view::npos)
        return std::min(slash, backslash); // The one that was found, if any.
    return std::max(slash, backslash);
#else
    return path.rfind('/');
#endif
}

/**
 * @param path                          Path to scan.
 * @param from                          Offset to start at.
 * @return                              Offset of the first character at or after `from` that isn't a separator, or the
 *                                      size of `path` if there is none.
 */
[[nodiscard]] inline size_t skipPathSeparators(std::string_view path, size_t from) {
    while (from < path.size() && isPathSeparator(path[from]))
        from++;
    return from;
}

} // namespace detail

class PathSplitIterator {
 public:
    using iterator_concept = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::string_view;

    PathSplitIterator() = default;

    [[nodiscard]] std::string_view operator*() const {
        return _path.substr(_start, _end - _start);
    }

    PathSplitIterator &operator++() {
        advance();
        return *this;
    }

    PathSplitIterator operator++(int) {
        PathSplitIterator result = *this;
        advance();
        return result;
    }

    friend bool operator==(const PathSplitIterator &l, const PathSplitIterator &r) {
        return l._path.data() + l._start == r._path.data() + r._start;
    }

    friend bool operator==(const PathSplitIterator &l, std::default_sentinel_t) {
        return l._start == l._path.size();
    }

 private:
    friend class PathSplit;

    explicit PathSplitIterator(std::string_view path) : _path(path) {
#ifdef _WINDOWS
        _nextSlash = _path.find('/');
        _nextBackslash = _path.find('\\');
#endif
        advance();
    }

    void advance() {
        _start = detail::skipPathSeparators(_path, _end);
        _end = _start == _path.size() ? _start : std::min(nextSeparator(_start), _path.size());
    }

    size_t nextSeparator(size_t from) {
#ifdef _WINDOWS
        // Each slash has its own cursor, searched with memchr only once the scan has passed it, so that both cross the
        // path once in all.
        if (_nextSlash < from)
            _nextSlash = _path.find('/', from);
        if (_nextBackslash < from)
            _nextBackslash = _path.find('\\', from);
        return std::min(_nextSlash, _nextBackslash);
#else
        return _path.find('/', from);
#endif
    }

    std::string_view _path;
    size_t _start = 0;
    size_t _end = 0;
#ifdef _WINDOWS
    size_t _nextSlash = std::string_view::npos;
    size_t _nextBackslash = std::string_view::npos;
#endif
};

/**
 * Segments of a path, with runs of separators taken as one, so there are no empty segments. The segments point into
 * the split string, and so does the range itself.
 */
class PathSplit : public ViewInterface<PathSplit> {
 public:
    PathSplit() = default;

    /**
     * @param path                      String to split, with its root already taken off.
     */
    explicit PathSplit(std::string_view path) : _path(path) {}

    [[nodiscard]] PathSplitIterator begin() const {
        return PathSplitIterator(_path);
    }

    [[nodiscard]] std::default_sentinel_t end() const {
        return {};
    }

 private:
    std::string_view _path;
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
template<>
inline constexpr bool std::ranges::enable_borrowed_range<PathSplit> = true;
#endif
