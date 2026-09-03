#pragma once

#include <ranges>
#include <concepts>

#include "Utility/String/Split.h"

class Path;
class PathView;

/**
 * Lazy view over the name segments of a `Path`. The root is not a segment - anchor with `Path::root` if you need it.
 */
class PathSplit : public detail::SplitView<detail::CharSplitter> {
    using base_type = detail::SplitView<detail::CharSplitter>;

 public:
    PathSplit() = default;

    // We disable conversions with `std::same_as<std::string_view> auto` because the only valid value to pass into the
    // functions below is an element of this `SplitView`, and that's always a `std::string_view`. Copying this value
    // into a separate `std::string` and then passing it in will blow up.

    [[nodiscard]] inline PathView tailAt(std::same_as<std::string_view> auto chunk) const;
    [[nodiscard]] inline PathView tailAfter(std::same_as<std::string_view> auto chunk) const;

 private:
    friend class Path;
    friend class PathView;
    explicit PathSplit(std::string_view s)
        : base_type(s.empty() ? base_type() : base_type(s, detail::CharSplitter('/'))), _isEmpty(s.empty()) {}

 private:
    // A default-constructed SplitView points at no buffer at all - it marks itself past-the-end by setting _begin
    // above _end, so str() comes back as a negative-length view. Remembering how we were built is the only way to
    // tell that apart from a real one.
    bool _isEmpty = true;
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking PathSplit by value.
template<>
inline constexpr bool std::ranges::enable_borrowed_range<PathSplit> = true;
#endif
