#pragma once

#include <cstddef>

namespace detail {

template<class T>
class RangeIterator {
 public:
    using value_type = T;
    using reference = value_type;

    // Note that this is a very basic implementation that doesn't even satisfy
    // the iterator concept. The goal here is to just make the range-based for loop work.

    constexpr RangeIterator() {}
    constexpr RangeIterator(value_type pos): pos_(pos) {}

    constexpr friend bool operator==(RangeIterator l, RangeIterator r) {
        return l.pos_ == r.pos_;
    }

    constexpr reference operator*() const {
        return pos_;
    }

    constexpr RangeIterator &operator++() {
        pos_ = static_cast<value_type>(static_cast<ptrdiff_t>(pos_) + 1);
        return *this;
    }

    constexpr RangeIterator operator++(int) {
        RangeIterator tmp = *this;
        ++*this;
        return tmp;
    }

 private:
    value_type pos_ = T();
};

} // namespace detail


template<class T>
class Range {
 public:
    Range() {}
    Range(T begin, T end) : begin_(begin), end_(end) {}

    friend detail::RangeIterator<T> begin(const Range &range) {
        return { range.begin_ };
    }

    friend detail::RangeIterator<T> end(const Range &range) {
        return { range.end_ };
    }

 private:
    T begin_ = T();
    T end_ = T();
};

template<class T>
Range<T> make_range(T first, T last) {
    return Range<T>(first, static_cast<T>(static_cast<ptrdiff_t>(last) + 1));
}
