#pragma once

#include <cassert>
#include <array>
#include <type_traits>

#include <unordered_map>

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
