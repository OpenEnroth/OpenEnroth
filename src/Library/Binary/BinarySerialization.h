#pragma once

#include "ArraySerialization.h"
#include "BlobSerialization.h"
#include "MemCopySerialization.h"

namespace detail {

struct BlobSerializer {
    template<class T>
    Blob operator()(const T &value) const {
        Blob result;
        Serialize(value, &result);
        return result;
    }
};

template<class T>
struct BlobDeserializer {
    T operator()(const Blob &blob) const {
        T result;
        Deserialize(blob, &result);
        return result;
    }
};

} // namespace detail

constexpr detail::BlobSerializer toBlob;

template<class T>
constexpr detail::BlobDeserializer<T> fromBlob;
