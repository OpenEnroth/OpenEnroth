#pragma once

#include <cassert>
#include <array>
#include <type_traits>


namespace detail {

class IndexedArrayMapSentinel {};

template<class Array>
class IndexedArrayMapIterator {
 public:
    using key_type = typename Array::key_type;
    constexpr static bool is_const = std::is_const_v<Array>;
    using base_reference = std::conditional_t<is_const, typename Array::const_reference, typename Array::reference>;
    using reference = std::pair<key_type, base_reference>;

    // Note that this is a very basic implementation that's doesn't even satisfy
    // the iterator concept. The goal here is to just make the range-based for loop work.

    constexpr IndexedArrayMapIterator() {};
    constexpr IndexedArrayMapIterator(Array *array, key_type pos): array_(array), pos_(pos) {}

    constexpr friend bool operator==(IndexedArrayMapIterator l, IndexedArrayMapSentinel r) {
        return l.pos_ == static_cast<key_type>(l.array_->size());
    }

    constexpr reference operator*() const {
        return {pos_, (*array_)[pos_]};
    }

    constexpr IndexedArrayMapIterator &operator++() {
        pos_ = static_cast<key_type>(std::to_underlying(pos_) + 1);
        return *this;
    }

    constexpr IndexedArrayMapIterator operator++(int) {
        IndexedArrayMapIterator tmp = *this;
        ++*this;
        return tmp;
    }

private:
    Array *array_ = nullptr;
    key_type pos_ = key_type();
};

template<class Array>
constexpr IndexedArrayMapIterator<Array> begin(const IndexedArrayMapIterator<Array> &iterator) {
    return iterator;
}

template<class Array>
constexpr IndexedArrayMapSentinel end(const IndexedArrayMapIterator<Array> &) {
    return {};
}

} // namespace detail


/**
 * An `std::array`-like class that basically does the same thing but is indexed with an enum (type of the enum is
 * inferred from the `Size` parameter), and supports an `std::map`-like initialization, so that the user doesn't have
 * to manually double check that the order of the values in the initializer matches the order of the values of the
 * enum that's used for indexing.
 *
 * @tparam T                            Array element type.
 * @tparam Size                         Array size. Value must be of enum type.
 */
template<class T, auto Size>
class IndexedArray: public std::array<T, static_cast<size_t>(Size)> {
    static_assert(std::is_enum_v<decltype(Size)>, "Size must be an enum");
    using base_type = std::array<T, static_cast<size_t>(Size)>;

 public:
    using key_type = decltype(Size);
    using typename base_type::value_type;
    using typename base_type::reference;
    using typename base_type::const_reference;

    /**
     * Creates an uninitialized indexed array.
     */
    constexpr IndexedArray() {}

    /**
     * An `std::map`-like constructor for indexed array. The size of the provided initializer list must match
     * array size.
     *
     * Example usage:
     * @code
     * constinit IndexedArray<int, MonsterCount> MaxHP = {
     *     {Peasant, 1}
     *     {AzureDragon, 1000},
     * };
     * @endcode
     *
     * @param init                      Initializer list of key-value pairs.
     */
    constexpr IndexedArray(std::initializer_list<std::pair<key_type, value_type>> init) {
        assert(init.size() == size() || init.size() == 1);
        assert(is_unique(init));

        if (init.size() == 1) {
            // This is support for = {{}} initialization syntax, the same one as for std::array.
            // Note that = {} still leaves the array uninitialized (calling default constructor).

            for (value_type &value : *this)
                value = init.begin()->second;
        } else {
            // And this is a normal map-like constructor
            for (const auto &pair: init)
                (*this)[pair.first] = pair.second;
        }
    }

    // default operator= is OK

    /**
     * Use this function is you want to iterate over this indexed array as if it was a map, e.g.:
     * @code
     * for (auto &&pair: array.map_view()) {
     *     // key (enum value) is now in pair.first, value in pair.second.
     * }
     * @endcode
     *
     * @return                          Map-like view over the elements of this indexed array.
     */
    constexpr detail::IndexedArrayMapIterator<IndexedArray> map_view() {
        return {this, key_type()};
    }

    constexpr detail::IndexedArrayMapIterator<const IndexedArray> map_view() const {
        return {this, key_type()};
    }

    using base_type::begin;
    using base_type::end;
    using base_type::cbegin;
    using base_type::cend;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::crbegin;
    using base_type::crend;

    using base_type::size;
    using base_type::max_size;
    using base_type::data;
    using base_type::empty;
    using base_type::front;
    using base_type::back;

    using base_type::fill;
    using base_type::swap;

    constexpr reference at(key_type n) {
        return base_type::at(std::to_underlying(n));
    }

    constexpr const_reference at(key_type n) const {
        return base_type::at(std::to_underlying(n));
    }

    constexpr reference operator[](key_type n) noexcept {
        return base_type::operator[](std::to_underlying(n));
    }

    constexpr const_reference operator[](key_type n) const noexcept {
        return base_type::operator[](std::to_underlying(n));
    }

 private:
    constexpr static bool is_unique(std::initializer_list<std::pair<key_type, value_type>> init) {
        for (auto i = init.begin(); i < init.end(); i++)
            for (auto j = i + 1; j < init.end(); j++)
                if (i->first == j->first)
                    return false;
        return true;
    }
};
