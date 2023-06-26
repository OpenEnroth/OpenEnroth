#pragma once

#include <cstdint>

#include "Utility/Flags.h"

enum class ChestFlag : uint16_t {
    CHEST_TRAPPED = 0x1,
    CHEST_ITEMS_PLACED = 0x2,
    CHEST_OPENED = 0x4,
};
using enum ChestFlag;
MM_DECLARE_FLAGS(ChestFlags, ChestFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(ChestFlags)
