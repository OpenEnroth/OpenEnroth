#pragma once

#include <vector>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"

struct PlayerFrame {
    CharacterExpressionID expression;
    uint16_t uTextureID;
    Duration uAnimTime;
    Duration uAnimLength;
    int16_t uFlags;
};

struct PlayerFrameTable {
    unsigned int GetFrameIdByExpression(CharacterExpressionID expression);
    Duration GetDurationByExpression(CharacterExpressionID expression);
    PlayerFrame *GetFrameBy_x(int uFramesetID, Duration gameTime);
    PlayerFrame *GetFrameBy_y(int *a2, Duration *a3, Duration a4);

    std::vector<PlayerFrame> pFrames;
};

extern struct PlayerFrameTable *pPlayerFrameTable;  // idb
