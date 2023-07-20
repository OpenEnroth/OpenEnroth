#pragma once

#include <span>
#include <cassert>
#include <array>
#include <vector>
#include <type_traits>

#include "MemCopySerialization.h"
#include "BinaryTags.h"

namespace detail {
template<class T, size_t N>
void deserializeSpan(InputStream & src, std::span<T, N> dst) {
    if constexpr (is_memcopy_serializable_v<T>) {
        size_t bytesExpected = dst.size() * sizeof(T);
        size_t bytesRead = src.read(dst.data(), bytesExpected);
        if (bytesRead != bytesExpected)
            throwBinarySerializationNoMoreDataError(bytesRead % sizeof(T), sizeof(T), typeid(T).name());
    } else {
        for (T &element : dst)
            deserialize(src, &element);
    }
}

template<class T, size_t N>
void serializeSpan(std::span<const T, N> src, OutputStream *dst) {
    if constexpr (is_memcopy_serializable_v<T>) {
        dst->write(src.data(), src.size() * sizeof(T));
    } else {
        for (const T &element : src)
            serialize(element, dst);
    }
}

template<class T>
void deserializeAppend(InputStream &src, std::vector<T> *dst, size_t size) {
    dst->resize(dst->size() + size);
    T *end = dst->data() + dst->size();

    deserializeSpan(src, std::span(end - size, end));
}
} // namespace detail


//
// std::array support.
//

template<class T, size_t N>
void serialize(const std::array<T, N> &src, OutputStream *dst) {
    detail::serializeSpan(std::span(src), dst);
}

template<class T, size_t N>
void deserialize(InputStream &src, std::array<T, N> *dst) {
    detail::deserializeSpan(src, std::span(*dst));
}


//
// std::vector support.
//

template<class T>
void serialize(const std::vector<T> &src, OutputStream *dst, UnsizedTag) {
    detail::serializeSpan(std::span(src), dst);
}

template<class T>
void serialize(const std::vector<T> &src, OutputStream *dst) {
    assert(src.size() <= UINT32_MAX); // Basically a check for memory corruption.

    uint32_t size = src.size();
    serialize(size, dst);
    serialize(src, dst, unsized());
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst, AppendTag) {
    uint32_t size;
    deserialize(src, &size);
    detail::deserializeAppend(src, dst, size);
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst, PresizedTag tag) {
    dst->clear();
    detail::deserializeAppend(src, dst, tag.size);
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst) {
    dst->clear();
    deserialize(src, dst, append());
}

