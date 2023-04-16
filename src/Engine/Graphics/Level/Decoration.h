#pragma once

#include <array>
#include <vector>
#include <cstdint>

#include "Utility/Geometry/Vec.h"
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


/*   74 */
#pragma pack(push, 1)
// TODO(captainurist): need to introduce LevelDecoration_MM7
struct LevelDecoration {
    LevelDecoration();
    int GetGlobalEvent();
    bool IsInteractive();
    bool IsObeliskChestActive();

    uint16_t uDecorationDescID;
    LevelDecorationFlags uFlags;
    Vec3i vPosition;
    int32_t _yawAngle;
    uint16_t uCog;
    uint16_t uEventID;
    uint16_t uTriggerRange;
    int16_t field_1A;
    int16_t _idx_in_stru123;
    int16_t field_1E;
};

extern std::vector<LevelDecoration> pLevelDecorations;
extern LevelDecoration *activeLevelDecoration;  // 5C3420
#pragma pack(pop)
