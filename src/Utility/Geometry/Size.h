#pragma once

template<class T>
struct Size {
    T w = 0;
    T h = 0;

    Size() = default;
    Size(T w, T h): w(w), h(h) {}
};

using Sizei = Size<int>;
