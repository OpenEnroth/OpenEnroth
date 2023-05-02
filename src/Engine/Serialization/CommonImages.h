#pragma once

#include <cstring>
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <type_traits>

template<class T>
static void Serialize(const T &src, T *dst) {
    *dst = src;
}

template<class T>
static void Deserialize(const T &src, T *dst) {
    *dst = src;
}

template<class T1, class T2> requires (!std::is_same_v<T1, T2>)
void Serialize(const std::vector<T1> &src, std::vector<T2> *dst) {
    dst->clear();
    dst->reserve(src.size());
    for(const T1 &element : src)
        Serialize(element, &dst->emplace_back());
}

template<class T1, class T2> requires (!std::is_same_v<T1, T2>)
void Deserialize(const std::vector<T1> &src, std::vector<T2> *dst) {
    dst->clear();
    dst->reserve(src.size());
    for (const T1 &element : src)
        Deserialize(element, &dst->emplace_back());
}

template<class T1, size_t N1, class T2, size_t N2> requires (!std::is_same_v<T1, T2>)
static void Serialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        Serialize(src[i], &(*dst)[i]);
}

template<class T1, size_t N1, class T2, size_t N2> requires (!std::is_same_v<T1, T2>)
static void Deserialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        Deserialize(src[i], &(*dst)[i]);
}

template<size_t N>
void Serialize(const std::string &src, std::array<char, N> *dst) {
    memset(dst->data(), 0, N);
    memcpy(dst->data(), src.data(), std::min(src.size(), N - 1));
}

template<size_t N>
void Deserialize(const std::array<char, N> &src, std::string *dst) {
    const char *end = static_cast<const char *>(memchr(src.data(), 0, N));
    size_t size = end == nullptr ? N : end - src.data();
    *dst = std::string(src.data(), size);
}
