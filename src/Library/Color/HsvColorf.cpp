#include "HsvColorf.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Color.h"
#include "Colorf.h"

Colorf HsvColorf::toColorf() const {
    assert(h >= 0.0f && h <= 360.0f);
    assert(s >= 0.0f && s <= 1.0f);
    assert(v >= 0.0f && v <= 1.0f);
    assert(a >= 0.0f && a <= 1.0f);

    if (s == 0.0)
        return Colorf(v, v, v, a);

    float h2 = h;
    if (h2 == 360.0f)
        h2 = 0.0f;

    float hh = h2 / 60; // to sixth segments
    int segment = static_cast<int>(hh);
    float fraction = hh - segment;
    float p = (1.0f - s) * v;
    float q = (1.0f - fraction * s) * v;
    float t = (1.0f - (1.0f - fraction) * s) * v;

    switch (segment) {
    case 0: return Colorf(v, t, p, a);
    case 1: return Colorf(q, v, p, a);
    case 2: return Colorf(p, v, t, a);
    case 3: return Colorf(p, q, v, a);
    case 4: return Colorf(t, p, v, a);
    case 5:
    default: return Colorf(v, p, q, a);
    }
}

[[nodiscard]] Color HsvColorf::toColor() const {
    return toColorf().toColor();
}

[[nodiscard]] HsvColorf HsvColorf::adjusted(float dh, float xs, float xv) const {
    assert(dh >= -180.0f && dh <= 180.0f);
    assert(xs >= 0.0f);
    assert(xv >= 0.0f);

    HsvColorf result;
    result.h = std::fmod(h + dh + 360.0f, 360.0f);
    result.s = std::clamp(s * xs, 0.0f, 1.0f);
    result.v = std::clamp(v * xv, 0.0f, 1.0f);
    result.a = a;
    return result;
}
