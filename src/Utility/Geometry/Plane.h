#pragma once

#include "Utility/Math/FixPoint.h"

#include "Vec.h"

struct Planei {
    Vec3i vNormal; // Plane normal, unit vector stored as fixpoint.
    int dist = 0;  // D in A*x + B*y + C*z + D = 0 (basically D = -A*x_0 - B*y_0 - C*z_0), stored as fixpoint.

    /**
     * @param point                     Point to calculate distance to. Note that the point is NOT in fixpoint format.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    int SignedDistanceTo(const Vec3i &point) {
        return SignedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * Same as `SignedDistanceTo`, but returns the distance as a fixpoint number. To get the distance in original
     * coordinates, divide by 2^16.
     *
     * @see SignedDistanceTo(const Vec3i &)
     */
    int SignedDistanceToAsFixpoint(const Vec3i &point) {
        return SignedDistanceToAsFixpoint(point.x, point.y, point.z);
    }

    /**
     * @see SignedDistanceTo(const Vec3i &)
     */
    int SignedDistanceTo(const Vec3s &point) {
        return SignedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * @see SignedDistanceTo(const Vec3i &)
     */
    int SignedDistanceTo(int x, int y, int z) {
        return SignedDistanceToAsFixpoint(x, y, z) >> 16;
    }

    /**
     * @see SignedDistanceToAsFixpoint(const Vec3i &)
     */
    int SignedDistanceToAsFixpoint(int x, int y, int z) {
        return this->dist + this->vNormal.x * x + this->vNormal.y * y + this->vNormal.z * z;
    }
};

static_assert(sizeof(Planei) == 16);


struct Planef {
    Vec3f vNormal;
    float dist = 0.0f;

    /**
     * @param point                     Point to calculate distance to.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    float SignedDistanceTo(const Vec3f &point) {
        return this->dist + this->vNormal.x * point.x + this->vNormal.y * point.y + this->vNormal.z * point.z;
    }
};

static_assert(sizeof(Planef) == 16);



/**
 * Helper structure for calculating Z-coordinate of a point on a plane given x and y, basically a storage for
 * coefficients in `z = ax + by + c` equation.
 *
 * Coefficients are stored in fixpoint format (16 fraction bits).
 */
struct PlaneZCalcll {
    int64_t a = 0;
    int64_t b = 0;
    int64_t c = 0;

    int32_t Calculate(int32_t x, int32_t y) const {
        return static_cast<int32_t>((a * x + b * y + c + 0x8000) >> 16);
    }

    void Init(const Planei &plane) {
        if (plane.vNormal.z == 0) {
            this->a = this->b = this->c = 0;
        } else {
            this->a = -fixpoint_div(plane.vNormal.x, plane.vNormal.z);
            this->b = -fixpoint_div(plane.vNormal.y, plane.vNormal.z);
            this->c = -fixpoint_div(plane.dist, plane.vNormal.z);
        }
    }
};
