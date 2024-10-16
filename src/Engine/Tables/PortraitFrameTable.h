#pragma once

#include <vector>

#include "Engine/Data/PortraitFrameData.h"
#include "Engine/Time/Duration.h"

struct PortraitFrameTable {
    int animationId(CharacterPortrait portrait);
    Duration animationDuration(CharacterPortrait portrait);
    int animationFrameIndex(int animationId, Duration frameTime);
    int talkFrameIndex(int *animationId, Duration *currentTime, Duration dt);

    std::vector<PortraitFrameData> pFrames;
};

extern PortraitFrameTable *pPortraitFrameTable;  // idb
