#pragma once

#include <vector>

#include "Engine/Data/PortraitFrameData.h"
#include "Engine/Time/Duration.h"

struct PortraitFrameTable {
    unsigned int GetFrameIdByPortrait(CharacterPortrait portrait);
    Duration GetDurationByPortrait(CharacterPortrait portrait);
    PortraitFrameData *GetFrameBy_x(int uFramesetID, Duration gameTime);
    PortraitFrameData *GetFrameBy_y(int *a2, Duration *a3, Duration a4);

    std::vector<PortraitFrameData> pFrames;
};

extern PortraitFrameTable *pPortraitFrameTable;  // idb
