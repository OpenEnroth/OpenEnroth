#pragma once

#include <type_traits>

#include "Color.h"

struct Colorf {
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    constexpr Colorf() = default;

    template<class R, class G, class B, class A = float>
        requires std::is_floating_point_v<R> && std::is_floating_point_v<G> && std::is_floating_point_v<B> && std::is_floating_point_v<A>
    constexpr Colorf(R r, G g, B b, A a = 1.0f): r(r), g(g), b(b), a(a) {}
};
static_assert(sizeof(Colorf) == 16);
static_assert(alignof(Colorf) == 4);


constexpr inline Colorf Color::toColorf() const {
    Colorf result;
    result.r = r / 255.0f;
    result.g = g / 255.0f;
    result.b = b / 255.0f;
    result.a = a / 255.0f;
    return result;
}
