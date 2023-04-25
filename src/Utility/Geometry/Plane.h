#pragma once

#include "Utility/Math/FixPoint.h"
#include "Utility/Math/Float.h"

#include "Vec.h"

struct Planei {
    Vec3i normal; // Plane normal, unit vector stored as fixpoint.
    int dist = 0;  // D in A*x + B*y + C*z + D = 0 (basically D = -A*x_0 - B*y_0 - C*z_0), stored as fixpoint.

    /**
     * @param point                     Point to calculate distance to. Note that the point is NOT in fixpoint format.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    int signedDistanceTo(const Vec3i &point) {
        return signedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * Same as `signedDistanceTo`, but returns the distance as a fixpoint number. To get the distance in original
     * coordinates, divide by 2^16.
     *
     * @see signedDistanceTo(const Vec3i &)
     */
    int signedDistanceToAsFixpoint(const Vec3i &point) {
        return signedDistanceToAsFixpoint(point.x, point.y, point.z);
    }

    /**
     * @see signedDistanceTo(const Vec3i &)
     */
    int signedDistanceTo(const Vec3s &point) {
        return signedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * @see signedDistanceTo(const Vec3i &)
     */
    int signedDistanceTo(int x, int y, int z) {
        return signedDistanceToAsFixpoint(x, y, z) >> 16;
    }

    /**
     * @see signedDistanceToAsFixpoint(const Vec3i &)
     */
    int signedDistanceToAsFixpoint(int x, int y, int z) {
        return this->dist + this->normal.x * x + this->normal.y * y + this->normal.z * z;
    }
};

static_assert(sizeof(Planei) == 16);


struct Planef {
    Vec3f normal;
    float dist = 0.0f;

    /**
     * @param point                     Point to calculate distance to.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    float signedDistanceTo(const Vec3f &point) {
        return this->dist + this->normal.x * point.x + this->normal.y * point.y + this->normal.z * point.z;
    }
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

    void init(const Planei &plane) {
        if (plane.normal.z == 0) {
            a = b = c = 0;
        } else {
            a = -fixpoint_to_float(plane.normal.x) / fixpoint_to_float(plane.normal.z);
            b = -fixpoint_to_float(plane.normal.y) / fixpoint_to_float(plane.normal.z);
            c = -fixpoint_to_float(plane.dist) / fixpoint_to_float(plane.normal.z);
        }
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
