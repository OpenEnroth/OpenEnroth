#pragma once

#include <ranges>
#include <concepts>

#include "Utility/String/Split.h"

class FileSystemPathView;

class FileSystemPathSplit : public detail::SplitView<detail::CharSplitter> {
    using base_type = detail::SplitView<detail::CharSplitter>;
 public:
    FileSystemPathSplit() = default;

    // We disable conversions with `std::same_as<std::string_view> auto` because the only valid value to pass into the
    // functions below is an element of this `SplitView`, and that's always a `std::string_view`. Copying this value
    // into a separate `std::string` and then passing it in will blow up.

    [[nodiscard]] inline FileSystemPathView tailAt(std::same_as<std::string_view> auto chunk) const;
    [[nodiscard]] inline FileSystemPathView tailAfter(std::same_as<std::string_view> auto chunk) const;

 private:
    friend class FileSystemPathView;
    explicit FileSystemPathSplit(std::string_view path) :
        base_type(path.size() <= 1 ? base_type() : base_type(path.substr(1), detail::CharSplitter('/'))), // What follows the root.
        _path(path) {}

 private:
    std::string_view _path; // The whole path, `str()` of an empty `SplitView` is not a valid range.
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking FileSystemPathSplit by value.
template<>
inline constexpr bool std::ranges::enable_borrowed_range<FileSystemPathSplit> = true;
#endif
