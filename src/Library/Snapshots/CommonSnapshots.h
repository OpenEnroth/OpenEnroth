#pragma once

#include <cstring>
#include <algorithm>
#include <array>
#include <string>
#include <span> // NOLINT
#include <type_traits>

#include "Library/Binary/BinaryConcepts.h"

#include "Utility/Segment.h"
#include "Utility/IndexedArray.h"

#include "SnapshotConcepts.h"

//
// Identity snapshotting.
//

template<class T>
void snapshot(const T &src, T *dst) {
    *dst = src;
}

template<class T>
void reconstruct(const T &src, T *dst) {
    *dst = src;
}


//
// Standard conversions support.
//

template<class From, class To>
struct ConvertTag {};

namespace tags {
template<class From, class To>
constexpr ConvertTag<From, To> convert;
} // namespace tags

template<class T1, class T2>
void snapshot(const T1 &src, T2 *dst, ConvertTag<T1, T2>) {
    *dst = src;
}

template<class T1, class T2>
void reconstruct(const T1 &src, T2 *dst, ConvertTag<T1, T2>) {
    *dst = src;
}


//
// std::string to/from std::array support.
//

template<size_t N>
void snapshot(const std::string &src, std::array<char, N> *dst) {
    memset(dst->data(), 0, N);
    memcpy(dst->data(), src.data(), std::min(src.size(), N - 1));
}

template<size_t N>
void reconstruct(const std::array<char, N> &src, std::string *dst) {
    const char *end = static_cast<const char *>(memchr(src.data(), 0, N));
    size_t size = end == nullptr ? N : end - src.data();
    *dst = std::string(src.data(), size);
}


//
// std::vector support.
//

template<ResizableContiguousContainer Src, ResizableContiguousContainer Dst, class... Tags> requires DifferentElementTypes<Src, Dst>
void snapshot(const Src &src, Dst *dst, const Tags &... tags) {
    dst->resize(src.size());

    std::span srcSpan(src.data(), src.size());
    std::span dstSpan(dst->data(), dst->size());
    for (size_t i = 0; i < srcSpan.size(); i++)
        snapshot(srcSpan[i], &dstSpan[i], tags...);
}

template<ResizableContiguousContainer Src, ResizableContiguousContainer Dst, class... Tags> requires DifferentElementTypes<Src, Dst>
void reconstruct(const Src &src, Dst *dst, const Tags &... tags) {
    dst->resize(src.size());

    std::span srcSpan(src.data(), src.size());
    std::span dstSpan(dst->data(), dst->size());
    for (size_t i = 0; i < srcSpan.size(); i++)
        reconstruct(srcSpan[i], &dstSpan[i], tags...);
}


//
// std::array support.
//

template<class T1, size_t N1, class T2, size_t N2, class... Tags> requires (!std::is_same_v<T1, T2>)
void snapshot(const std::array<T1, N1> &src, std::array<T2, N2> *dst, const Tags &... tags) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        snapshot(src[i], &(*dst)[i], tags...);
}

template<class T1, size_t N1, class T2, size_t N2, class... Tags> requires (!std::is_same_v<T1, T2>)
void reconstruct(const std::array<T1, N1> &src, std::array<T2, N2> *dst, const Tags &... tags) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        reconstruct(src[i], &(*dst)[i], tags...);
}


//
// IndexedArray support
//

template<class T1, size_t N, class T2, auto L, auto H, class... Tags>
void snapshot(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst, const Tags &... tags) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : src.indices())
        snapshot(src[index], &(*dst)[i++], tags...);
}

template<class T1, size_t N, class T2, auto L, auto H, class... Tags>
void reconstruct(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst, const Tags &... tags) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : dst->indices())
        reconstruct(src[i++], &(*dst)[index], tags...);
}


//
// Crude IndexedSpan support.
//

template<auto FirstIndex, auto LastIndex>
struct SegmentTag {
    static constexpr size_t SIZE = static_cast<ptrdiff_t>(LastIndex) - static_cast<ptrdiff_t>(FirstIndex) + 1;

    constexpr Segment<decltype(FirstIndex)> segment() const {
        return Segment(FirstIndex, LastIndex);
    }
};

namespace tags {
template<auto First, auto Last>
constexpr SegmentTag<First, Last> segment;
} // namespace tags

template<class T1, size_t N, class T2, auto L, auto H, auto LL, auto HH>
void snapshot(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst, SegmentTag<LL, HH> tag) {
    static_assert(L <= LL && HH <= H && SegmentTag<LL, HH>::SIZE == N);
    for (size_t i = 0; auto index : tag.segment())
        snapshot(src[index], &(*dst)[i++]);
}

template<class T1, size_t N, class T2, auto L, auto H, auto LL, auto HH>
void reconstruct(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst, SegmentTag<LL, HH> tag) {
    static_assert(L <= LL && HH <= H && SegmentTag<LL, HH>::SIZE == N);
    for (size_t i = 0; auto index : tag.segment())
        reconstruct(src[i++], &(*dst)[index]);
}
