#pragma once

template<class T>
struct Size {
    T w = 0;
    T h = 0;

    Size() = default;
    Size(T w, T h): w(w), h(h) {}

    bool operator==(const Size &c) const {
        return c.h == h && c.w == w;
    }
};

using Sizei = Size<int>;
using Sizef = Size<float>;
