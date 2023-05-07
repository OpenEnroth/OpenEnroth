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

template<class Via, class T>
auto appendVia(std::vector<T> *dst) {
    return detail::AppendViaDstVector<Via, T>(dst);
}

template<class Via, class T>
void Deserialize(InputStream &src, detail::AppendViaDstVector<Via, T> dst) {
    static_assert(!std::is_same_v<Via, T>, "Intermediate and target types must be different.");

    uint32_t size;
    Deserialize(src, &size);

    Via tmp;
    for (size_t i = 0; i < size; i++) {
        Deserialize(src, &tmp);
        Deserialize(tmp, &dst.dst->emplace_back());
    }
}

template<class Via, class T>
auto via(T *dst) {
    return detail::ViaDst<Via, T>(dst);
}

template<class Via, class T>
void Deserialize(InputStream &src, detail::ViaDst<Via, T> dst) {
    static_assert(!std::is_same_v<Via, T>, "Intermediate and target types must be different.");

    Via tmp;
    Deserialize(src, &tmp);
    Deserialize(tmp, dst.dst);
}
