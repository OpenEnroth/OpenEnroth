#pragma once

#include "Library/Binary/BinarySerialization.h"

#include "Utility/String/Encoding.h"
#include "Utility/Segment.h"

template<class Via>
struct ViaTag {};

template<class Via>
struct is_greedy_tag<ViaTag<Via>> : std::true_type {};

template<class From, class To>
struct CastTag {};

template<auto FirstIndex, auto LastIndex>
struct SegmentTag {
    static constexpr size_t SIZE = static_cast<ptrdiff_t>(LastIndex) - static_cast<ptrdiff_t>(FirstIndex) + 1;

    constexpr Segment<decltype(FirstIndex)> segment() const {
        return Segment(FirstIndex, LastIndex);
    }
};

struct ReverseBitOrderTag {};

namespace tags {
/**
 * Serialization tag that instructs the binary serialization framework to first read a `Via` object
 * from a stream, and then use it to reconstruct the target object. All tags that follow `tags::via` in
 * the argument list are forwarded to the `snapshot` / `reconstruct` call on the intermediate type.
 *
 * Example usage:
 * ```
 * SaveGameHeader header;
 * deserialize(headerBlob, &header, tags::via<SaveGameHeader_MM7>, tags::encoding(ENCODING_ASCII));
 * ```
 *
 * @tparam Via                          Intermediate type to read from the stream.
 */
template<class Via>
constexpr ViaTag<Via> via;

/**
 * Snapshot tag that performs a `static_cast` between source and destination types during snapshot/reconstruct.
 * Both template parameters must exactly match the types passed to `snapshot` / `reconstruct`.
 *
 * Example usage:
 * ```
 * uint32_t src = 42;
 * uint8_t dst;
 * snapshot(src, &dst, tags::cast<uint32_t, uint8_t>);
 * ```
 *
 * @tparam From                         Source type for the cast.
 * @tparam To                           Destination type for the cast.
 */
template<class From, class To>
constexpr CastTag<From, To> cast;

/**
 * Snapshot tag that extracts a sub-range from an `IndexedArray` into a `std::array` during snapshot/reconstruct.
 * The array size must match the number of elements in the `[First, Last]` range.
 *
 * Example usage:
 * ```
 * IndexedArray<int, ITEM_FIRST, ITEM_LAST> src;
 * std::array<int, 10> dst;
 * snapshot(src, &dst, tags::segment<ITEM_FIRST, ITEM_10>);
 * ```
 *
 * @tparam First                        First index of the range (inclusive).
 * @tparam Last                         Last index of the range (inclusive).
 */
template<auto First, auto Last>
constexpr SegmentTag<First, Last> segment;

/**
 * Snapshot tag that reverses the bit order within each byte during `IndexedBitset` snapshot/reconstruct.
 * MM7 uses inverted bit order for bitset serialization, so this tag is needed to match the on-disk format.
 *
 * Example usage:
 * ```
 * IndexedBitset<1, 32> bits;
 * std::array<uint8_t, 4> bytes;
 * snapshot(bits, &bytes, tags::reverseBits);
 * ```
 */
constexpr ReverseBitOrderTag reverseBits;
} // namespace tags
