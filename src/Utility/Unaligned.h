#pragma once

#include <cstddef>
#include <cstring>
#include <array>
#include <type_traits>

/**
 * Reading unaligned memory is UB in C++, so this is what this wrapper is for.
 *
 * @param data                          Unaligned data to read.
 * @return                              Read data.
 */
template<class T>
T readUnaligned(const void *data) {
    T result;
    memcpy(&result, data, sizeof(T));
    return result;
}

/**
 * A `T` with alignment 1. Use it for fields of packed structs that don't land on an address aligned for `T`. Accessing
 * those through a `T *` or a `T &` is UB, while this one reads and writes through `memcpy` and is safe anywhere.
 */
template<class T>
class Unaligned {
    static_assert(std::is_trivially_copyable_v<T>);

 public:
    Unaligned() = default;

    Unaligned(const T &value) { // NOLINT: constructor is intentionally implicit.
        *this = value;
    }

    Unaligned &operator=(const T &value) {
        memcpy(_bytes.data(), &value, sizeof(T));
        return *this;
    }

    operator T() const {
        return readUnaligned<T>(_bytes.data());
    }

 private:
    std::array<std::byte, sizeof(T)> _bytes;
};
