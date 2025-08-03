#pragma once

#include <cstdint>
#include <utility>
#include <string>

#include "Engine/Objects/CharacterEnums.h"

/**
 * Simple POD-like class for storing full skill value (level and mastery).
 */
class CombinedSkillValue {
 public:
    CombinedSkillValue();
    CombinedSkillValue(int level, Mastery mastery);

    /**
     * @param level                     Skill level to check.
     * @param mastery                   Skill mastery to check.
     * @return                          Whether the provided skill-mastery pair is valid. Note that `CombinedSkillValue`
     *                                  constructor asserts on invalid skill-mastery pairs.
     */
    static bool isValid(int level, Mastery mastery);

    static CombinedSkillValue none();
    static CombinedSkillValue novice(int level = 1);
    static CombinedSkillValue increaseLevel(CombinedSkillValue current);
    static CombinedSkillValue increaseMastery(CombinedSkillValue current, Mastery newMastery);
    static CombinedSkillValue fromJoined(uint16_t joinedValue);
    static std::pair<int, Mastery> fromJoinedUnchecked(uint16_t joinedValue);

    /**
     * @return                          Binary representation of a skill-mastery pair, as it was originally stored
     *                                  in M&M data structures.
     */
    [[nodiscard]] uint16_t joined() const;

    [[nodiscard]] int level() const;
    [[nodiscard]] Mastery mastery() const;

    explicit operator bool() const { return _level > 0; }

    friend bool operator==(const CombinedSkillValue &l, const CombinedSkillValue &r) = default;

 private:
    int _level = 0;
    Mastery _mastery = MASTERY_NONE;
};
