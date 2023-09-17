#pragma once

#include <cstring>

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
