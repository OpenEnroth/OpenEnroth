#pragma once

template<class T>
struct Rect {
    T x = 0;
    T y = 0;
    T w = 0;
    T h = 0;

    Rect() = default;
    Rect(T x, T y, T w, T h): x(x), y(y), w(w), h(h) {}
};

using Recti = Rect<int>;
