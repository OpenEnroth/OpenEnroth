#include <algorithm>

#include "CombinedSkillValue.h"

CombinedSkillValue::CombinedSkillValue(int level, CharacterSkillMastery mastery) {
    assert(isLevelValid(level));
    assert(isMasteryValid(mastery));
    _level = level;
    _mastery = mastery;
}

CombinedSkillValue::CombinedSkillValue() {
    _level = 0;
    _mastery = CHARACTER_SKILL_MASTERY_NONE;
}

uint16_t CombinedSkillValue::join() const {
    return ::ConstructSkillValue(_mastery, _level);
}

int CombinedSkillValue::level() const {
    return _level;
}

CombinedSkillValue& CombinedSkillValue::setLevel(int level) {
    assert(isLevelValid(level));
    _level = level;
    return *this;
}

CharacterSkillMastery CombinedSkillValue::mastery() const {
    return _mastery;
}

CombinedSkillValue& CombinedSkillValue::setMastery(CharacterSkillMastery mastery) {
    assert(isMasteryValid(mastery));
    _mastery = mastery;
    return *this;
}

bool CombinedSkillValue::isLevelValid(int level) {
    return level >= 0 && level <= 63;
}

bool CombinedSkillValue::isMasteryValid(CharacterSkillMastery mastery) {
    return mastery == CHARACTER_SKILL_MASTERY_NONE || mastery == CHARACTER_SKILL_MASTERY_NOVICE || mastery == CHARACTER_SKILL_MASTERY_EXPERT ||
           mastery == CHARACTER_SKILL_MASTERY_MASTER || mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER;
}

CombinedSkillValue CombinedSkillValue::fromJoined(uint16_t joinedValue) {
    CHARACTER_SKILL_LEVEL lvl  = ::GetSkillLevel(joinedValue);
    CharacterSkillMastery mst = ::GetSkillMastery(joinedValue);
    return CombinedSkillValue(lvl, mst);
}

CombinedSkillValue CombinedSkillValue::novice() {
    return CombinedSkillValue(1, CHARACTER_SKILL_MASTERY_NOVICE);
}

void CombinedSkillValue::reset() {
    _level = 0;
    _mastery = CHARACTER_SKILL_MASTERY_NONE;
}
