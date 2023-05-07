#pragma once

#include <span>
#include <cassert>
#include <array>
#include <vector>
#include <type_traits>

#include "MemCopySerialization.h"

namespace detail {
template<class T>
struct UnsizedSrcVector {
    explicit UnsizedSrcVector(const std::vector<T> &src) : src(src) {}
    const std::vector<T> &src;
};

template<class T>
struct PresizedDstVector {
    explicit PresizedDstVector(size_t size, std::vector<T> *dst) : size(size), dst(dst) {}
    size_t size;
    std::vector<T> *dst;
};

template<class T>
struct AppendDstVector {
    explicit AppendDstVector(std::vector<T> *dst) : dst(dst) {}
    std::vector<T> *dst;
};

template<class T, size_t N>
void DeserializeSpan(InputStream &src, std::span<T, N> dst) {
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

    DeserializeSpan(src, std::span(end - size, end));
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
    detail::DeserializeSpan(src, std::span(*dst));
}


//
// std::vector support.
//

/**
 * Creates a serialization wrapper that instructs the binary serialization framework to NOT write the vector size
 * into the stream. Note that this implies that you'll have to use the `presized` wrapper when deserializing.
 *
 * @param src                           Vector to serialize.
 * @return                              Wrapper object to be passed into `serialize` call.
 * @see presized
 */
template<class T>
auto unsized(const std::vector<T> &src) {
    return detail::UnsizedSrcVector<T>(src);
}

/**
 * Creates a deserialization wrapper that instructs the binary serialization to use the supplied vector size instead of
 * reading it from the stream. Note that this implies that the serialization must have been performed with the
 * `unsized` wrapper.
 *
 * @param size                          Number of elements to deserialize.
 * @param dst                           Vector to deserialize into.
 * @return                              Wrapper object to be passed into `deserialize` call.
 * @see unsized
 */
template<class T>
auto presized(size_t size, std::vector<T> *dst) {
    return detail::PresizedDstVector<T>(size, dst);
}

/**
 * Creates a deserialization wrapper that instructs the binary serialization framework to append deserialized elements
 * into the target vector instead of replacing its contents. This is useful when you want to deserialize several
 * sequences into the same `std::vector` object - normally subsequent `deserialize` calls would replace the vector
 * contents, but using this wrapper lets you accumulate the results of several `deserialize` calls instead.
 *
 * @param dst                           Vector to append deserialized elements to.
 * @return                              Wrapper object to be passed into `deserialize` call.
 */
template<class T>
auto appendTo(std::vector<T> *dst) {
    return detail::AppendDstVector<T>(dst);
}

template<class T>
void serialize(detail::UnsizedSrcVector<T> src, OutputStream *dst) {
    detail::serializeSpan(std::span(src.src), dst);
}

template<class T>
void serialize(const std::vector<T> &src, OutputStream *dst) {
    assert(src.size() <= UINT32_MAX); // Basically a check for memory corruption.

    uint32_t size = src.size();
    serialize(size, dst);
    serialize(unsized(src), dst);
}

template<class T>
void deserialize(InputStream &src, detail::AppendDstVector<T> dst) {
    uint32_t size;
    deserialize(src, &size);
    detail::deserializeAppend(src, dst.dst, size);
}

template<class T>
void deserialize(InputStream &src, detail::PresizedDstVector<T> dst) {
    dst.dst->clear();
    detail::deserializeAppend(src, dst.dst, dst.size);
}

template<class T>
void deserialize(InputStream &src, std::vector<T> *dst) {
    dst->clear();
    deserialize(src, appendTo(dst));
}

