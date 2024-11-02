#pragma once

#include <ranges>

#include "Utility/String/Split.h"

class FileSystemPathView;

class FileSystemPathSplit : public detail::SplitView {
    using base_type = detail::SplitView;
 public:
    FileSystemPathSplit() = default;

    [[nodiscard]] inline FileSystemPathView tailAt(std::string_view chunk) const;
    [[nodiscard]] inline FileSystemPathView tailAfter(std::string_view chunk) const;

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
