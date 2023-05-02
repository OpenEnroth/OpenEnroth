#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "Utility/Memory/Blob.h"

struct ActiveOverlay {
    ActiveOverlay();
    void Reset();

    int16_t field_0;
    int16_t field_2;
    int16_t sprite_frame_time;  // int16_t field_4;
    int16_t field_6;
    int16_t screen_space_x;  // int16_t field_8;
    int16_t screen_space_y;  // int16_t field_A;
    int16_t field_C;
    int16_t field_E;
    int field_10;
};

struct ActiveOverlayList {
    void Reset();
    int _4418B6(int uOverlayID, int16_t a3, int a4, int a5, int16_t a6);
    void DrawTurnBasedIcon();

    std::array<ActiveOverlay, 50> pOverlays;
    int field_3E8 = 0;
};

struct OverlayDesc {
    uint16_t uOverlayID = 0;
    uint16_t uOverlayType = 0;
    uint16_t uSpriteFramesetID = 0;
    int16_t field_6 = 0;
};

struct OverlayList {
    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
    bool FromFileTxt(const char *Args);
    void InitializeSprites();

    std::vector<OverlayDesc> pOverlays;
};

extern struct ActiveOverlayList *pActiveOverlayList;  // idb
extern struct OverlayList *pOverlayList;
