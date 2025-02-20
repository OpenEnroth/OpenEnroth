#pragma once

#include "Engine/Objects/CharacterEnums.h" // TODO(captainurist): Data -> Objects dependency, we don't want that.

#include "Engine/Time/Duration.h"

#include "FrameEnums.h"

struct PortraitFrameData {
    CharacterPortrait portrait = PORTRAIT_INVALID; // Set on the 1st frame only.
    int textureIndex = 0; // Index of the face texture to look up, actual file name in icons.lod will be smth like
                          // "pcXX-YY", where YY is this index.
    Duration frameLength; // Length of this frame in the animation.
    Duration animationLength; // Set on the 1st frame, total animation length.
    FrameFlags flags;
};
