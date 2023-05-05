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
            Deserialize(src, &element);
    }
}

template<class T, size_t N>
void SerializeSpan(std::span<const T, N> src, OutputStream *dst) {
    if constexpr (is_memcopy_serializable_v<T>) {
        dst->write(src.data(), src.size() * sizeof(T));
    } else {
        for (const T &element : src)
            Serialize(element, dst);
    }
}

template<class T>
void DeserializeAppend(InputStream &src, std::vector<T> *dst, size_t size) {
    dst->resize(dst->size() + size);
    T *end = dst->data() + dst->size();

    DeserializeSpan(src, std::span(end - size, end));
}
} // namespace detail


//
// std::array support.
//

template<class T, size_t N>
void Serialize(const std::array<T, N> &src, OutputStream *dst) {
    detail::SerializeSpan(std::span(src), dst);
}

template<class T, size_t N>
void Deserialize(InputStream &src, std::array<T, N> *dst) {
    detail::DeserializeSpan(src, std::span(*dst));
}


//
// std::vector support.
//

template<class T>
auto unsized(const std::vector<T> &src) {
    return detail::UnsizedSrcVector<T>(src);
}

template<class T>
auto presized(size_t size, std::vector<T> *dst) {
    return detail::PresizedDstVector<T>(size, dst);
}

template<class T>
auto appendTo(std::vector<T> *dst) {
    return detail::AppendDstVector<T>(dst);
}

template<class T>
void Serialize(detail::UnsizedSrcVector<T> src, OutputStream *dst) {
    detail::SerializeSpan(std::span(src.src), dst);
}

template<class T>
void Serialize(const std::vector<T> &src, OutputStream *dst) {
    assert(src.size() <= UINT32_MAX); // Basically a check for memory corruption.

    uint32_t size = src.size();
    Serialize(size, dst);
    Serialize(unsized(src), dst);
}

template<class T>
void Deserialize(InputStream &src, detail::AppendDstVector<T> dst) {
    uint32_t size;
    Deserialize(src, &size);
    detail::DeserializeAppend(src, dst.dst, size);
}

template<class T>
void Deserialize(InputStream &src, detail::PresizedDstVector<T> dst) {
    dst.dst->clear();
    detail::DeserializeAppend(src, dst.dst, dst.size);
}

template<class T>
void Deserialize(InputStream &src, std::vector<T> *dst) {
    dst->clear();
    Deserialize(src, appendTo(dst));
}

