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
    Rect(Point<T> topLeft, Point<T> bottomRight) : x(topLeft.x), y(topLeft.y), w(bottomRight.x - topLeft.x), h(bottomRight.y - topLeft.y) {}

    bool contains(const Point<T> &point) {
        return x <= point.x && point.x < x + w && y <= point.y && point.y < y + h;
    }

    Point<T> topLeft() const {
        return {x, y};
    }

    Point<T> bottomRight() const {
        return {x + w, y + h};
    }

    Point<T> topRight() const {
        return {x + w, y};
    }

    Point<T> bottomLeft() const {
        return {x, y + h};
    }

    Point<T> center() const {
        return {x + w / 2, y + h / 2};
    }

    Size<T> size() const {
        return {w, h};
    }

    friend bool operator==(const Rect &l, const Rect &r) = default;
};

using Recti = Rect<int>;
