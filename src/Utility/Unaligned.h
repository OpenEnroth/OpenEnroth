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
 * Writing through a misaligned `T *` is UB in C++, so this is the counterpart of `readUnaligned`.
 *
 * @param data                          Unaligned memory to write to.
 * @param value                         Value to write.
 */
template<class T>
void writeUnaligned(void *data, const T &value) {
    memcpy(data, &value, sizeof(T));
}

/**
 * A `T` with alignment 1, for a packed struct field that doesn't land on an address aligned for `T` and is accessed
 * through a pointer or a reference. A `T *` or a `T &` to such a field is UB to use, while this one reads and writes
 * through `memcpy` and is safe anywhere. Plain member access to a packed field is fine without it.
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
        writeUnaligned(_bytes.data(), value);
        return *this;
    }

    operator T() const {
        return readUnaligned<T>(_bytes.data());
    }

 private:
    std::array<std::byte, sizeof(T)> _bytes;
};
