#pragma once

#include "Utility/Flags.h"

// note the values are important because they are read from the config
enum class ChestOverflowMode {
    // Vanilla behaviour where items that don't fit are lost
    VANILLA = 0,
    // OE will try to place items that didn't fit every time the chest is opened again.
    OE_RETAIN_ON_NEXT_OPEN = 1,
    // OE will try to place items that didn't fit every time an item is picked up from the chest
    OE_LIVE_REPLENISH = 2,
};
using enum ChestOverflowMode;
MM_DECLARE_FLAGS(ChestOverflowModes, ChestOverflowMode)
MM_DECLARE_OPERATORS_FOR_FLAGS(ChestOverflowModes)
