#pragma once

#include <cstdint>
#include <cmath>
#include <type_traits>
#include <algorithm>

#include "OurMath.h"

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);

template<class From, class To>
struct vector_conversion_allowed : std::false_type {};

#define WOMM_ALLOW_VECTOR_CONVERSION(FROM, TO) \
template<> \
struct vector_conversion_allowed<FROM, TO> : std::true_type {};

WOMM_ALLOW_VECTOR_CONVERSION(int16_t, int32_t)
WOMM_ALLOW_VECTOR_CONVERSION(int16_t, int64_t)
WOMM_ALLOW_VECTOR_CONVERSION(int32_t, int64_t)

#pragma pack(push, 1)
template <class T>
struct Vec2 {
    T x = 0;
    T y = 0;

    Vec2() = default;

    Vec2(T a, T b) : x(a), y(b) {}
};
#pragma pack(pop)

using Vec2_int_ = Vec2<int32_t>;
using Vec2_float_ = Vec2<float>;

const float pi = static_cast<float>(M_PI);

#pragma pack(push, 1)
template <class T>
struct Vec3 {
    T x = 0;
    T y = 0;
    T z = 0;

    Vec3() = default;
    Vec3(const Vec3 &other) = default;

    template<class OtherT> requires vector_conversion_allowed<OtherT, T>::value
    Vec3(const Vec3<OtherT> &other) : x(other.x), y(other.y), z(other.z) {}

    Vec3(T a, T b, T c) : x(a), y(b), z(c) {}

    static void Rotate(T sDepth, T sRotY, T sRotX, Vec3<T> v, T *outx, T *outy, T *outz) {
        float cosf_x = cos(pi * sRotX / 1024.0f);
        float sinf_x = sin(pi * sRotX / 1024.0f);
        float cosf_y = cos(pi * sRotY / 1024.0f);
        float sinf_y = sin(pi * sRotY / 1024.0f);

        *outx = v.x + (int)(sinf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outy = v.y + (int)(cosf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outz = v.z + (int)(sinf_x * (float)(sDepth /*>> 16*/));
    }

    void Normalize() requires std::is_floating_point_v<T> {
        T denom = static_cast<T>(1.0) / Length(*this);
        x *= denom;
        y *= denom;
        z *= denom;
    }

    friend Vec3<short> ToShortVector(const Vec3 &v) requires std::is_floating_point_v<T> {
        return Vec3<short>(std::round(v.x), std::round(v.y), std::round(v.z));
    }

    friend Vec3<int> ToIntVector(const Vec3 &v) requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(v.x), std::round(v.y), std::round(v.z));
    }

    friend Vec3<int> ToFixpointVector(const Vec3 &v) requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(v.x * 65536.0), std::round(v.y * 65536.0), std::round(v.z * 65536.0));
    }

    friend Vec3<float> ToFloatVector(const Vec3 &v) requires std::is_integral_v<T> {
        return Vec3<float>(v.x, v.y, v.z);
    }

    friend Vec3<float> ToFloatVectorFromFixpoint(const Vec3 &v) requires std::is_integral_v<T> {
        return Vec3<float>(v.x / 65536.0, v.y / 65536.0, v.z / 65536.0);
    }

    friend T LengthSqr(const Vec3 &v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    friend T Length(const Vec3 &v) {
        return std::sqrt(LengthSqr(v));
    }

    friend Vec3 operator+(const Vec3 &l, const Vec3 &r) {
        return Vec3(l.x + r.x, l.y + r.y, l.z + r.z);
    }

    friend Vec3 operator-(const Vec3 &l, const Vec3 &r) {
        return Vec3(l.x - r.x, l.y - r.y, l.z - r.z);
    }

    friend Vec3 operator/(const Vec3 &l, T r) {
        return Vec3(l.x / r, l.y / r, l.z / r);
    }

    friend Vec3 operator*(const Vec3 &l, T r) {
        return Vec3(l.x * r, l.y * r, l.z * r);
    }

    friend Vec3 operator*(T l, const Vec3 &r) {
        return r * l;
    }

    Vec3 &operator+=(const Vec3 &v) {
        *this = *this + v;
        return *this;
    }

    friend Vec3 Cross(const Vec3 &l, const Vec3 &r) {
        return Vec3(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x);
    }

    friend T Dot(const Vec3 &l, const Vec3 &r) {
        return l.x * r.x + l.y * r.y + l.z * r.z;
    }
};
#pragma pack(pop)

using Vec3_short_ = Vec3<int16_t>;
using Vec3_int_ = Vec3<int32_t>;
using Vec3_int64_ = Vec3<int64_t>;
using Vec3_float_ = Vec3<float>;

#pragma pack(push, 1)
struct Vec4_int_ {
    int x = 0;
    int y = 0;
    int z = 0;
    int w = 0;
};
#pragma pack(pop)

/*   82 */
#pragma pack(push, 1)
struct Plane_int_ {
    Vec3_int_ vNormal; // Plane normal, unit vector stored as fixpoint.
    int dist = 0;      // D in A*x + B*y + C*z + D = 0 (basically D = -A*x_0 - B*y_0 - C*z_0), stored as fixpoint.

    /**
     * @param point                     Point to calculate distance to. Note that the point is NOT in fixpoint format.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    int SignedDistanceTo(const Vec3_int_ &point) {
        return SignedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * Same as `SignedDistanceTo`, but returns the distance as a fixpoint number. To get the distance in original
     * coordinates, divide by 2^16.
     *
     * @see SignedDistanceTo(const Vec3_int_ &)
     */
    int SignedDistanceToAsFixpoint(const Vec3_int_ &point) {
        return SignedDistanceToAsFixpoint(point.x, point.y, point.z);
    }

    /**
     * @see SignedDistanceTo(const Vec3_int_ &)
     */
    int SignedDistanceTo(const Vec3_short_ &point) {
        return SignedDistanceTo(point.x, point.y, point.z);
    }

    /**
     * @see SignedDistanceTo(const Vec3_int_ &)
     */
    int SignedDistanceTo(int x, int y, int z) {
        return SignedDistanceToAsFixpoint(x, y, z) >> 16;
    }

    /**
     * @see SignedDistanceToAsFixpoint(const Vec3_int_ &)
     */
    int SignedDistanceToAsFixpoint(int x, int y, int z) {
        return this->dist + this->vNormal.x * x + this->vNormal.y * y + this->vNormal.z * z;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
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
};
#pragma pack(pop)

using BBox_int_ = BBox<int>;
using BBox_short_ = BBox<short>;
using BBox_float_ = BBox<float>;

#pragma pack(push, 1)
struct Plane_float_ {
    Vec3_float_ vNormal;
    float dist = 0.0f;

    /**
     * @param point                     Point to calculate distance to.
     * @return                          Signed distance to the provided point from this plane. Positive value
     *                                  means that `point` is in the half-space that the normal is pointing to,
     *                                  and this usually is "outside" the model that the face belongs to.
     */
    float SignedDistanceTo(const Vec3_float_ &point) {
        return this->dist + this->vNormal.x * point.x + this->vNormal.y * point.y + this->vNormal.z * point.z;
    }
};
#pragma pack(pop)

/**
 * Helper structure for calculating Z-coordinate of a point on a plane given x and y, basically a storage for
 * coefficients in `z = ax + by + c` equation.
 *
 * Coefficients are stored in fixpoint format (16 fraction bits).
 */
struct PlaneZCalc_int64_ {
    int64_t a = 0;
    int64_t b = 0;
    int64_t c = 0;

    int32_t Calculate(int32_t x, int32_t y) const {
        return static_cast<int32_t>((a * x + b * y + c + 0x8000) >> 16);
    }

    void Init(const Plane_int_ &plane) {
        if (plane.vNormal.z == 0) {
            this->a = this->b = this->c = 0;
        } else {
            this->a = -fixpoint_div(plane.vNormal.x, plane.vNormal.z);
            this->b = -fixpoint_div(plane.vNormal.y, plane.vNormal.z);
            this->c = -fixpoint_div(plane.dist, plane.vNormal.z);
        }
    }
};
