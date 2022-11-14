#pragma once

#include <algorithm>

#include "Vec.h"

template<class T>
struct BBox {
    T x1 = 0;
    T x2 = 0;
    T y1 = 0;
    T y2 = 0;
    T z1 = 0;
    T z2 = 0;

    static BBox FromPoint(const Vec3<T> &center, T radius) {
        BBox result;
        result.x1 = center.x - radius;
        result.x2 = center.x + radius;
        result.y1 = center.y - radius;
        result.y2 = center.y + radius;
        result.z1 = center.z - radius;
        result.z2 = center.z + radius;
        return result;
    }

    bool ContainsXY(T x, T y) const {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }

    bool Contains(const Vec3<T> &pos) const {
        return x1 <= pos.x && pos.x <= x2 && y1 <= pos.y && pos.y <= y2 && z1 <= pos.z && pos.z <= z2;
    }

    template<class U>
    bool Intersects(const BBox<U> &other) const {
        return
            this->x1 <= other.x2 && this->x2 >= other.x1 &&
            this->y1 <= other.y2 && this->y2 >= other.y1 &&
            this->z1 <= other.z2 && this->z2 >= other.z1;
    }

    friend BBox operator|(const BBox &l, const BBox &r) {
        BBox result;
        result.x1 = std::min(l.x1, r.x1);
        result.x2 = std::max(l.x2, r.x2);
        result.y1 = std::min(l.y1, r.y1);
        result.y2 = std::max(l.y2, r.y2);
        result.z1 = std::min(l.z1, r.z1);
        result.z2 = std::max(l.z2, r.z2);
        return result;
    }

    // TODO(captainurist): IsCloseTo(bb, dist)
    BBox Expanded(T radius) const {
        BBox result;
        result.x1 = this->x1 - radius;
        result.x2 = this->x2 + radius;
        result.y1 = this->y1 - radius;
        result.y2 = this->y2 + radius;
        result.z1 = this->z1 - radius;
        result.z2 = this->z2 + radius;
        return result;
    }
};

using BBoxi = BBox<int>;
using BBoxs = BBox<short>;
using BBoxf = BBox<float>;

static_assert(sizeof(BBoxi) == 24);
static_assert(sizeof(BBoxs) == 12);
static_assert(sizeof(BBoxf) == 24);
