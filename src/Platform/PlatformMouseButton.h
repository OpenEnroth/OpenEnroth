#pragma once

#include "Utility/Flags.h"

enum class PlatformMouseButton {
    None    = 0,
    Left    = 0x1,
    Middle  = 0x2,
    Right   = 0x4
};

MM_DECLARE_FLAGS(PlatformMouseButtons, PlatformMouseButton)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformMouseButtons)
