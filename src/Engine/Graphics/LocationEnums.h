#pragma once

// TODO(captainurist): merge with MapEnums.h

// TODO(captainurist): enum?
#define DAY_ATTRIB_FOG 1

enum class LevelType {
    LEVEL_NULL = 0,
    LEVEL_INDOOR = 0x1,
    LEVEL_OUTDOOR = 0x2,
};
using enum LevelType;

enum {
    MODEL_INDOOR = -1
};
