#pragma once

#include <stdint.h>
#include <vector>
#include "Engine/Objects/CharacterEnums.h"
#include "Utility/Memory/Blob.h"

enum CharacterExpressionID : uint16_t;

struct PlayerFrame {
    CharacterExpressionID expression;
    uint16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};

struct PlayerFrameTable {
    unsigned int GetFrameIdByExpression(CharacterExpressionID expression);
    int GetDurationByExpression(CharacterExpressionID expression);
    PlayerFrame *GetFrameBy_x(int uFramesetID, int gameTime);
    PlayerFrame *GetFrameBy_y(int *a2, int *a3, int a4);
    int FromFileTxt(const char *Args);

    std::vector<PlayerFrame> pFrames;
};

extern struct PlayerFrameTable *pPlayerFrameTable;  // idb
