#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "Engine/Pid.h"

#include "Utility/Memory/Blob.h"

// TODO(pskelton): Overlays in MM7/ MM8 are blank so most of this isnt used. MM6 does have overlays - investigate if needed

struct ActiveOverlay {
    ActiveOverlay();
    void Reset();

    int16_t indexToOverlayList;
    int16_t spriteFrameTime;
    int16_t animLength;
    int16_t screenSpaceX;
    int16_t screenSpaceY;
    Pid pid;
    int16_t projSize;
    int fpDamageMod;
};

struct ActiveOverlayList {
    void Reset();
    int _4418B6(int uOverlayID, Pid pid, int animLength, int fpDamageMod, int16_t projSize);
    void DrawTurnBasedIcon();

    std::array<ActiveOverlay, 50> pOverlays;
};

struct OverlayDesc {
    uint16_t uOverlayID = 0;
    uint16_t uOverlayType = 0;
    uint16_t uSpriteFramesetID = 0;
    int16_t spriteFramesetGroup = 0;
};

struct OverlayList {
    bool FromFileTxt(const char *Args);
    void InitializeSprites();

    std::vector<OverlayDesc> pOverlays;
};

extern struct ActiveOverlayList *pActiveOverlayList;
extern struct OverlayList *pOverlayList;
