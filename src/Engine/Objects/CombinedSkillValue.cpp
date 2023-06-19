#include <algorithm>

#include "CombinedSkillValue.h"

CombinedSkillValue::CombinedSkillValue(int level, PLAYER_SKILL_MASTERY mastery) {
    assert(isLevelValid(level));
    assert(isMasteryValid(mastery));
    _level = level;
    _mastery = mastery;
}

CombinedSkillValue::CombinedSkillValue() {
    _level = 0;
    _mastery = PLAYER_SKILL_MASTERY_NONE;
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

PLAYER_SKILL_MASTERY CombinedSkillValue::mastery() const {
    return _mastery;
}

CombinedSkillValue& CombinedSkillValue::setMastery(PLAYER_SKILL_MASTERY mastery) {
    assert(isMasteryValid(mastery));
    _mastery = mastery;
    return *this;
}

bool CombinedSkillValue::isLevelValid(int level) {
    return level >= 0 && level <= 63;
}

bool CombinedSkillValue::isMasteryValid(PLAYER_SKILL_MASTERY mastery) {
    return mastery == PLAYER_SKILL_MASTERY_NONE || mastery == PLAYER_SKILL_MASTERY_NOVICE || mastery == PLAYER_SKILL_MASTERY_EXPERT ||
           mastery == PLAYER_SKILL_MASTERY_MASTER || mastery == PLAYER_SKILL_MASTERY_GRANDMASTER;
}

CombinedSkillValue CombinedSkillValue::fromJoined(uint16_t joinedValue) {
    PLAYER_SKILL_LEVEL lvl  = ::GetSkillLevel(joinedValue);
    PLAYER_SKILL_MASTERY mst = ::GetSkillMastery(joinedValue);
    return CombinedSkillValue(lvl, mst);
}

CombinedSkillValue CombinedSkillValue::novice() {
    return CombinedSkillValue(1, PLAYER_SKILL_MASTERY_NOVICE);
}

void CombinedSkillValue::reset() {
    _level = 0;
    _mastery = PLAYER_SKILL_MASTERY_NONE;
}
