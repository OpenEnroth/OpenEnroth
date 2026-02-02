#pragma once

#include <type_traits>

#include "Library/Binary/BinarySerialization.h"

template<class Via>
struct ViaTag {};

template<class Via>
struct ViaEachTag {};

template<class Via>
struct is_greedy_tag<ViaTag<Via>> : std::true_type {};

namespace tags {
/**
 * Serialization tag that instructs the binary serialization framework to first read a `Via` object
 * from a stream, and then use it to reconstruct the target object.
 *
 * Example usage:
 * ```
 * SaveGameHeader header;
 * deserialize(headerBlob, &header, tags::via<SaveGameHeader_MM7>);
 * ```
 *
 * @tparam Via                          Intermediate type to read from the stream.
 */
template<class Via>
constexpr ViaTag<Via> via;

/**
 * Serialization tag for containers that instructs the binary serialization framework to use `tags::via<Via>`
 * for each element in the container.
 *
 * Example usage:
 * ```
 * std::vector<PortraitFrameData> frames;
 * deserialize(blob, &frames, tags::presized(count), tags::via_each<PortraitFrameData_MM7>);
 * ```
 *
 * @tparam Via                          Intermediate type to use for each element.
 */
template<class Via>
constexpr ViaEachTag<Via> via_each;

} // namespace tags

template<class Src, class Dst, class Via>
void deserialize(Src &&src, Dst *dst, ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp);
    reconstruct(tmp, dst);
}

template<class Src, class Dst, class Via>
void serialize(const Src &src, Dst *dst, ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp);
    serialize(tmp, dst);
}

template<StdSpan Dst, class Via>
void deserialize(InputStream &src, Dst *dst, ViaEachTag<Via>) {
    for (auto &element : *dst)
        deserialize(src, &element, tags::via<Via>);
}

template<StdSpan Src, class Via>
void serialize(const Src &src, OutputStream *dst, ViaEachTag<Via>) {
    for (const auto &element : src)
        serialize(element, dst, tags::via<Via>);
}
