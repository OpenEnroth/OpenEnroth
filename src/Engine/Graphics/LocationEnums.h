#pragma once

// TODO(captainurist): enum?
#define DAY_ATTRIB_FOG 1

enum class LEVEL_TYPE {
    LEVEL_null = 0,
    LEVEL_Indoor = 0x1,
    LEVEL_Outdoor = 0x2,
};
using enum LEVEL_TYPE;

enum {
    MODEL_INDOOR = -1
};
