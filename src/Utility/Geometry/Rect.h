#pragma once

#include "Point.h"
#include "Size.h"

template<class T>
struct Rect {
    T x = 0;
    T y = 0;
    T w = 0;
    T h = 0;

    Rect() = default;
    Rect(T x, T y, T w, T h): x(x), y(y), w(w), h(h) {}

    bool Contains(const Point<T> &point) {
        return x <= point.x && point.x < x + w && y <= point.y && point.y < y + h;
    }

    Point<T> TopLeft() const {
        return {x, y};
    }

    Point<T> BottomRight() const {
        return {x + w, y + h};
    }

    ::Size<T> Size() const {
        return {w, h};
    }
};

using Recti = Rect<int>;
