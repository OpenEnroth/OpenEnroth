#pragma once

#include <typeinfo>
#include <type_traits>

#include "Utility/Streams/OutputStream.h"
#include "Utility/Streams/InputStream.h"

#include "BinaryExceptions.h"

template<class T>
constexpr std::is_arithmetic<T> isMemCopySerializable(std::type_identity<T>) {
    return {}; // By default only arithmetic (floating point or integral) types are memcopy-serializable.
}

/**
 * Type trait that's used by the binary serialization framework to check if a type can be binary-serialized with a
 * simple `memcpy` call.
 *
 * Instead of specializing this trait directly for your class, use `MM_DECLARE_MEMCOPY_SERIALIZABLE`.
 *
 * @tparam T                            Type to get the trait value for.
 * @see is_memcopy_serializable_v
 * @see MM_DECLARE_MEMCOPY_SERIALIZABLE
 */
template<class T>
struct is_memcopy_serializable : decltype(isMemCopySerializable(std::type_identity<T>())) {};

template<class T>
constexpr bool is_memcopy_serializable_v = is_memcopy_serializable<T>::value;

/**
 * Invoke this macro for a type to use it with binary serialization functions via simple memory copy.
 *
 * @param T                             Type to declare as memcopy-serializable.
 */
#define MM_DECLARE_MEMCOPY_SERIALIZABLE(T)                                                                              \
constexpr std::true_type isMemCopySerializable(std::type_identity<T>) {                                                 \
    return {};                                                                                                          \
}


template<class T> requires is_memcopy_serializable_v<T>
void serialize(const T &src, OutputStream *dst) {
    dst->write(&src, sizeof(T));
}

template<class T> requires is_memcopy_serializable_v<T>
void deserialize(InputStream &src, T *dst) {
    size_t bytes = src.read(dst, sizeof(T));
    if (bytes != sizeof(T))
        throwBinarySerializationNoMoreDataError(bytes, sizeof(T), typeid(T).name());
}
