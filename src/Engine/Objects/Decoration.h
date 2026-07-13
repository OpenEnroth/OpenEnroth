#pragma once

#include <vector>
#include <cstdint>

#include "Library/Geometry/Vec.h"

#include "DecorationEnums.h"

struct LevelDecoration {
    LevelDecoration();
    int GetGlobalEvent();
    bool IsInteractive();
    bool IsObeliskChestActive();

    DecorationId uDecorationDescID = DECORATION_NULL;
    LevelDecorationFlags uFlags;
    Vec3f vPosition;
    int32_t _yawAngle = 0; // Only used for party spawn points, see `MapStartPoint`.
    uint16_t uCog = 0;
    uint16_t uEventID = 0;
    uint16_t uTriggerRange = 0;
    int16_t eventVarId = 0;
};

extern std::vector<LevelDecoration> pLevelDecorations;
extern std::vector<int> decorationsWithSound;
extern LevelDecoration *activeLevelDecoration;  // 5C3420
