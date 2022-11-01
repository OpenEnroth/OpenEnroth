#pragma once

#include <cstddef>

namespace detail {

template<class T>
class RangeIterator {
 public:
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T;
    using iterator_category = std::random_access_iterator_tag; // Not technically correct though.

    constexpr RangeIterator() = default;
    constexpr explicit RangeIterator(value_type pos): pos_(pos) {}

    constexpr friend auto operator<=>(RangeIterator l, RangeIterator r) = default;

    constexpr reference operator*() const {
        return pos_;
    }

    constexpr RangeIterator &operator++() {
        pos_ = static_cast<value_type>(static_cast<difference_type>(pos_) + 1);
        return *this;
    }

    constexpr RangeIterator operator++(int) {
        RangeIterator tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr RangeIterator &operator--() {
        pos_ = static_cast<value_type>(static_cast<difference_type>(pos_) - 1);
        return *this;
    }

    constexpr RangeIterator operator--(int) {
        RangeIterator tmp = *this;
        --*this;
        return tmp;
    }

    constexpr RangeIterator &operator+=(difference_type r) {
        return *this = *this + r;
    }

    constexpr RangeIterator &operator-=(difference_type r) {
        return *this = *this - r;
    }

    constexpr friend difference_type operator-(RangeIterator l, RangeIterator r) {
        return static_cast<difference_type>(l.pos_) - static_cast<difference_type>(r.pos_);
    }

    constexpr friend RangeIterator operator-(RangeIterator l, difference_type r) {
        return RangeIterator(static_cast<value_type>(static_cast<difference_type>(l.pos_) - r));
    }

    constexpr friend RangeIterator operator+(RangeIterator l, difference_type r) {
        return RangeIterator(static_cast<value_type>(static_cast<difference_type>(l.pos_) + r));
    }

    constexpr friend RangeIterator operator+(difference_type l, RangeIterator r) {
        return r + l;
    }

 private:
    value_type pos_ = T();
};

} // namespace detail


template<class T>
class Range {
 public:
    using iterator = detail::RangeIterator<T>;
    using value_type = typename iterator::value_type;
    using difference_type = typename iterator::difference_type;
    using reference = typename iterator::reference;

    Range() {}
    Range(T begin, T end) : begin_(begin), end_(end) {}

    iterator begin() const {
        return iterator(begin_);
    }

    iterator end() const {
        return iterator(end_);
    }

 private:
    T begin_ = T();
    T end_ = T();
};

template<class T>
Range<T> make_range(T first, T last) {
    return Range<T>(first, static_cast<T>(static_cast<ptrdiff_t>(last) + 1));
}
