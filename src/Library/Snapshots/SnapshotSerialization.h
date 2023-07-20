#pragma once

#include <vector>
#include <type_traits>

#include "Library/Binary/BinarySerialization.h"

class InputStream;
namespace detail {
template<class Via>
struct AppendViaTag {};
template<class Via>
struct ViaTag {};
} // namespace detail

/**
 * Creates a deserialization tag that instructs the binary serialization framework to read a vector of `Via` objects
 * from the stream, and then reconstruct them into game objects & append those to the target vector.
 *
 * Example usage:
 * ```
 * std::vector<MonsterDesc> monsters;
 * Blob monstersBlob = readMonstersBlob();
 * deserialize(monstersBlob, &monsters, appendVia<MonsterDesc_MM7>());
 * ```
 *
 * @tparam Via                          Intermediate type to read from the stream.
 * @return                              Tag object to be passed into the `deserialize` call.
 */
template<class Via>
detail::AppendViaTag<Via> appendVia() {
    return {};
}

template<class Dst, class Via>
void deserialize(InputStream &src, std::vector<Dst> *dst, detail::AppendViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    uint32_t size;
    deserialize(src, &size);

    Via tmp;
    for (size_t i = 0; i < size; i++) {
        deserialize(src, &tmp);
        reconstruct(tmp, &dst->emplace_back());
    }
}

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

template<RegularBinarySource Src, class Dst, class Via>
void deserialize(Src &&src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp); // No need for std::forward here, src is never treated as an rvalue.
    reconstruct(tmp, dst);
}

template<class Src, RegularBinaryTarget Dst, class Via>
void serialize(const Src &src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp);
    serialize(tmp, dst);
}
