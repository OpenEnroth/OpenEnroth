#pragma once

#include <cstddef>
#include <iterator>
#include <ranges>
#include <string_view>

#include "Utility/View.h"

class PathView;

class PathSplitIterator {
 public:
    using iterator_concept = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::string_view;

    PathSplitIterator() = default;

    [[nodiscard]] std::string_view operator*() const {
        return _segment;
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
        return l._segment.data() == r._segment.data() && l._segment.size() == r._segment.size();
    }

    friend bool operator==(const PathSplitIterator &l, std::default_sentinel_t) {
        return l._segment.empty();
    }

 private:
    friend class PathSplit;

    explicit PathSplitIterator(std::string_view rest) : _rest(rest) {
        advance();
    }

    void advance() {
        size_t start = _rest.find_first_not_of('/');
        if (start == std::string_view::npos) {
            _segment = {};
            _rest = {};
            return;
        }
        size_t end = _rest.find('/', start);
        _segment = _rest.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
        _rest = _rest.substr(start + _segment.size());
    }

    std::string_view _segment;
    std::string_view _rest;
};

/**
 * Segments of a path after its root, with runs of separators taken as one, so there are no empty segments. The
 * segments point into the path, and so does the range itself.
 */
class PathSplit : public ViewInterface<PathSplit> {
 public:
    PathSplit() = default;

    [[nodiscard]] PathSplitIterator begin() const {
        return PathSplitIterator(_path);
    }

    [[nodiscard]] std::default_sentinel_t end() const {
        return {};
    }

 private:
    friend class PathView;

    explicit PathSplit(std::string_view path) : _path(path) {}

    std::string_view _path;
};

#ifndef __DOXYGEN__ // Doxygen chokes here...
template<>
inline constexpr bool std::ranges::enable_borrowed_range<PathSplit> = true;
#endif
