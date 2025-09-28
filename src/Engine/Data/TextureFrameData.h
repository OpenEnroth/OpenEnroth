#pragma once

#include <string>

#include "Engine/Time/Duration.h"

#include "FrameEnums.h"

struct TextureFrameData {
    std::string textureName; // Texture name, to be looked up in bitmaps.lod.
    Duration frameLength; // Duration of this frame.
    Duration animationLength; // Total animation duration. Set only for the first frame in a sequence.
    FrameFlags flags;
};
