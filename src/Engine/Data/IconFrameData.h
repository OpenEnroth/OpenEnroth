#pragma once

#include <string>

#include "Engine/Time/Duration.h"

#include "FrameEnums.h"

struct IconFrameData {
    std::string textureName; // Texture name for this frame, to be looked up in icons.lod.
    FrameFlags flags;
    std::string animationName; // Animation name, set on the 1st frame.
    Duration animationLength; // Set on the 1st frame, total animation length.
    Duration frameLength; // Length of this frame in the animation.
};
