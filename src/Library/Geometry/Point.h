#pragma once

#include "Vec.h"
#include "Size.h"

template<class T>
using Point = Vec2<T>;

using Pointi = Point<int>;

template<class T>
Point<T> operator-(Point<T> l, Size<T> r) {
    return {l.x - r.w, l.y - r.h};
}
