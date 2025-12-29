#pragma once

template<class T>
struct Size {
    T w = 0;
    T h = 0;

    constexpr Size() = default;
    constexpr Size(T w, T h): w(w), h(h) {}

    [[nodiscard]] constexpr friend Size operator/(Size l, T r) {
        return {l.w / r, l.h / r};
    }

    [[nodiscard]] constexpr friend bool operator==(const Size &l, const Size &r) = default;
};

using Sizei = Size<int>;
using Sizef = Size<float>;
