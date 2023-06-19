#pragma once

#include <vector>
#include <type_traits>

#include "Library/Binary/BinarySerialization.h"

namespace LOD {
class File;
class WriteableFile;
}

class InputStream;
class OutputStream;

namespace detail {
template<class Via>
struct AppendViaTag {};

template<class Via>
struct ViaTag {};

template<class T>
constexpr bool is_binary_deserialization_source_v = std::is_base_of_v<InputStream, T> || std::is_base_of_v<LOD::File, T>;

template<class T>
constexpr bool is_binary_serialization_target_v = std::is_base_of_v<OutputStream, T> || std::is_base_of_v<LOD::WriteableFile, T>;
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

template<class Src, class Dst, class Via> requires detail::is_binary_deserialization_source_v<std::remove_cvref_t<Src>>
void deserialize(Src &&src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp); // No need for std::forward here, src is never treated as an rvalue.
    reconstruct(tmp, dst);
}

template<class Src, class Dst, class Via> requires detail::is_binary_serialization_target_v<Dst>
void serialize(const Src &src, Dst *dst, detail::ViaTag<Via>) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp);
    serialize(tmp, dst);
}
