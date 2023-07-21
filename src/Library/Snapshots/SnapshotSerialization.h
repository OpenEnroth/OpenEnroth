#pragma once

#include <vector>
#include <type_traits>

#include "Library/Binary/BinarySerialization.h"

namespace detail {
template<class Via>
struct ViaTag {};
} // namespace detail

/**
 * Creates a deserialization tag that instructs the binary serialization framework to first read a `Via` object
 * from a stream, and then use it to reconstruct the target object.
 *
 * Example usage:
 * ```
 * SaveGameHeader header;
 * Blob headerBlob = readHeaderBlob();
 * deserialize(headerBlob, &header, via<SaveGameHeader_MM7>());
 * ```
 *
 * @tparam Via                          Intermediate type to read from the stream.
 * @return                              Tag object to be passed into the `deserialize` call.
 */
template<class Via>
detail::ViaTag<Via> via() {
    return {};
}

template<NonBinaryProxy Src, NonBinaryProxy Dst, class Via> requires (!StdSpan<Dst>) // std::span is handled below.
void deserialize(Src &src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp); // No need for std::forward here, src is never treated as an rvalue.
    reconstruct(tmp, dst);
}

template<NonBinaryProxy Src, NonBinaryProxy Dst, class Via> requires (!StdSpan<Src>) // std::span is handled below.
void serialize(const Src &src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp);
    serialize(tmp, dst);
}

template<NonBinaryProxy Src, StdSpan Dst, class Via>
void deserialize(Src &src, Dst *dst, detail::ViaTag<Via> tag) {
    for (auto &element : *dst)
        deserialize(src, &element, tag);
}

template<StdSpan Src, NonBinaryProxy Dst, class Via>
void serialize(const Src &src, Dst *dst, detail::ViaTag<Via> tag) {
    for (const auto &element : src)
        serialize(element, dst, tag);
}
