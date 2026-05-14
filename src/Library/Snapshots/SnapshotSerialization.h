#pragma once

#include <type_traits>

#include "SnapshotTags.h"

template<class Via>
struct ViaTag {};

template<class Via>
struct is_greedy_tag<ViaTag<Via>> : std::true_type {};

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

} // namespace tags

template<class Src, class Dst, class Via, class... Tags>
void deserialize(Src &&src, Dst *dst, ViaTag<Via>, const Tags &... tags) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp);
    reconstruct(tmp, dst, tags...);
}

template<class Src, class Dst, class Via, class... Tags>
void serialize(const Src &src, Dst *dst, ViaTag<Via>, const Tags &... tags) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp, tags...);
    serialize(tmp, dst);
}
