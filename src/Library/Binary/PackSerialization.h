#pragma once

#include <cstddef>
#include <utility>

#include "Utility/Streams/OutputStream.h"
#include "Utility/Streams/InputStream.h"

#include "BinaryTags.h"

namespace detail {

template<class T, class... Tags, size_t... Indices>
void serializePack(const T &src, OutputStream *dst, const TagPack<Tags...> &pack, std::index_sequence<Indices...>) {
    serialize(src, dst, std::get<Indices>(pack.tags)...);
}

template<class T, class... Tags, size_t... Indices>
void deserializePack(InputStream &src, T *dst, const TagPack<Tags...> &pack, std::index_sequence<Indices...>) {
    deserialize(src, dst, std::get<Indices>(pack.tags)...);
}

} // namespace detail

template<class T, class... Tags>
void serialize(const T &src, OutputStream *dst, const TagPack<Tags...> &pack) {
    detail::serializePack(src, dst, pack, std::index_sequence_for<Tags...>());
}

template<class T, class... Tags>
void deserialize(InputStream &src, T *dst, const TagPack<Tags...> &pack) {
    detail::deserializePack(src, dst, pack, std::index_sequence_for<Tags...>());
}
