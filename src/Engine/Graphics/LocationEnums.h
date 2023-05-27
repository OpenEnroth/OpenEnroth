#pragma once

// TODO(captainurist): enum?
#define DAY_ATTRIB_FOG 1

enum class LEVEL_TYPE {
    LEVEL_NULL = 0,
    LEVEL_INDOOR = 0x1,
    LEVEL_OUTDOOR = 0x2,
};
using enum LEVEL_TYPE;

enum {
    MODEL_INDOOR = -1
};
