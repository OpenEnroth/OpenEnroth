#include "CombinedSkillValue.h"

#include <cassert>
#include <string>
#include <utility>
#include "Utility/IndexedArray.h"
#include "Utility/String/Format.h"

CombinedSkillValue::CombinedSkillValue(int level, CharacterSkillMastery mastery) {
    assert(level >= 0 && level <= 63);
    assert((level == 0) ^ (mastery != CHARACTER_SKILL_MASTERY_NONE));

    _level = level;
    _mastery = mastery;
}

CombinedSkillValue::CombinedSkillValue() {
    _level = 0;
    _mastery = CHARACTER_SKILL_MASTERY_NONE;
}

CombinedSkillValue CombinedSkillValue::none() {
    return CombinedSkillValue();
}

CombinedSkillValue CombinedSkillValue::novice(int level) {
    return CombinedSkillValue(level, CHARACTER_SKILL_MASTERY_NOVICE);
}

CombinedSkillValue CombinedSkillValue::increaseLevel(CombinedSkillValue current) {
    assert(current != CombinedSkillValue::none());
    return CombinedSkillValue(current.level() + 1, current.mastery());
}

CombinedSkillValue CombinedSkillValue::increaseMastery(CombinedSkillValue current, CharacterSkillMastery newMastery) {
    assert(current != CombinedSkillValue::none());
    assert(static_cast<int>(newMastery) == static_cast<int>(current.mastery()) + 1);
    return CombinedSkillValue(current.level(), newMastery);
}

CombinedSkillValue CombinedSkillValue::fromJoined(uint16_t joinedValue) {
    auto [level, mastery] = fromJoinedUnchecked(joinedValue);
    return CombinedSkillValue(level, mastery);
}

std::pair<int, CharacterSkillMastery> CombinedSkillValue::fromJoinedUnchecked(uint16_t joinedValue) {
    // Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
    // So max possible stored skill level is 63.
    int level = joinedValue & 0x3F;

    CharacterSkillMastery mastery;
    if (joinedValue & 0x100) {
        mastery = CHARACTER_SKILL_MASTERY_GRANDMASTER;
    } else if (joinedValue & 0x80) {
        mastery = CHARACTER_SKILL_MASTERY_MASTER;
    } else if (joinedValue & 0x40) {
        mastery = CHARACTER_SKILL_MASTERY_EXPERT;
    } else if (level != 0) {
        mastery = CHARACTER_SKILL_MASTERY_NOVICE;
    } else {
        mastery = CHARACTER_SKILL_MASTERY_NONE;
    }

    return {level, mastery};
}

uint16_t CombinedSkillValue::joined() const {
    uint16_t result = _level;

    if (_mastery == CHARACTER_SKILL_MASTERY_EXPERT) {
        result |= 0x40;
    } else if (_mastery == CHARACTER_SKILL_MASTERY_MASTER) {
        result |= 0x80;
    } else if (_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
        result |= 0x100;
    }

    return result;
}

int CombinedSkillValue::level() const {
    return _level;
}

CharacterSkillMastery CombinedSkillValue::mastery() const {
    return _mastery;
}
