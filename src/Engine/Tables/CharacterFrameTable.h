#pragma once

#include <vector>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Time/Duration.h"

struct PlayerFrame {
    CharacterPortrait portrait;
    uint16_t uTextureID;
    Duration uAnimTime;
    Duration uAnimLength;
    int16_t uFlags;
};

struct PlayerFrameTable {
    unsigned int GetFrameIdByPortrait(CharacterPortrait portrait);
    Duration GetDurationByPortrait(CharacterPortrait portrait);
    PlayerFrame *GetFrameBy_x(int uFramesetID, Duration gameTime);
    PlayerFrame *GetFrameBy_y(int *a2, Duration *a3, Duration a4);

    std::vector<PlayerFrame> pFrames;
};

extern PlayerFrameTable *pPlayerFrameTable;  // idb
