#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <type_traits>

#include "Utility/Math/TrigLut.h"

template <class T>
struct Vec2 {
    T x = 0;
    T y = 0;

    constexpr Vec2() = default;
    constexpr Vec2(T a, T b) : x(a), y(b) {}

    [[nodiscard]] Vec2<int> toInt() const requires std::is_floating_point_v<T> {
        return Vec2<int>(std::round(x), std::round(y));
    }

    [[nodiscard]] auto lengthSqr() const {
        // Note that auto return type is important because this way Vec2s::lengthSqr returns int.
        return x * x + y * y;
    }

    [[nodiscard]] T length() const {
        return std::sqrt(lengthSqr());
    }

    friend bool operator==(const Vec2 &, const Vec2 &) = default;

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

    friend Vec2 operator-(const Vec2 &v) {
        return Vec2(-v.x, -v.y);
    }

    Vec2 &operator+=(const Vec2 &v) {
        *this = *this + v;
        return *this;
    }

    Vec2 &operator-=(const Vec2 &v) {
        *this = *this - v;
        return *this;
    }

    friend T dot(const Vec2 &l, const Vec2 &r) {
        return l.x * r.x + l.y * r.y;
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

    constexpr Vec3() = default;
    constexpr Vec3(const Vec3 &other) = default;
    constexpr Vec3(T a, T b, T c) : x(a), y(b), z(c) {}

    static Vec3 fromPolar(T length, int yaw, int pitch) {
        float cosPitch = TrigLUT.cos(pitch);
        float sinPitch = TrigLUT.sin(pitch);
        float cosYaw = TrigLUT.cos(yaw);
        float sinYaw = TrigLUT.sin(yaw);
        return Vec3(cosYaw * cosPitch * length, sinYaw * cosPitch * length, sinPitch * length);
    }

    Vec2<T> xy() {
        return Vec2<T>(x, y);
    }

    void normalize() requires std::is_floating_point_v<T> {
        T length = this->length();
        T denom = 0;
        if (length > 0)
            denom = static_cast<T>(1.0) / length;
        x *= denom;
        y *= denom;
        z *= denom;
    }

    [[nodiscard]] Vec3<int> toInt() const requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(x), std::round(y), std::round(z));
    }

    // TODO(captainurist): We introduced this one because we couldn't easily retrace old traces.
    //                     Drop once we switch over to fp everywhere.
    [[nodiscard]] Vec3<int> toIntTrunc() const requires std::is_floating_point_v<T> {
        return Vec3<int>(std::trunc(x), std::trunc(y), std::trunc(z));
    }

    [[nodiscard]] Vec3<int> toFixpoint() const requires std::is_floating_point_v<T> {
        return Vec3<int>(std::round(x * 65536.0), std::round(y * 65536.0), std::round(z * 65536.0));
    }

    [[nodiscard]] Vec3<float> toFloat() const requires std::is_integral_v<T> {
        return Vec3<float>(x, y, z);
    }

    [[nodiscard]] Vec3<float> toFloatFromFixpoint() const requires std::is_integral_v<T> {
        return Vec3<float>(x / 65536.0, y / 65536.0, z / 65536.0);
    }

    [[nodiscard]] auto lengthSqr() const {
        // Note that auto return type is important because this way Vec3s::lengthSqr returns int.
        return x * x + y * y + z * z;
    }

    [[nodiscard]] T length() const {
        return std::sqrt(lengthSqr());
    }

    [[nodiscard]] T chebyshevLength() const { // L-inf norm.
        return std::max({std::abs(x), std::abs(y), std::abs(z)});
    }

    friend bool operator==(const Vec3 &l, const Vec3 &r) = default;

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

    friend Vec3 operator-(const Vec3 &v) {
        return Vec3(-v.x, -v.y, -v.z);
    }

    Vec3 &operator+=(const Vec3 &v) {
        *this = *this + v;
        return *this;
    }

    Vec3 &operator-=(const Vec3 &v) {
        *this = *this - v;
        return *this;
    }

    Vec3 &operator*=(T v) {
        *this = *this * v;
        return *this;
    }

    Vec3 &operator/=(T v) {
        *this = *this / v;
        return *this;
    }

    friend Vec3 cross(const Vec3 &l, const Vec3 &r) {
        return Vec3(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x);
    }

    friend T dot(const Vec3 &l, const Vec3 &r) {
        return l.x * r.x + l.y * r.y + l.z * r.z;
    }
};

using Vec3s = Vec3<int16_t>;
using Vec3i = Vec3<int32_t>;
using Vec3f = Vec3<float>;

static_assert(sizeof(Vec3s) == 6);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec3f) == 12);
