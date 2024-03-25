#pragma once

#include <vector>
#include <cstdint>

#include "Library/Geometry/Vec.h"

#include "Utility/Flags.h"

enum class LevelDecorationFlag : uint16_t {
    LEVEL_DECORATION_TRIGGERED_BY_TOUCH = 0x01,
    LEVEL_DECORATION_TRIGGERED_BY_MONSTER = 0x02,
    LEVEL_DECORATION_TRIGGERED_BY_OBJECT = 0x04,
    LEVEL_DECORATION_VISIBLE_ON_MAP = 0x08,
    LEVEL_DECORATION_CHEST = 0x10,
    LEVEL_DECORATION_INVISIBLE = 0x20,
    LEVEL_DECORATION_OBELISK_CHEST = 0x40,
};
using enum LevelDecorationFlag;
MM_DECLARE_FLAGS(LevelDecorationFlags, LevelDecorationFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(LevelDecorationFlags)


struct LevelDecoration {
    LevelDecoration();
    int GetGlobalEvent();
    bool IsInteractive();
    bool IsObeliskChestActive();

    uint16_t uDecorationDescID;
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
