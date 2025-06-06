#pragma once

#include "Utility/Math/Float.h"

#include "Vec.h"

struct Planef {
    Vec3f normal; // Plane normal, unit vector.
    float dist = 0.0f; // D in A*x + B*y + C*z + D = 0. Basically, D = -A*x_0 - B*y_0 - C*z_0.

    /**
     * @param point                     Point to calculate distance to.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    [[nodiscard]] float signedDistanceTo(const Vec3f &point) const {
        return this->dist + this->normal.x * point.x + this->normal.y * point.y + this->normal.z * point.z;
    }

    friend bool operator==(const Planef& l, const Planef& r) = default;
};
static_assert(sizeof(Planef) == 16);


/**
 * Helper structure for calculating Z-coordinate of a point on a plane given x and y, basically a storage for
 * coefficients in `z = ax + by + c` equation.
 */
struct PlaneZCalcf {
    float a = 0;
    float b = 0;
    float c = 0;

    [[nodiscard]] float calculate(float x, float y) const {
        return a * x + b * y + c;
    }

    void init(const Planef &plane) {
        if (fuzzyIsNull(plane.normal.z)) {
            a = b = c = 0;
        } else {
            a = -plane.normal.x / plane.normal.z;
            b = -plane.normal.y / plane.normal.z;
            c = -plane.dist / plane.normal.z;
        }
    }
};
