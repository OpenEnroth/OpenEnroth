#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "Engine/Data/OverlayData.h"
#include "Engine/Pid.h"

#include "Core/Time/Duration.h"

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

// TODO(captainurist): move to Engine/Tables as OverlayTable.
struct OverlayList {
    void InitializeSprites();

    std::vector<OverlayData> pOverlays;
};

extern ActiveOverlayList *pActiveOverlayList;
extern OverlayList *pOverlayList;
