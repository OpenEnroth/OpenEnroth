#pragma once

#include <vector>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"

struct PlayerFrame {
    CharacterPortrait expression;
    uint16_t uTextureID;
    Duration uAnimTime;
    Duration uAnimLength;
    int16_t uFlags;
};

struct PlayerFrameTable {
    unsigned int GetFrameIdByExpression(CharacterPortrait expression);
    Duration GetDurationByExpression(CharacterPortrait expression);
    PlayerFrame *GetFrameBy_x(int uFramesetID, Duration gameTime);
    PlayerFrame *GetFrameBy_y(int *a2, Duration *a3, Duration a4);

    std::vector<PlayerFrame> pFrames;
};

extern PlayerFrameTable *pPlayerFrameTable;  // idb
