#pragma once

#include <span>
#include <cassert>
#include <array>
#include <vector>
#include <type_traits>

#include "MemCopySerialization.h"

namespace detail {
struct UnsizedTag {};

struct PresizedTag {
    explicit PresizedTag(size_t size) : size(size) {}
    size_t size;
};

struct AppendTag {};

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

/**
 * Creates a serialization tag that instructs the binary serialization framework to NOT write the vector size
 * into the stream. Note that this implies that you'll have to use the `presized` tag when deserializing.
 *
 * @return                              Tag object to be passed into the `serialize` call.
 * @see presized
 */
inline detail::UnsizedTag unsized() {
    return {};
}

/**
 * Creates a deserialization tag that instructs the binary serialization to use the supplied vector size instead of
 * reading it from the stream. Note that this implies that the serialization must have been performed with the
 * `unsized` tag.
 *
 * @param size                          Number of elements to deserialize.
 * @return                              Tag object to be passed into the `deserialize` call.
 * @see unsized
 */
inline detail::PresizedTag presized(size_t size) {
    return detail::PresizedTag(size);
}

/**
 * Creates a deserialization tag that instructs the binary serialization framework to append deserialized elements
 * into the target vector instead of replacing its contents. This is useful when you want to deserialize several
 * sequences into the same `std::vector` object - normally subsequent `deserialize` calls would replace the vector
 * contents, but using this wrapper lets you accumulate the results of several `deserialize` calls instead.
 *
 * @return                              Tag object to be passed into `deserialize` call.
 */
inline detail::AppendTag append() {
    return {};
}

template<class T>
void serialize(const std::vector<T> &src, OutputStream *dst, detail::UnsizedTag) {
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
void deserialize(InputStream &src, std::vector<T> *dst, detail::AppendTag) {
    uint32_t size;
    deserialize(src, &size);
    detail::deserializeAppend(src, dst, size);
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst, detail::PresizedTag tag) {
    dst->clear();
    detail::deserializeAppend(src, dst, tag.size);
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst) {
    dst->clear();
    deserialize(src, dst, append());
}

