#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

template<class From, class To>
struct vector_conversion_allowed : std::false_type {};

#define MM_ALLOW_VECTOR_CONVERSION(FROM, TO) \
template<> \
struct vector_conversion_allowed<FROM, TO> : std::true_type {};

MM_ALLOW_VECTOR_CONVERSION(int16_t, int32_t)
MM_ALLOW_VECTOR_CONVERSION(int16_t, int64_t)
MM_ALLOW_VECTOR_CONVERSION(int32_t, int64_t)

template <class T>
struct Vec2 {
    T x = 0;
    T y = 0;

    Vec2() = default;
    Vec2(T a, T b) : x(a), y(b) {}

    friend Vec2 operator+(const Vec2 &l, const Vec2 &r) {
        return Vec2(l.x + r.x, l.y + r.y);
    }

    friend Vec2 operator-(const Vec2 &l, const Vec2 &r) {
        return Vec2(l.x - r.x, l.y - r.y);
    }

    friend Vec2 operator/(const Vec2 &l, T r) {
        return Vec2(l.x / r, l.y / r);
    }

    friend Vec2 operator*(const Vec2 &l, T r) {
        return Vec2(l.x * r, l.y * r);
    }

    friend Vec2 operator*(T l, const Vec2 &r) {
        return r * l;
    }

    Vec2 &operator+=(const Vec2 &v) {
        *this = *this + v;
        return *this;
    }

    Vec2 &operator-=(const Vec2 &v) {
        *this = *this - v;
        return *this;
    }
};

using Vec2i = Vec2<int32_t>;
using Vec2f = Vec2<float>;

static_assert(sizeof(Vec2i) == 8);
static_assert(sizeof(Vec2f) == 8);


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
        float cosf_x = cos(M_PI * sRotX / 1024.0f);
        float sinf_x = sin(M_PI * sRotX / 1024.0f);
        float cosf_y = cos(M_PI * sRotY / 1024.0f);
        float sinf_y = sin(M_PI * sRotY / 1024.0f);

        *outx = v.x + (int)(sinf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outy = v.y + (int)(cosf_y * cosf_x * (float)(sDepth /*>> 16*/));
        *outz = v.z + (int)(sinf_x * (float)(sDepth /*>> 16*/));
    }

    void Normalize() requires std::is_floating_point_v<T> {
        T denom = static_cast<T>(1.0) / this->Length();
        x *= denom;
        y *= denom;
        z *= denom;
    }

    Vec3<T> Abs() const requires std::is_integral_v<T> {
        return Vec3<T>(abs(x), abs(y), abs(z));
    }

    Vec3<short> ToShort() const requires std::is_floating_point_v<T> {
        return Vec3<short>(std::round(x), std::round(y), std::round(z));
    }

    Vec3<int> ToInt() const requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(x), std::round(y), std::round(z));
    }

    Vec3<int> ToFixpoint() const requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(x * 65536.0), std::round(y * 65536.0), std::round(z * 65536.0));
    }

    Vec3<float> ToFloat() const requires std::is_integral_v<T> {
        return Vec3<float>(x, y, z);
    }

    Vec3<float> ToFloatFromFixpoint() const requires std::is_integral_v<T> {
        return Vec3<float>(x / 65536.0, y / 65536.0, z / 65536.0);
    }

    auto LengthSqr() const {
        // Note that auto return type is important because this way Vec3s::LengthSqr returns int.
        return x * x + y * y + z * z;
    }

    T Length() const {
        return std::sqrt(LengthSqr());
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

    Vec3 &operator-=(const Vec3 &v) {
        *this = *this - v;
        return *this;
    }

    friend Vec3 Cross(const Vec3 &l, const Vec3 &r) {
        return Vec3(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x);
    }

    friend T Dot(const Vec3 &l, const Vec3 &r) {
        return l.x * r.x + l.y * r.y + l.z * r.z;
    }
};

using Vec3s = Vec3<int16_t>;
using Vec3i = Vec3<int32_t>;
using Vec3f = Vec3<float>;

static_assert(sizeof(Vec3s) == 6);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec3f) == 12);
