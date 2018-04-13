#pragma once

#include "Engine/Objects/Player.h"

/*   46 */
#pragma pack(push, 1)
struct PlayerFrame {
    CHARACTER_EXPRESSION_ID expression;
    unsigned __int16 uTextureID;
    __int16 uAnimTime;
    __int16 uAnimLength;
    __int16 uFlags;
};
#pragma pack(pop)

/*   47 */
#pragma pack(push, 1)
struct PlayerFrameTable {
    inline PlayerFrameTable() : uNumFrames(0), pFrames(nullptr) {}

    unsigned int GetFrameIdByExpression(CHARACTER_EXPRESSION_ID expression);
    PlayerFrame *GetFrameBy_x(unsigned int uFramesetID, unsigned int uFrameID);
    PlayerFrame *GetFrameBy_y(int *a2, int *a3, int a4);
    void ToFile();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    int FromFileTxt(const char *Args);

    unsigned int uNumFrames;
    struct PlayerFrame *pFrames;
};
#pragma pack(pop)

extern struct PlayerFrameTable *pPlayerFrameTable;  // idb
