#include "CombinedSkillValue.h"

#include <cassert>

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

CombinedSkillValue CombinedSkillValue::novice() {
    return CombinedSkillValue(1, CHARACTER_SKILL_MASTERY_NOVICE);
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
    CHARACTER_SKILL_LEVEL lvl  = ::GetSkillLevel(joinedValue);
    CharacterSkillMastery mst = ::GetSkillMastery(joinedValue);
    return CombinedSkillValue(lvl, mst);
}

uint16_t CombinedSkillValue::join() const {
    return ::ConstructSkillValue(_mastery, _level);
}

int CombinedSkillValue::level() const {
    return _level;
}

CharacterSkillMastery CombinedSkillValue::mastery() const {
    return _mastery;
}
