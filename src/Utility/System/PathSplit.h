#pragma once

#include <cstring> // For memchr.
#include <ranges>
#include <string_view>

#include "Utility/String/Split.h"

namespace detail {
/**
 * Splitter that takes a run of path separators as a single separator. The string it splits must not start or end
 * with a separator.
 */
class PathSplitter {
 public:
    using result_type = const char *;

    PathSplitter() = default;

    explicit PathSplitter(const char *end) : _end(end) {}

    result_type split(const char *begin, const char *end) const {
        const char *p = static_cast<const char *>(memchr(begin, '/', end - begin));
        return p ? p : end;
    }

    const char *begin(result_type result) const {
        return result;
    }

    const char *end(result_type result) const {
        if (result == _end)
            return result + 1;
        while (*result == '/')
            result++;
        return result;
    }

    friend bool operator==(const PathSplitter &l, const PathSplitter &r) = default;

 private:
    const char *_end = nullptr;
};
} // namespace detail

/**
 * Lazy view over the segments of a path that come after its root. Runs of separators count as one, so there are no
 * empty segments.
 */
class PathSplit : public detail::SplitView<detail::PathSplitter> {
    using base_type = detail::SplitView<detail::PathSplitter>;
 public:
    PathSplit() = default;

 private:
    friend class PathView;
    explicit PathSplit(std::string_view s) : base_type(trimmed(s)) {}

    static base_type trimmed(std::string_view s) {
        size_t first = s.find_first_not_of('/');
        if (first == std::string_view::npos)
            return base_type();
        s = s.substr(first, s.find_last_not_of('/') + 1 - first);
        return base_type(s, detail::PathSplitter(s.data() + s.size()));
    }
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking PathSplit by value.
template<>
inline constexpr bool std::ranges::enable_borrowed_range<PathSplit> = true;
#endif
