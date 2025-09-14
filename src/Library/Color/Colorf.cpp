#include "Colorf.h"

#include <cassert>
#include <algorithm>

#include "HsvColorf.h"

[[nodiscard]] HsvColorf Colorf::toHsvColorf() const {
    assert(r >= 0.0f && r <= 1.0f);
    assert(g >= 0.0f && g <= 1.0f);
    assert(b >= 0.0f && b <= 1.0f);
    assert(a >= 0.0f && a <= 1.0f);

    HsvColorf result;
    result.a = a;

    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float delta = max - min;

    // value
    result.v = max;

    // saturation
    if (max == 0.0) {  // r=g=b=0
        result.s = 0.0;
    } else {
        result.s = delta / max;
    }

    float hcalc = 0.0f;

    // hue
    if (max == min) {
        hcalc = 0.0f;
    } else if (max == r) {
        hcalc = (g - b) / delta; // yellow and mag
    } else if (max == g) {
        hcalc = (b - r) / delta + 2.0;    // cyan and yellow
    } else {
        hcalc = (r - g) / delta + 4.0;  // mag and cyan
    }

    result.h = hcalc * 60.0f;  // to degree
    if (result.h < 0.0f)
        result.h += 360.0f;

    return result;
}
