#pragma once

#include <bitset>
#include <type_traits>

template<auto FirstIndex, auto LastIndex>
class IndexedBitset {
    static_assert(LastIndex >= FirstIndex, "IndexedBitset must be non-empty");
    static_assert(std::is_enum_v<decltype(FirstIndex)> || std::is_integral_v<decltype(FirstIndex)>, "FirstIndex must be an enum or an integral type");
    static_assert(std::is_same_v<decltype(FirstIndex), decltype(LastIndex)>, "FirstIndex and LastIndex must be of the same type");

 public:
    static constexpr size_t SIZE = static_cast<ptrdiff_t>(LastIndex) - static_cast<ptrdiff_t>(FirstIndex) + 1;
    using key_type = decltype(FirstIndex);

    size_t size() const {
        return _bitset.size();
    }

    void fill(bool value) {
        if (value) {
            set();
        } else {
            reset();
        }
    }

    void set() {
        _bitset.set();
    }

    void reset() {
        _bitset.reset();
    }

    void reset(key_type index) {
        set(index, false);
    }

    void set(key_type index, bool value = true) {
        _bitset.set(static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(FirstIndex), value);
    }

    bool test(key_type index) {
        return _bitset.test(static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(FirstIndex));
    }

    auto operator[](key_type index) {
        checkIndex(index);
        return _bitset[static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(FirstIndex)];
    }

    auto operator[](key_type index) const {
        checkIndex(index);
        return _bitset[static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(FirstIndex)];
    }

    friend bool operator==(const IndexedBitset &l, const IndexedBitset &r) {
        return l._bitset == r._bitset;
    }

 private:
    void checkIndex(key_type index) const {
        assert(index >= FirstIndex && index <= LastIndex);
    }

 private:
    std::bitset<SIZE> _bitset;
};
