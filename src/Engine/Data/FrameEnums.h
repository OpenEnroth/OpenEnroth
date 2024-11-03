#pragma once

#include "Utility/Flags.h"

enum class FrameFlag {
    FRAME_HAS_MORE = 0x1, // There are more frames belonging to the current animation after this frame.
    FRAME_FIRST = 0x4, // This frame is the first one in an animation.
};
using enum FrameFlag;
MM_DECLARE_FLAGS(FrameFlags, FrameFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(FrameFlags)
