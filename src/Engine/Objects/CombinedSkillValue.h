#pragma once

#include <cstdint>

#include "Engine/Objects/CharacterEnums.h"

#include "Utility/Segment.h"

/* Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
 * So max possible stored skill level is 63.
 */

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
 public:
    CombinedSkillValue();
    CombinedSkillValue(int level, CharacterSkillMastery mastery);

    static CombinedSkillValue none();
    static CombinedSkillValue novice();
    static CombinedSkillValue increaseLevel(CombinedSkillValue current);
    static CombinedSkillValue increaseMastery(CombinedSkillValue current, CharacterSkillMastery newMastery);
    static CombinedSkillValue fromJoined(uint16_t);

    /**
     * @return                          Binary representation of a skill-mastery pair, as it was originally stored
     *                                  in M&M data structures.
     */
    [[nodiscard]] uint16_t join() const;

    [[nodiscard]] int level() const;
    [[nodiscard]] CharacterSkillMastery mastery() const;

    explicit operator bool() const { return _level > 0; }

    friend bool operator==(const CombinedSkillValue &l, const CombinedSkillValue &r) = default;

 private:
    int _level = 0;
    CharacterSkillMastery _mastery = CHARACTER_SKILL_MASTERY_NONE;
};
