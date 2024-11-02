#pragma once

#include <ranges>
#include <concepts>

#include "Utility/String/Split.h"

class FileSystemPathView;

class FileSystemPathSplit : public detail::SplitView {
    using base_type = detail::SplitView;
 public:
    FileSystemPathSplit() = default;

    // We disable conversions with `std::same_as<std::string_view> auto` because the only valid value to pass into the
    // functions below is an element of this `SplitView`, and that's always a `std::string_view`. Copying this value
    // into a separate `std::string` and then passing it in will blow up.

    [[nodiscard]] inline FileSystemPathView tailAt(std::same_as<std::string_view> auto chunk) const;
    [[nodiscard]] inline FileSystemPathView tailAfter(std::same_as<std::string_view> auto chunk) const;

    [[nodiscard]] inline FileSystemPathView tailAt(detail::SplitViewIterator pos) const;

 private:
    friend class FileSystemPathView;
    explicit FileSystemPathSplit(std::string_view s) : base_type(s.empty() ? base_type() : base_type(s, '/')) {}
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking FileSystemPathSplit by value.
template<>
inline constexpr bool std::ranges::enable_borrowed_range<FileSystemPathSplit> = true;
#endif
