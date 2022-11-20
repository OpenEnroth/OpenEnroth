#pragma once

#include <cstdint>

#include "Utility/Flags.h"

enum class PlatformModifier : uint32_t {
    Shift  = 0x00000001,
    Ctrl   = 0x00000002,
    Alt    = 0x00000004,
    Meta   = 0x00000008,
    NumPad = 0x00000010,
};

MM_DECLARE_FLAGS(PlatformModifiers, PlatformModifier)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformModifiers)
