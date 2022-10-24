#pragma once

#include <cassert>
#include <array>
#include <type_traits>
#include <utility>

#include "Utility/Workaround/ToUnderlying.h"


namespace detail {

using std::to_underlying;

template<class T>
T to_underlying(T value) requires std::is_integral_v<T> {
    return value;
}

class IndexedArrayKeysSentinel {};

template<auto Size, auto Zero>
class IndexedArrayKeysIterator {
 public:
    using value_type = decltype(Size);
    using reference = value_type;

    // Note that this is a very basic implementation that doesn't even satisfy
    // the iterator concept. The goal here is to just make the range-based for loop work.

    constexpr IndexedArrayKeysIterator() {}
    constexpr IndexedArrayKeysIterator(value_type pos): pos_(pos) {}

    constexpr friend bool operator==(IndexedArrayKeysIterator l, IndexedArrayKeysSentinel r) {
        return l.pos_ == Size;
    }

    constexpr reference operator*() const {
        return pos_;
    }

    constexpr IndexedArrayKeysIterator &operator++() {
        pos_ = static_cast<value_type>(detail::to_underlying(pos_) + 1);
        return *this;
    }

    constexpr IndexedArrayKeysIterator operator++(int) {
        IndexedArrayKeysIterator tmp = *this;
        ++*this;
        return tmp;
    }

 private:
    value_type pos_ = Zero;
};

template<auto Size, auto Zero>
struct IndexedArrayKeysRange {
    IndexedArrayKeysIterator<Size, Zero> begin;
};

template<auto Size, auto Zero>
constexpr IndexedArrayKeysIterator<Size, Zero> begin(const IndexedArrayKeysRange<Size, Zero> &range) {
    return range.begin;
}

template<auto Size, auto Zero>
constexpr IndexedArrayKeysSentinel end(const IndexedArrayKeysRange<Size, Zero> &) {
    return {};
}

} // namespace detail


/**
 * An `std::array`-like class that basically does the same thing but can be indexed with an enum (type is inferred
 * from the `Size` parameter), and supports an `std::map`-like initialization, so that the user doesn't have
 * to manually double check that the order of the values in the initializer matches the order of the values of the
 * enum that's used for indexing.
 *
 * Some code examples:
 * @code
 * enum class TriBool {
 *    True,
 *    False,
 *    DontKnow,
 *    TriBool_Size
 * };
 * using enum TriBool;
 *
 * // IndexedArray supports the same initialization syntax as `std::array`.
 * IndexedArray<std::string, TriBool_Size> userMessageMap = {{}};
 *
 * // And it can also be constructed like an `std::map`.
 * IndexedArray<std::string, TriBool_Size> defaultMessageMap = {
 *     {True, "true"},
 *     {False, "false"},
 *     {DontKnow, "unknown"}
 * };
 *
 * extern TriBool f(int);
 *
 * // Access IndexedArray elements using enum values as indices.
 * std::cout << "f(10)=" << defaultMessageMap[f(10)] << std::endl;
 *
 * // Iterate through the IndexedArray like it's an array...
 * for (auto &value : userMessageMap)
 *     value.clear();
 *
 * // ...or get a view of its indices and use it as you would use a traditional array.
 * for (TriBool i : defaultMessageMap.indices())
 *     userMessageMap[i] = defaultMessageMap[i];
 * @endcode
 *
 * @tparam T                            Array element type.
 * @tparam Size                         Array size. Value must be of enum type or integral type.
 * @tparam Zero                         Index of the first element. Basically, valid indices are in range [`Zero`, `Size`).
 *                                      If this parameter is provided, then the size of the indexed array might not
 *                                      equal `Size`.
 */
template<class T,
         auto Size,
         auto Zero = static_cast<decltype(Size)>(0),
         size_t StorageSize = static_cast<size_t>(Size) - static_cast<size_t>(Zero)>
class IndexedArray: public std::array<T, StorageSize> {
    static_assert(std::is_enum_v<decltype(Size)> || std::is_integral_v<decltype(Size)>, "Size must be an enum or an integral type");
    static_assert(std::is_same_v<decltype(Size), decltype(Zero)>, "Size and Zero must be of the same type");
    using base_type = std::array<T, StorageSize>;

 public:
    using key_type = decltype(Size);
    using typename base_type::value_type;
    using typename base_type::reference;
    using typename base_type::const_reference;

    /**
     * Creates an uninitialized indexed array.
     *
     * This is the constructor that gets called when you use aggregate initialization, so the behavior is the same as
     * with `std::array`:
     * @code
     * IndexedArray<int, TriBool_Size> uninitializedIntegers = {};
     * @endcode
     *
     * If you want to default-initialize array elements, see the other constructor.
     */
    constexpr IndexedArray() {}

    /**
     * An `std::map`-like constructor for indexed array. The size of the provided initializer list must match
     * array size. Alternatively, this constructor can be used to default-initialize the indexed array using the same
     * syntax as is used for `std::array`.
     *
     * Example usage:
     * @code
     * enum class Monster {
     *     Peasant,
     *     AzureDragon,
     *     MonsterCount
     * };
     * using enum Monster;
     *
     * constinit IndexedArray<int, MonsterCount> maxHP = {
     *     {Peasant, 1}
     *     {AzureDragon, 1000}
     * };
     *
     * IndexedArray<int, MonsterCount> killCount = {{}}; // ints inside the array are default-initialized to zero.
     * @endcode
     *
     * @param init                      Initializer list of key-value pairs.
     */
    constexpr IndexedArray(std::initializer_list<std::pair<key_type, value_type>> init) {
        assert(init.size() == size() || init.size() == 1);
        assert(is_unique(init));

        if (init.size() == 1) {
            // This is support for = {{}} initialization syntax, the same one as for std::array.
            for (value_type &value : *this)
                value = init.begin()->second;
        } else {
            // And this is a normal map-like constructor.
            for (const auto &pair : init)
                (*this)[pair.first] = pair.second;
        }
    }

    // default operator= is OK

    /**
     * Use this function is you want to iterate over this indexed array's like it's a normal array, e.g.:
     * @code
     * for (SomeEnum i : array.keys()) {
     *     // use i and array[i]
     * }
     * @endcode
     *
     * @return                          View over the valid indices for the elements of this indexed array.
     */
    constexpr detail::IndexedArrayKeysRange<Size, Zero> indices() const {
        return {};
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
        return base_type::at(detail::to_underlying(n) - detail::to_underlying(Zero));
    }

    constexpr const_reference at(key_type n) const {
        return base_type::at(detail::to_underlying(n) - detail::to_underlying(Zero));
    }

    constexpr reference operator[](key_type n) noexcept {
        return base_type::operator[](detail::to_underlying(n) - detail::to_underlying(Zero));
    }

    constexpr const_reference operator[](key_type n) const noexcept {
        return base_type::operator[](detail::to_underlying(n) - detail::to_underlying(Zero));
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
