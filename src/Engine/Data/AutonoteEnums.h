#pragma once

#include <cstdint>

enum class AutonoteType : uint32_t {
    AUTONOTE_POTION_RECIPE = 0,
    AUTONOTE_STAT_HINT = 1,
    AUTONOTE_OBELISK = 2,
    AUTONOTE_SEER = 3,
    AUTONOTE_MISC = 4,
    AUTONOTE_TEACHER = 5,
};
using enum AutonoteType;
