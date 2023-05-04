#pragma once

#include <vector>

#include "Engine/Objects/Player.h"

#include "Utility/Memory/Blob.h"

struct PlayerFrame {
    CHARACTER_EXPRESSION_ID expression;
    uint16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};

struct PlayerFrameTable {
    unsigned int GetFrameIdByExpression(CHARACTER_EXPRESSION_ID expression);
    PlayerFrame *GetFrameBy_x(unsigned int uFramesetID, unsigned int uFrameID);
    PlayerFrame *GetFrameBy_y(int *a2, int *a3, int a4);
    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
    int FromFileTxt(const char *Args);

    std::vector<PlayerFrame> pFrames;
};

extern struct PlayerFrameTable *pPlayerFrameTable;  // idb
