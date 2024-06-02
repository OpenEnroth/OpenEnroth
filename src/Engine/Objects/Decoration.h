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

    DecorationId uDecorationDescID;
    LevelDecorationFlags uFlags;
    Vec3f vPosition;
    int32_t _yawAngle; // Only used for party spawn points, see `MapStartPoint`.
    uint16_t uCog;
    uint16_t uEventID;
    uint16_t uTriggerRange;
    int16_t eventVarId;
};

extern std::vector<LevelDecoration> pLevelDecorations;
extern std::vector<int> decorationsWithSound;
extern LevelDecoration *activeLevelDecoration;  // 5C3420
