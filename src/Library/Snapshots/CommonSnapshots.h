#pragma once

#include <cstring>
#include <algorithm>
#include <array>
#include <string>
#include <span>
#include <type_traits>
#include <deque>
#include <vector>

#include "Library/Binary/BinaryConcepts.h"

#include "Utility/Segment.h"
#include "Utility/IndexedArray.h"
#include "Utility/IndexedBitset.h"

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
// static_cast support.
//

template<class From, class To>
struct CastTag {};

namespace tags {
template<class From, class To>
constexpr CastTag<From, To> cast;
} // namespace tags

template<class T1, class T2>
void snapshot(const T1 &src, T2 *dst, CastTag<T1, T2>) {
    *dst = static_cast<T2>(src);
}

template<class T1, class T2>
void reconstruct(const T1 &src, T2 *dst, CastTag<T1, T2>) {
    *dst = static_cast<T2>(src);
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
// std::deque to std::vector support for pActors.
//

template<class T1, class T2, class... Tags>
void snapshot(const std::deque<T1> &src, std::vector<T2> *dst, const Tags &... tags) {
    dst->resize(src.size());

    for (size_t i = 0; i < src.size(); i++)
        snapshot(src[i], &(*dst)[i], tags...);
}

template<class T1, class T2, class... Tags>
void reconstruct(const std::vector<T1> &src, std::deque<T2> *dst, const Tags &... tags) {
    dst->resize(src.size());

    for (size_t i = 0; i < src.size(); i++)
        reconstruct(src[i], &(*dst)[i], tags...);
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


//
// IndexedBitset support.
// MM uses inverted bit order for serialization, we don't want to have this as default behavior, so we introduce a tag.
//
struct ReverseBitOrderTag {};
namespace tags {
constexpr ReverseBitOrderTag reverseBits;
} // namespace tags

template<size_t N, auto L, auto H>
static void snapshot(const IndexedBitset<L, H> &src, std::array<uint8_t, N> *dst, ReverseBitOrderTag) {
    assert(dst->size() * 8 == src.size());

    dst->fill(0);
    for (size_t i = 0; auto index : src.indices()) {
        (*dst)[i / 8] |= src[index] << (7 - i % 8);
        i++;
    }
}

template<size_t N, auto L, auto H>
static void reconstruct(const std::array<uint8_t, N> &src, IndexedBitset<L, H> *dst, ReverseBitOrderTag) {
    assert(dst->size() == src.size() * 8);

    for (size_t i = 0; auto index : dst->indices()) {
        dst->set(index, (src[i / 8] >> (7 - i % 8)) & 1);
        i++;
    }
}
