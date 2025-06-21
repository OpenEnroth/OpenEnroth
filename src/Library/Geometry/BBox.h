#pragma once

#include <cassert>
#include <algorithm>
#include <tuple> // For std::tie.

#include "Vec.h"

template<class T>
struct BBox {
    T x1 = 0;
    T x2 = 0;
    T y1 = 0;
    T y2 = 0;
    T z1 = 0;
    T z2 = 0;

    /**
     * @param center                    Center of the bounding box.
     * @param halfSide                  Half the length of the edge of the resulting bounding box cube.
     * @return                          Cubic bounding box centered at `center` with sides twice the `halfSide`.
     */
    [[nodiscard]] static BBox cubic(const Vec3<T> &center, T halfSide) {
        assert(halfSide >= 0);

        BBox result;
        result.x1 = center.x - halfSide;
        result.x2 = center.x + halfSide;
        result.y1 = center.y - halfSide;
        result.y2 = center.y + halfSide;
        result.z1 = center.z - halfSide;
        result.z2 = center.z + halfSide;
        return result;
    }

    /**
     * @param a                         Point a.
     * @param b                         Point b.
     * @return                          Bounding box containing both passed points.
     */
    [[nodiscard]] static BBox forPoints(const Vec3<T> &a, const Vec3<T> &b) {
        BBox result;
        std::tie(result.x1, result.x2) = std::minmax(a.x, b.x);
        std::tie(result.y1, result.y2) = std::minmax(a.y, b.y);
        std::tie(result.z1, result.z2) = std::minmax(a.z, b.z);
        return result;
    }

    template<class Range>
    [[nodiscard]] static BBox forPoints(const Range &range) {
        auto pos = std::begin(range);
        auto end = std::end(range);
        if (pos == end)
            return BBox();

        BBox result;
        result.x1 = pos->x;
        result.x2 = pos->x;
        result.y1 = pos->y;
        result.y2 = pos->y;
        result.z1 = pos->z;
        result.z2 = pos->z;

        while (++pos != end) {
            result.x1 = std::min(result.x1, pos->x);
            result.x2 = std::max(result.x2, pos->x);
            result.y1 = std::min(result.y1, pos->y);
            result.y2 = std::max(result.y2, pos->y);
            result.z1 = std::min(result.z1, pos->z);
            result.z2 = std::max(result.z2, pos->z);
        }

        return result;
    }

    [[nodiscard]] static BBox forCylinder(const Vec3<T> bottomCenter, T radius, T height) {
        assert(radius >= 0 && height >= 0);

        BBox result;
        result.x1 = bottomCenter.x - radius;
        result.x2 = bottomCenter.x + radius;
        result.y1 = bottomCenter.y - radius;
        result.y2 = bottomCenter.y + radius;
        result.z1 = bottomCenter.z;
        result.z2 = bottomCenter.z + height;
        return result;
    }

    [[nodiscard]] bool containsXY(T x, T y) const {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }

    [[nodiscard]] bool contains(const Vec3<T> &pos) const {
        return x1 <= pos.x && pos.x <= x2 && y1 <= pos.y && pos.y <= y2 && z1 <= pos.z && pos.z <= z2;
    }

    template<class U>
    [[nodiscard]] bool intersects(const BBox<U> &other) const {
        return
            x1 <= other.x2 && x2 >= other.x1 &&
            y1 <= other.y2 && y2 >= other.y1 &&
            z1 <= other.z2 && z2 >= other.z1;
    }

    template<class U, class V>
    [[nodiscard]] bool intersectsCube(const Vec3<U> &center, V halfSide) const {
        assert(halfSide >= 0);

        return
            x1 <= center.x + halfSide && x2 >= center.x - halfSide &&
            y1 <= center.y + halfSide && y2 >= center.y - halfSide &&
            z1 <= center.z + halfSide && z2 >= center.z - halfSide;
    }

    template<class U>
    [[nodiscard]] bool intersectsCuboid(const Vec3<U> &center, const Vec3<U> &halfSize) const {
        assert(halfSize.x >= 0 && halfSize.y >= 0 && halfSize.z >= 0);

        return
            x1 <= center.x + halfSize.x && x2 >= center.x - halfSize.x &&
            y1 <= center.y + halfSize.y && y2 >= center.y - halfSize.y &&
            z1 <= center.z + halfSize.z && z2 >= center.z - halfSize.z;
    }

    [[nodiscard]] friend BBox operator|(const BBox &l, const BBox &r) {
        BBox result;
        result.x1 = std::min(l.x1, r.x1);
        result.x2 = std::max(l.x2, r.x2);
        result.y1 = std::min(l.y1, r.y1);
        result.y2 = std::max(l.y2, r.y2);
        result.z1 = std::min(l.z1, r.z1);
        result.z2 = std::max(l.z2, r.z2);
        return result;
    }

    [[nodiscard]] Vec3<T> center() const {
        return Vec3<T>((x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2);
    }

    [[nodiscard]] Vec3<T> size() const {
        return Vec3<T>(x2 - x1, y2 - y1, z2 - z1);
    }
};

using BBoxi = BBox<int>;
using BBoxf = BBox<float>;

static_assert(sizeof(BBoxi) == 24);
static_assert(sizeof(BBoxf) == 24);
