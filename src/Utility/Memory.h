#pragma once

#include <cstring>
#include <cstdint>

template<class T>
inline void memzero(T *dst) {
    memset(dst, 0, sizeof(*dst));
}

inline void memset32(void *ptr, uint32_t value, int count) {
    uint32_t *p = static_cast<uint32_t *>(ptr);
    for (int i = 0; i < count; i++)
        *p++ = value;
}
