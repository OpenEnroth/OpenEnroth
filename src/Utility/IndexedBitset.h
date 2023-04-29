#pragma once

#include <bitset>

template<auto FirstIndex, auto LastIndex>
class IndexedBitset {
 public:
    using key_type = decltype(FirstIndex);

    size_t size() const { return LastIndex - FirstIndex + 1; }

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
        checkIndex(index);
        _bitset.set(index - FirstIndex, value);
    }

    bool test(key_type index) {
        checkIndex(index);
        return _bitset.test(index);
    }

    auto operator [](key_type index) {
        checkIndex(index);
        return _bitset[index - FirstIndex];
    }

    auto operator [](key_type index) const {
        checkIndex(index);
        return _bitset[index - FirstIndex];
    }
 private:
    void checkIndex(key_type index) const {
        assert(index >= FirstIndex && index <= LastIndex);
    }

    std::bitset<LastIndex - FirstIndex + 1> _bitset;
};
