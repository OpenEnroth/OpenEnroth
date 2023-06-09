#pragma once

#include <cstring>
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <type_traits>

#include "Utility/Segment.h"
#include "Utility/IndexedArray.h"
#include "Utility/IndexedBitset.h"

// TODO(captainurist): rename this file?


//
// Identity serialization.
//

template<class T>
void serialize(const T &src, T *dst) {
    *dst = src;
}

template<class T>
void deserialize(const T &src, T *dst) {
    *dst = src;
}


//
// (De)serialization via standard conversions.
//

namespace detail {
template<class From, class To>
struct ConvertTag {};
} // namespace detail

template<class From, class To>
inline detail::ConvertTag<From, To> convert() {
    return {};
}

template<class T1, class T2>
void serialize(const T1 &src, T2 *dst, detail::ConvertTag<T1, T2>) {
    *dst = src;
}

template<class T1, class T2>
void deserialize(const T1 &src, T2 *dst, detail::ConvertTag<T1, T2>) {
    *dst = src;
}


//
// std::string to/from std::array support.
//

template<size_t N>
void serialize(const std::string &src, std::array<char, N> *dst) {
    memset(dst->data(), 0, N);
    memcpy(dst->data(), src.data(), std::min(src.size(), N - 1));
}

template<size_t N>
void deserialize(const std::array<char, N> &src, std::string *dst) {
    const char *end = static_cast<const char *>(memchr(src.data(), 0, N));
    size_t size = end == nullptr ? N : end - src.data();
    *dst = std::string(src.data(), size);
}


//
// Basic tag support.
//

struct NoTag {};

template<class T1, class T2, class Tag> requires std::is_same_v<Tag, NoTag>
void serialize(const T1 &src, T2 *dst, const Tag &) {
    serialize(src, dst);
}

template<class T1, class T2, class Tag> requires std::is_same_v<Tag, NoTag>
void deserialize(const T1 &src, T2 *dst, const Tag &) {
    deserialize(src, dst);
}


//
// std::vector support.
//

template<class T1, class T2, class Tag = NoTag> requires (!std::is_same_v<T1, T2>)
void serialize(const std::vector<T1> &src, std::vector<T2> *dst, const Tag &tag = {}) {
    dst->clear();
    dst->reserve(src.size());
    for(const T1 &element : src)
        serialize(element, &dst->emplace_back(), tag);
}

template<class T1, class T2, class Tag = NoTag> requires (!std::is_same_v<T1, T2>)
void deserialize(const std::vector<T1> &src, std::vector<T2> *dst, const Tag &tag = {}) {
    dst->clear();
    dst->reserve(src.size());
    for (const T1 &element : src)
        deserialize(element, &dst->emplace_back(), tag);
}


//
// std::array support.
//

template<class T1, size_t N1, class T2, size_t N2, class Tag = NoTag> requires (!std::is_same_v<T1, T2>)
void serialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst, const Tag &tag = {}) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        serialize(src[i], &(*dst)[i], tag);
}

template<class T1, size_t N1, class T2, size_t N2, class Tag = NoTag> requires (!std::is_same_v<T1, T2>)
void deserialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst, const Tag &tag = {}) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        deserialize(src[i], &(*dst)[i], tag);
}


//
// IndexedArray support
//

template<class T1, size_t N, class T2, auto L, auto H, class Tag = NoTag>
void serialize(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst, const Tag &tag = {}) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : src.indices())
        serialize(src[index], &(*dst)[i++], tag);
}

template<class T1, size_t N, class T2, auto L, auto H, class Tag = NoTag>
void deserialize(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst, const Tag &tag = {}) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : dst->indices())
        deserialize(src[i++], &(*dst)[index], tag);
}


//
// Crude IndexedSpan support.
//

namespace detail {
template<auto FirstIndex, auto LastIndex>
struct SegmentTag {
    static constexpr size_t SIZE = static_cast<ptrdiff_t>(LastIndex) - static_cast<ptrdiff_t>(FirstIndex) + 1;

    constexpr Segment<decltype(FirstIndex)> segment() const {
        return Segment(FirstIndex, LastIndex);
    }
};
} // namespace detail

template<auto First, auto Last>
detail::SegmentTag<First, Last> segment() {
    return {};
}

template<class T1, size_t N, class T2, auto L, auto H, auto LL, auto HH>
void serialize(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst, detail::SegmentTag<LL, HH> tag) {
    static_assert(L <= LL && HH <= H && detail::SegmentTag<LL, HH>::SIZE == N);
    for (size_t i = 0; auto index : tag.segment())
        serialize(src[index], &(*dst)[i++]);
}

template<class T1, size_t N, class T2, auto L, auto H, auto LL, auto HH>
void deserialize(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst, detail::SegmentTag<LL, HH> tag) {
    static_assert(L <= LL && HH <= H && detail::SegmentTag<LL, HH>::SIZE == N);
    for (size_t i = 0; auto index : tag.segment())
        deserialize(src[i++], &(*dst)[index]);
}


//
// IndexedBitset support.
//

template<class T, size_t N, auto L, auto H>
void serialize(const IndexedBitset<L, H> &src, std::array<T, N> *dst) {
    assert(dst->size() * sizeof(T) * 8 == src.size());
    size_t i = 1, j = 0;
    while (i < src.size()) {
        T val = 0;
        // Bits inside each array element indexed backwards
        for (size_t k = 0; k < (sizeof(T) * 8); k++, i++) {
            val |= src[i] << ((sizeof(T) * 8) - k - 1);
        }
        serialize(val, &(*dst)[j]);
        j++;
    }
}

template<class T, size_t N, auto L, auto H>
void deserialize(const std::array<T, N> &src, IndexedBitset<L, H> *dst) {
    assert(dst->size() == src.size() * sizeof(T) * 8);
    size_t i = 1, j = 0;
    while (i < dst->size()) {
        T val = 0;
        deserialize(src[j], &val);
        // Bits inside each array element indexed backwards
        for (size_t k = 0; k < (sizeof(T) * 8); k++, i++) {
            dst->set(i, !!(val & (1 << ((sizeof(T) * 8) - k - 1))));
        }
        j++;
    }
}
