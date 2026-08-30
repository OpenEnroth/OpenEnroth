#pragma once

#include <vector>

#include "Engine/Data/PortraitFrameData.h"

#include "Core/Time/Duration.h"

struct PortraitFrameTable {
    int animationId(PortraitId portrait);
    Duration animationDuration(PortraitId portrait);
    int animationFrameIndex(int animationId, Duration frameTime);

    std::vector<PortraitFrameData> pFrames;
};

extern PortraitFrameTable *pPortraitFrameTable;  // idb
