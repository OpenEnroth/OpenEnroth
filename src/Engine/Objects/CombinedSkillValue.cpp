#include "CombinedSkillValue.h"

CombinedSkillValue::CombinedSkillValue(int joinedValue) {
    _level = ::GetSkillLevel(joinedValue);
    _mastery = ::GetSkillMastery(joinedValue);
}

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

int CombinedSkillValue::join() const {
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
