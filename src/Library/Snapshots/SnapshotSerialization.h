#pragma once

#include <vector>
#include <type_traits>

#include "Library/Binary/BinarySerialization.h"

template<class Via>
struct ViaTag {};

namespace tags {
/**
 * Deserialization tag that instructs the binary serialization framework to first read a `Via` object
 * from a stream, and then use it to reconstruct the target object.
 *
 * Example usage:
 * ```
 * SaveGameHeader header;
 * Blob headerBlob = readHeaderBlob();
 * deserialize(headerBlob, &header, tags::via<SaveGameHeader_MM7>);
 * ```
 *
 * @tparam Via                          Intermediate type to read from the stream.
 */
template<class Via>
constexpr ViaTag<Via> via;
} // namespace tags

// Note: in theory the overloads below could accept a Tags... tail parameter and forward it to reconstruct / snapshot,
// but I can't think of any sane use case where this would be needed. So, no tag tails.

template<NonBinaryProxy Src, NonBinaryProxy Dst, class Via> requires (!StdSpan<Dst>) // std::span is handled below.
void deserialize(Src &src, Dst *dst, ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp); // No need for std::forward here, src is never treated as an rvalue.
    reconstruct(tmp, dst);
}

template<NonBinaryProxy Src, NonBinaryProxy Dst, class Via> requires (!StdSpan<Src>) // std::span is handled below.
void serialize(const Src &src, Dst *dst, ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp);
    serialize(tmp, dst);
}

template<NonBinaryProxy Src, StdSpan Dst, class Via>
void deserialize(Src &src, Dst *dst, ViaTag<Via> tag) {
    for (auto &element : *dst)
        deserialize(src, &element, tag);
}

template<StdSpan Src, NonBinaryProxy Dst, class Via>
void serialize(const Src &src, Dst *dst, ViaTag<Via> tag) {
    for (const auto &element : src)
        serialize(element, dst, tag);
}
