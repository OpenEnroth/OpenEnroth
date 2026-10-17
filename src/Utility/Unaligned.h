#pragma once

#include <cstring>
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
 * Writing unaligned memory is UB in C++, this is the counterpart of `readUnaligned`.
 *
 * @param data                          Unaligned memory to write to.
 * @param value                         Value to write, converted to `T` first.
 */
template<class T>
void writeUnaligned(T *data, const std::type_identity_t<T> &value) {
    memcpy(data, &value, sizeof(T));
}
