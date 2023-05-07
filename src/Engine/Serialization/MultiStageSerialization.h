#pragma once

#include <vector>

#include "Library/Binary/BinarySerialization.h"

class InputStream;

namespace detail {
template<class Via, class T>
struct AppendViaDstVector {
    explicit AppendViaDstVector(std::vector<T> *dst): dst(dst) {}
    std::vector<T> *dst;
};
template<class Via, class T>
struct ViaDst {
    explicit ViaDst(T *dst): dst(dst) {}
    T *dst;
};
} // namespace detail

/**
 * Creates a deserialization wrapper for a vector pointer that instructs the binary serialization framework to read
 * a vector of `Via` objects from the stream, and then deserialize them into game objects & append those to the target
 * vector.
 *
 * @tparam Via                          Intermediate type to read from the stream.
 * @param dst                           Target vector to append to.
 * @return                              Wrapper object to be passed into `deserialize` call.
 */
template<class Via, class T>
auto appendVia(std::vector<T> *dst) {
    return detail::AppendViaDstVector<Via, T>(dst);
}

template<class Via, class T>
void deserialize(InputStream &src, detail::AppendViaDstVector<Via, T> dst) {
    static_assert(!std::is_same_v<Via, T>, "Intermediate and target types must be different.");

    uint32_t size;
    deserialize(src, &size);

    Via tmp;
    for (size_t i = 0; i < size; i++) {
        deserialize(src, &tmp);
        deserialize(tmp, &dst.dst->emplace_back());
    }
}

/**
 * Creates a deserialization wrapper that instructs the binary serialization framework to first read a `Via` object
 * from a stream, and then deserialize it into a game object pointed to by `dst`.
 *
 * @tparam Via                          Intermediate type to read from the stream.
 * @param dst                           Target object to deserialize.
 * @return                              Wrapper object to be passed into `deserialize` call.
 */
template<class Via, class T>
auto via(T *dst) {
    return detail::ViaDst<Via, T>(dst);
}

template<class Via, class T>
void deserialize(InputStream &src, detail::ViaDst<Via, T> dst) {
    static_assert(!std::is_same_v<Via, T>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp);
    deserialize(tmp, dst.dst);
}
