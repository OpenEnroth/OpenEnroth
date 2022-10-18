#pragma once

#include <cassert>
#include <array>
#include <type_traits>
#include <utility>

#include "Workaround.h"


namespace detail {

class IndexedArrayKeysSentinel {};

template<auto Size>
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
        pos_ = static_cast<value_type>(std::to_underlying(pos_) + 1);
        return *this;
    }

    constexpr IndexedArrayKeysIterator operator++(int) {
        IndexedArrayKeysIterator tmp = *this;
        ++*this;
        return tmp;
    }

 private:
    value_type pos_ = value_type();
};

template<auto Size>
struct IndexedArrayKeysRange {
    IndexedArrayKeysIterator<Size> begin;
};

template<auto Size>
constexpr IndexedArrayKeysIterator<Size> begin(const IndexedArrayKeysRange<Size> &range) {
    return range.begin;
}

template<auto Size>
constexpr IndexedArrayKeysSentinel end(const IndexedArrayKeysRange<Size> &) {
    return {};
}

} // namespace detail


/**
 * An `std::array`-like class that basically does the same thing but is indexed with an enum (type of the enum is
 * inferred from the `Size` parameter), and supports an `std::map`-like initialization, so that the user doesn't have
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
    constexpr detail::IndexedArrayKeysRange<Size> indices() const {
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
