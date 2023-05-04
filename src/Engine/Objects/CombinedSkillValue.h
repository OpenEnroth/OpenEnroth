#pragma once
#include "Engine/Objects/PlayerEnums.h"
#include "Utility/Segment.h"

/* Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
 * So max possible stored skill level is 63.
 */
typedef uint16_t PLAYER_SKILL;
typedef uint8_t PLAYER_SKILL_LEVEL;

enum class PLAYER_SKILL_MASTERY : int32_t {  // TODO: type could be changed to something else when SpriteObject_MM7 implemented in
                                             // LegacyImages
    PLAYER_SKILL_MASTERY_NONE = 0,
    PLAYER_SKILL_MASTERY_NOVICE = 1,
    PLAYER_SKILL_MASTERY_EXPERT = 2,
    PLAYER_SKILL_MASTERY_MASTER = 3,
    PLAYER_SKILL_MASTERY_GRANDMASTER = 4,

    PLAYER_SKILL_MASTERY_FIRST = PLAYER_SKILL_MASTERY_NOVICE,
    PLAYER_SKILL_MASTERY_LAST = PLAYER_SKILL_MASTERY_GRANDMASTER
};
using enum PLAYER_SKILL_MASTERY;

inline Segment<PLAYER_SKILL_MASTERY> SkillMasteries() {
    return Segment(PLAYER_SKILL_MASTERY_FIRST, PLAYER_SKILL_MASTERY_LAST);
}

inline PLAYER_SKILL_LEVEL GetSkillLevel(PLAYER_SKILL skill_value) {
    return skill_value & 0x3F;
}

/**
 * @offset 0x00458244.
 */
inline PLAYER_SKILL_MASTERY GetSkillMastery(PLAYER_SKILL skill_value) {
    // PLAYER_SKILL_MASTERY_NONE equal PLAYER_SKILL_MASTERY_NOVICE with skill level 0.
    // if (skill_value == 0)
    //    return PLAYER_SKILL_MASTERY_NONE;

    switch (skill_value & 0x1C0) {
        case 0x100:
            return PLAYER_SKILL_MASTERY_GRANDMASTER;
        case 0x80:
            return PLAYER_SKILL_MASTERY_MASTER;
        case 0x40:
            return PLAYER_SKILL_MASTERY_EXPERT;
        case 0x00:
            return PLAYER_SKILL_MASTERY_NOVICE;
    }

    assert(false);  // should not get here
    return PLAYER_SKILL_MASTERY_NONE;
}

inline void SetSkillLevel(PLAYER_SKILL *skill_value, PLAYER_SKILL_LEVEL level) {
    *skill_value = (*skill_value & ~0x3F) | (level & 0x3F);
}

inline void SetSkillMastery(PLAYER_SKILL *skill_value, PLAYER_SKILL_MASTERY mastery) {
    assert(mastery <= PLAYER_SKILL_MASTERY_GRANDMASTER);

    *skill_value &= 0x3F;

    switch (mastery) {
        case (PLAYER_SKILL_MASTERY_EXPERT):
            *skill_value |= 0x40;
            break;
        case (PLAYER_SKILL_MASTERY_MASTER):
            *skill_value |= 0x80;
            break;
        case (PLAYER_SKILL_MASTERY_GRANDMASTER):
            *skill_value |= 0x100;
            break;
        default:
            return;
    }
}

/**
 * Construct player skill value using skill mastery and skill level
 */
inline PLAYER_SKILL ConstructSkillValue(PLAYER_SKILL_MASTERY mastery, PLAYER_SKILL_LEVEL level) {
    PLAYER_SKILL skill = 0;

    SetSkillMastery(&skill, mastery);
    SetSkillLevel(&skill, level);

    return skill;
}

// Simple POD-like class for storing full skill value (level and mastery)
class CombinedSkillValue {
    int _level;
    PLAYER_SKILL_MASTERY _mastery;

 public:
    CombinedSkillValue();
    CombinedSkillValue(int level, PLAYER_SKILL_MASTERY mastery);
    explicit CombinedSkillValue(int joinedValue);

    // joins level and mastery into one integer
    int join() const;

    int level() const;
    CombinedSkillValue &setLevel(int level);

    PLAYER_SKILL_MASTERY mastery() const;
    CombinedSkillValue &setMastery(PLAYER_SKILL_MASTERY mastery);

    static bool isLevelValid(int level);
    static bool isMasteryValid(PLAYER_SKILL_MASTERY mastery);
};
