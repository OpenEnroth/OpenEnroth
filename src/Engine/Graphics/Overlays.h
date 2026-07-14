#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "Engine/Pid.h"
#include "Engine/Time/Duration.h"

// TODO(pskelton): Overlays in MM7/ MM8 are blank so most of this isnt used. MM6 does have overlays - investigate if needed

struct ActiveOverlay {
    ActiveOverlay();
    void Reset();

    int16_t indexToOverlayList = 0;
    int16_t spriteFrameTime = 0;
    int16_t animLength = 0;
    int16_t screenSpaceX = 0;
    int16_t screenSpaceY = 0;
    Pid pid;
    int16_t projSize = 0;
    int fpDamageMod = 0;
};

struct ActiveOverlayList {
    void Reset();
    int _4418B6(int uOverlayID, Pid pid, Duration animLength, int fpDamageMod, int16_t projSize);

    std::array<ActiveOverlay, 50> pOverlays;
};

struct OverlayDesc {
    uint16_t uOverlayID = 0;
    uint16_t uOverlayType = 0;
    uint16_t uSpriteFramesetID = 0;
    int16_t spriteFramesetGroup = 0;
};

struct OverlayList {
    void InitializeSprites();

    std::vector<OverlayDesc> pOverlays;
};

extern ActiveOverlayList *pActiveOverlayList;
extern OverlayList *pOverlayList;
