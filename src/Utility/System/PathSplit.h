#pragma once

#include <ranges>
#include <string_view>

#include "Utility/String/Split.h"

/**
 * Lazy view over the segments of a path that come after its root.
 */
class PathSplit : public detail::SplitView<detail::CharSplitter> {
    using base_type = detail::SplitView<detail::CharSplitter>;
 public:
    PathSplit() = default;

 private:
    friend class Path;
    friend class PathView;
    explicit PathSplit(std::string_view s) : base_type(s.empty() ? base_type() : base_type(s, detail::CharSplitter('/'))) {}
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking PathSplit by value.
template<>
inline constexpr bool std::ranges::enable_borrowed_range<PathSplit> = true;
#endif
