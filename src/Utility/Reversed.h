#pragma once

#include <iterator>
#include <utility>

namespace detail {
template <class Range>
class ReversedRange {
 public:
    constexpr explicit ReversedRange(Range &&) = delete; // Constructing from rvalue will lead to dangling iterators. Need P2718R0 for this to work.
    constexpr explicit ReversedRange(Range &range) : _range(range) {}

    constexpr auto begin() const {
        using std::crbegin;
        return crbegin(_range);
    }

    constexpr auto end() const {
        using std::crend;
        return crend(_range);
    }

    constexpr auto begin() {
        using std::rbegin;
        return rbegin(_range);
    }

    constexpr auto end() {
        using std::rend;
        return rend(_range);
    }

 private:
    Range &_range;
};
} // namespace detail

/**
 * Provides a reversed view into a range. Can be handy in a range-based for loop.
 *
 * @param range                         Range to get a reversed view into. Must not be an rvalue.
 * @return                              Reversed view.
 */
template <class Range>
constexpr auto reversed(Range &&range) {
    return detail::ReversedRange(std::forward<Range>(range));
}
