#pragma once

#include "ArraySerialization.h"
#include "BlobSerialization.h"
#include "MemCopySerialization.h"

namespace detail {
struct BlobSerializer {
    template<class T>
    Blob operator()(const T &value) const {
        Blob result;
        serialize(value, &result);
        return result;
    }
};

template<class T>
struct BlobDeserializer {
    T operator()(const Blob &blob) const {
        T result;
        deserialize(blob, &result);
        return result;
    }
};
} // namespace detail

/**
 * Ranges-friendly binary serialization object.
 *
 * Can be used directly as `return toBlob(someStruct)`, or passed into `std::views::transform`.
 *
 * Under the hood it's calling the `serialize` function using argument-dependent lookup.
 *
 * @see fromBlob
 */
constexpr detail::BlobSerializer toBlob;

/**
 * Ranges-friendly binary deserialization object.
 *
 * Can be used directly as `return fromBlob<T>(blob)`, or passed into `std::views::transform`.
 *
 * Under the hood it's calling the `deserialize` function using argument-dependent lookup.
 *
 * @see toBlob
 */
template<class T>
constexpr detail::BlobDeserializer<T> fromBlob;
