#pragma once

#include <cstdint>
#include <cmath>
#include <type_traits>

#include "OurMath.h"

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);

template<class From, class To>
struct vector_conversion_allowed : std::false_type {};

// Allow widening vector conversions.
template<>
struct vector_conversion_allowed<int16_t, int32_t> : std::true_type {};
template<>
struct vector_conversion_allowed<int16_t, int64_t> : std::true_type {};
template<>
struct vector_conversion_allowed<int32_t, int64_t> : std::true_type {};

#pragma pack(push, 1)
template <class T>
struct Vec2 {
    T x;
    T y;

    explicit Vec2(T a = 0, T b = 0) : x(a), y(b) {}
};
#pragma pack(pop)

using Vec2_int_ = Vec2<int32_t>;
using Vec2_float_ = Vec2<float>;

const float pi = static_cast<float>(M_PI);

#pragma pack(push, 1)
template <class T>
struct Vec3 {
    T x;
    T y;
    T z;

    explicit Vec3(T a = 0, T b = 0, T c = 0) : x(a), y(b), z(c) {}

    // TODO: rewrite with requires clause when we have C++20
    template<class OtherT, class = std::enable_if_t<vector_conversion_allowed<OtherT, T>::value>>
    Vec3(const Vec3<OtherT> &other) : x(other.x), y(other.y), z(other.z) {}

    template <class U>
    inline uint32_t GetDistanceTo(Vec3<U> &o) {
        return int_get_vector_length(abs(this->x - o.x), abs(this->y - o.y),
                                     abs(this->z - o.z));
    }

    static void Rotate(T sDepth, T sRotY, T sRotX, Vec3<T> v, T *outx, T *outy, T *outz) {
        float cosf_x = cos(pi * sRotX / 1024.0f);
        float sinf_x = sin(pi * sRotX / 1024.0f);
        float cosf_y = cos(pi * sRotY / 1024.0f);
        float sinf_y = sin(pi * sRotY / 1024.0f);

        *outx = v.x + (int)(sinf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outy = v.y + (int)(cosf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outz = v.z + (int)(sinf_x * (float)(sDepth /*>> 16*/));
    }

    static void Normalize(T *x, T *y, T *z) {
        extern int integer_sqrt(int val);
        int denom = *y * *y + *z * *z + *x * *x;
        int mult = 65536 / (integer_sqrt(denom) | 1);
        *x *= mult;
        *y *= mult;
        *z *= mult;
    }

    void Normalize_float() {
        double x = this->x;
        double y = this->y;
        double z = this->z;
        double s = sqrt(x * x + y * y + z * z);

        this->x = bankersRounding(x / s);
        this->y = bankersRounding(y / s);
        this->z = bankersRounding(z / s);
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

#pragma pack(push, 1)
struct Vec3_float_ {
    void Normalize();

    static Vec3_float_ *Cross(Vec3_float_ *v1, Vec3_float_ *pOut, float x,
                              float y, float z) {
        pOut->x = z * v1->y - y * v1->z;
        pOut->y = x * v1->z - z * v1->x;
        pOut->z = y * v1->x - x * v1->y;
        return pOut;
    }

    inline static float NegDot(Vec3_float_ *a1, Vec3_float_ *a2, float *a3) {
        return *a3 = -(a1->z * a2->z + a1->y * a2->y + a1->x * a2->x);
    }

    float x;
    float y;
    float z;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Vec4_int_ {
    int x;
    int y;
    int z;
    int w;
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
struct BBox_short_ {
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
    int16_t z1;
    int16_t z2;

    bool ContainsXY(int x, int y) const {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }

    bool Contains(const Vec3_short_ &pos) const {
        return x1 <= pos.x && pos.x <= x2 && y1 <= pos.y && pos.y <= y2 && z1 <= pos.z && pos.z <= z2;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BBox_int_ {
    int x1;
    int x2;
    int y1;
    int y2;
    int z1;
    int z2;

    bool Intersects(const BBox_short_ &other) const {
        return
            this->x1 <= other.x2 && this->x2 >= other.x1 &&
            this->y1 <= other.y2 && this->y2 >= other.y1 &&
            this->z1 <= other.z2 && this->z2 >= other.z1;
    }

    bool Intersects(const BBox_int_ &other) const {
        return
            this->x1 <= other.x2 && this->x2 >= other.x1 &&
            this->y1 <= other.y2 && this->y2 >= other.y1 &&
            this->z1 <= other.z2 && this->z2 >= other.z1;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Plane_float_ {
    struct Vec3_float_ vNormal;
    float dist;
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
