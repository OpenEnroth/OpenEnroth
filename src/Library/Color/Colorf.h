#pragma once

#include <type_traits>
#include "HsvColorf.h"

struct Color;

struct Colorf {
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    constexpr Colorf() = default;

    template<class R, class G, class B, class A = float>
        requires std::is_floating_point_v<R> && std::is_floating_point_v<G> && std::is_floating_point_v<B> && std::is_floating_point_v<A>
    constexpr Colorf(R r, G g, B b, A a = 1.0f): r(r), g(g), b(b), a(a) {}

    [[nodiscard]] constexpr Color toColor() const;

    /**
     * @offset 0x0048A7AA
     *
     * @return                          This color, converted to HSV.
     */
    [[nodiscard]] HsvColorf toHsv() const;
};
static_assert(sizeof(Colorf) == 16);
static_assert(alignof(Colorf) == 4);


