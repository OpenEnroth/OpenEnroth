#pragma once
#include <cstdint>
#include "Engine/Objects/CharacterEnums.h"
#include "Utility/Segment.h"

/* Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
 * So max possible stored skill level is 63.
 */
// TODO(pskelton): reduce or drop in favour of combined value
typedef uint16_t CHARACTER_SKILL;
typedef uint8_t CHARACTER_SKILL_LEVEL;

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterSkillMastery : int32_t {  // TODO: type could be changed to something else when SpriteObject_MM7 implemented in
                                             // LegacyImages
    CHARACTER_SKILL_MASTERY_NONE = 0,
    CHARACTER_SKILL_MASTERY_NOVICE = 1,
    CHARACTER_SKILL_MASTERY_EXPERT = 2,
    CHARACTER_SKILL_MASTERY_MASTER = 3,
    CHARACTER_SKILL_MASTERY_GRANDMASTER = 4,

    CHARACTER_SKILL_MASTERY_FIRST = CHARACTER_SKILL_MASTERY_NOVICE,
    CHARACTER_SKILL_MASTERY_LAST = CHARACTER_SKILL_MASTERY_GRANDMASTER
};
using enum CharacterSkillMastery;

inline Segment<CharacterSkillMastery> SkillMasteries() {
    return Segment(CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST);
}

inline CHARACTER_SKILL_LEVEL GetSkillLevel(const CHARACTER_SKILL skill_value) {
    return skill_value & 0x3F;
}

/**
 * @offset 0x00458244.
 */
inline CharacterSkillMastery GetSkillMastery(const CHARACTER_SKILL skill_value) {
    // CHARACTER_SKILL_MASTERY_NONE equal CHARACTER_SKILL_MASTERY_NOVICE with skill level 0.
    if (GetSkillLevel(skill_value) == 0)
        return CHARACTER_SKILL_MASTERY_NONE;

    switch (skill_value & 0x1C0) {
        case 0x100:
            return CHARACTER_SKILL_MASTERY_GRANDMASTER;
        case 0x80:
            return CHARACTER_SKILL_MASTERY_MASTER;
        case 0x40:
            return CHARACTER_SKILL_MASTERY_EXPERT;
        case 0x00:
            return CHARACTER_SKILL_MASTERY_NOVICE;
    }

    assert(false);  // should not get here
    return CHARACTER_SKILL_MASTERY_NONE;
}

inline void SetSkillLevel(CHARACTER_SKILL *skill_value, CHARACTER_SKILL_LEVEL level) {
    *skill_value = (*skill_value & ~0x3F) | (level & 0x3F);
}

inline void SetSkillMastery(CHARACTER_SKILL *skill_value, CharacterSkillMastery mastery) {
    assert(mastery <= CHARACTER_SKILL_MASTERY_GRANDMASTER);

    *skill_value &= 0x3F;

    switch (mastery) {
        case (CHARACTER_SKILL_MASTERY_EXPERT):
            *skill_value |= 0x40;
            break;
        case (CHARACTER_SKILL_MASTERY_MASTER):
            *skill_value |= 0x80;
            break;
        case (CHARACTER_SKILL_MASTERY_GRANDMASTER):
            *skill_value |= 0x100;
            break;
        default:
            return;
    }
}

// TODO(pskelton): drop
/**
 * Construct character skill value using skill mastery and skill level
 */
inline CHARACTER_SKILL ConstructSkillValue(CharacterSkillMastery mastery, CHARACTER_SKILL_LEVEL level) {
    CHARACTER_SKILL skill = 0;

    SetSkillMastery(&skill, mastery);
    SetSkillLevel(&skill, level);

    return skill;
}

// Simple POD-like class for storing full skill value (level and mastery)
class CombinedSkillValue {
    int _level = 0;
    CharacterSkillMastery _mastery = CHARACTER_SKILL_MASTERY_NONE;

 public:
    CombinedSkillValue();
    CombinedSkillValue(int level, CharacterSkillMastery mastery);

    // joins level and mastery into one integer
    [[nodiscard]] uint16_t join() const;

    int level() const;
    CombinedSkillValue &setLevel(int level);

    CharacterSkillMastery mastery() const;
    CombinedSkillValue &setMastery(CharacterSkillMastery mastery);

    static bool isLevelValid(int level);
    static bool isMasteryValid(CharacterSkillMastery mastery);
    static CombinedSkillValue novice();
    static CombinedSkillValue fromJoined(uint16_t);

    explicit operator bool() const { return _level > 0; }
    void reset();
};
