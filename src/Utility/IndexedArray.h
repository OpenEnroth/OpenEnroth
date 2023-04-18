#pragma once

#include <cassert>
#include <cstddef>
#include <array>
#include <type_traits>
#include <utility>

#include "Segment.h"

namespace detail {

enum class LastIndex {
    INVALID_LAST_INDEX
};
using enum LastIndex;

} // namespace detail


/**
 * An `std::array`-like class that supports some additional features:
 * - Can be indexed with an enum (type is inferred from the `Size` parameter).
 * - Supports an `std::map`-like initialization, so that the user doesn't have to manually double check that the order
 *   of the values in the initializer matches the order of the values of the enum that's used for indexing.
 * - Supports non-zero-based indexing (e.g. can be used to construct a Pascal-like array).
 *
 * The template itself is overloaded and can be used in several different ways:
 * - `IndexedArray<int, 10>` works the same as `std::array<int, 10>`.
 * - `IndexedArray<int, 1, 10>` creates a Pascal-like array with a first index of 1, and last index of 10
 *   (so the size is still 10 as in the previous example).
 * - `IndexedArray<int, SomeEnum_Count>` creates an indexed array that's indexed with an enum. The main use case for
 *   this is using an `enum class` for indexing to make it fully type-safe.
 * - `IndexedArray<int, FirstWeaponItem, LastWeaponItem>` creates a non-zero-based enum-indexed array.
 *
 * Some code examples:
 * @code
 * enum class TriBool {
 *    True,
 *    False,
 *    DontKnow
 * };
 * using enum TriBool;
 *
 * // IndexedArray supports the same initialization syntax as `std::array`.
 * IndexedArray<std::string, True, DontKnow> userMessageMap = {{}};
 *
 * // And it can also be constructed like an `std::map`.
 * IndexedArray<std::string, True, DontKnow> defaultMessageMap = {
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
 * @tparam SizeOrFirstIndex             Either array size or index of the 1st element. Value must be of enum or
 *                                      integral type.
 * @tparam LastIndex                    Index of the last element. If this parameter is provided, then the size of the
 *                                      indexed array is `LastIndex - SizeOrFirstIndex + 1`.
 */
template<
    class T,
    auto SizeOrFirstIndex,
    auto LastIndex = detail::INVALID_LAST_INDEX,
    bool IsZeroBased = std::is_same_v<decltype(LastIndex), detail::LastIndex>,
    ptrdiff_t Size =
        IsZeroBased ?
        static_cast<ptrdiff_t>(SizeOrFirstIndex) :
        static_cast<ptrdiff_t>(LastIndex) - static_cast<ptrdiff_t>(SizeOrFirstIndex) + 1>
class IndexedArray: public std::array<T, Size> {
    using base_type = std::array<T, Size>;
    using index_type = decltype(SizeOrFirstIndex);

    static_assert(Size >= 0, "IndexedArray size must be non-negative");
    static_assert(std::is_enum_v<index_type> || std::is_integral_v<index_type>, "SizeOrFirstIndex must be an enum or an integral type");
    static_assert(IsZeroBased || std::is_same_v<index_type, decltype(LastIndex)>, "SizeOrFirstIndex and LastIndex must be of the same type");

    static constexpr auto ActualFirstIndex =
        IsZeroBased ? static_cast<index_type>(0) : SizeOrFirstIndex;
    static constexpr auto ActualLastIndex =
        IsZeroBased ? static_cast<index_type>(static_cast<ptrdiff_t>(SizeOrFirstIndex) - 1) : static_cast<index_type>(static_cast<ptrdiff_t>(LastIndex));

 public:
    static constexpr size_t SIZE = Size;
    using key_type = index_type;
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
     * Use this function is you want to iterate over this indexed array like it's a normal array, e.g.:
     * @code
     * for (SomeEnum i : array.keys()) {
     *     // use i and array[i]
     * }
     * @endcode
     *
     * @return                          View over the valid indices for the elements of this indexed array.
     */
    constexpr Segment<key_type> indices() const {
        return Segment(ActualFirstIndex, ActualLastIndex);
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
        return base_type::at(static_cast<ptrdiff_t>(n) - static_cast<ptrdiff_t>(ActualFirstIndex));
    }

    constexpr const_reference at(key_type n) const {
        return base_type::at(static_cast<ptrdiff_t>(n) - static_cast<ptrdiff_t>(ActualFirstIndex));
    }

    constexpr reference operator[](key_type n) noexcept {
        return base_type::operator[](static_cast<ptrdiff_t>(n) - static_cast<ptrdiff_t>(ActualFirstIndex));
    }

    constexpr const_reference operator[](key_type n) const noexcept {
        return base_type::operator[](static_cast<ptrdiff_t>(n) - static_cast<ptrdiff_t>(ActualFirstIndex));
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
