#pragma once

#include <compare>
#include <cstddef>
#include <iterator>

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
class Segment {
 public:
    using iterator = detail::RangeIterator<T>;
    using value_type = typename iterator::value_type;
    using difference_type = typename iterator::difference_type;
    using reference = typename iterator::reference;

    constexpr Segment() {}
    constexpr Segment(T first, T last) : first_(first), last_(last) {}

    constexpr iterator begin() const {
        return iterator(first_);
    }

    constexpr iterator end() const {
        return iterator(last_) + 1;
    }

    constexpr T First() const {
        return first_;
    }

    constexpr T Last() const {
        return last_;
    }

    constexpr bool contains(T value) const {
        return first_ <= value && value <= last_;
    }

 private:
    T first_ = T();
    T last_ = T();
};
