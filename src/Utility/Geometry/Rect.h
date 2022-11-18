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
    Rect(Point<T> topLeft, Size<T> size): x(topLeft.x), y(topLeft.y), w(size.w), h(size.h) {}

    bool Contains(const Point<T> &point) {
        return x <= point.x && point.x < x + w && y <= point.y && point.y < y + h;
    }

    Point<T> TopLeft() const {
        return {x, y};
    }

    Point<T> BottomRight() const {
        return {x + w, y + h};
    }

    Point<T> Center() const {
        return {x + w / 2, y + h / 2};
    }

    ::Size<T> Size() const {
        return {w, h};
    }

    friend bool operator==(const Rect &l, const Rect &r) = default;
};

using Recti = Rect<int>;
