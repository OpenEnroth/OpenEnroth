#include <algorithm>

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

void CombinedSkillValue::learn() {
    _level = 1;
    _mastery = PLAYER_SKILL_MASTERY_NOVICE;
}

void CombinedSkillValue::reset() {
    _level = 0;
    _mastery = PLAYER_SKILL_MASTERY_NONE;
}

void CombinedSkillValue::subtract(const int sub) {
    _level -= GetSkillLevel(sub);
    _level = std::max(0, _level);
    // TODO(pskelton): check - should this be able to forget a skill '0' or min of '1'
    // TODO(pskelton): check - should this modify mastery as well
}

void CombinedSkillValue::add(const int add) {
    _level += GetSkillLevel(add);
    PLAYER_SKILL_MASTERY addmast = GetSkillMastery(add);
    if (addmast > _mastery) _mastery = addmast;
}

void CombinedSkillValue::set(const int set) {
    _level = ::GetSkillLevel(set);
    _mastery = ::GetSkillMastery(set);
}
