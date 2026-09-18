#include "CombinedSkillValue.h"

#include <cassert>
#include <string>
#include <utility>

#include "Engine/Objects/CharacterEnumFunctions.h"

#include "Library/Serialization/Serialization.h"
#include "Library/Serialization/SerializationExceptions.h"

#include "Utility/IndexedArray.h"
#include "Utility/Exception.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"

static constexpr IndexedArray<char, MASTERY_FIRST, MASTERY_LAST> masteryLetters = {{
    {MASTERY_NOVICE,        'N'},
    {MASTERY_EXPERT,        'E'},
    {MASTERY_MASTER,        'M'},
    {MASTERY_GRANDMASTER,   'G'}
}};

CombinedSkillValue::CombinedSkillValue(int level, Mastery mastery) {
    assert(isValid(level, mastery));

    _level = level;
    _mastery = mastery;
}

CombinedSkillValue::CombinedSkillValue() {
    _level = 0;
    _mastery = MASTERY_NONE;
}

bool CombinedSkillValue::isValid(int level, Mastery mastery) {
    if (level < 0 || level > 63)
        return false;

    if ((level == 0) == (mastery != MASTERY_NONE))
        return false;

    return true;
}

CombinedSkillValue CombinedSkillValue::none() {
    return CombinedSkillValue();
}

CombinedSkillValue CombinedSkillValue::novice(int level) {
    return CombinedSkillValue(level, MASTERY_NOVICE);
}

CombinedSkillValue CombinedSkillValue::increaseLevel(CombinedSkillValue current) {
    assert(current != CombinedSkillValue::none());
    return CombinedSkillValue(current.level() + 1, current.mastery());
}

CombinedSkillValue CombinedSkillValue::increaseMastery(CombinedSkillValue current, Mastery newMastery) {
    assert(current != CombinedSkillValue::none());
    assert(static_cast<int>(newMastery) == static_cast<int>(current.mastery()) + 1);
    return CombinedSkillValue(current.level(), newMastery);
}

CombinedSkillValue CombinedSkillValue::fromJoined(uint16_t joinedValue) {
    auto [level, mastery] = fromJoinedUnchecked(joinedValue);
    return CombinedSkillValue(level, mastery);
}

std::pair<int, Mastery> CombinedSkillValue::fromJoinedUnchecked(uint16_t joinedValue) {
    // Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
    // So max possible stored skill level is 63.
    int level = joinedValue & 0x3F;

    Mastery mastery;
    if (joinedValue & 0x100) {
        mastery = MASTERY_GRANDMASTER;
    } else if (joinedValue & 0x80) {
        mastery = MASTERY_MASTER;
    } else if (joinedValue & 0x40) {
        mastery = MASTERY_EXPERT;
    } else if (level != 0) {
        mastery = MASTERY_NOVICE;
    } else {
        mastery = MASTERY_NONE;
    }

    return {level, mastery};
}

uint16_t CombinedSkillValue::joined() const {
    uint16_t result = _level;

    if (_mastery == MASTERY_EXPERT) {
        result |= 0x40;
    } else if (_mastery == MASTERY_MASTER) {
        result |= 0x80;
    } else if (_mastery == MASTERY_GRANDMASTER) {
        result |= 0x100;
    }

    return result;
}

int CombinedSkillValue::level() const {
    return _level;
}

Mastery CombinedSkillValue::mastery() const {
    return _mastery;
}

bool trySerialize(const CombinedSkillValue &src, std::string *dst) {
    if (!src)
        return false;

    *dst = fmt::format("{}{}", masteryLetters[src.mastery()], src.level());
    return true;
}

bool tryDeserialize(std::string_view src, CombinedSkillValue *dst) {
    if (src.empty())
        return false;

    for (Mastery mastery : allSkillMasteries()) {
        if (masteryLetters[mastery] != ascii::toUpper(src[0]))
            continue;

        int level;
        if (!tryDeserialize(src.substr(1), &level) || !CombinedSkillValue::isValid(level, mastery))
            return false;

        *dst = CombinedSkillValue(level, mastery);
        return true;
    }

    return false;
}

void serialize(const CombinedSkillValue &src, std::string *dst) {
    if (!trySerialize(src, dst))
        throw Exception("Cannot serialize an empty skill value");
}

void deserialize(std::string_view src, CombinedSkillValue *dst) {
    if (!tryDeserialize(src, dst))
        throwDeserializationError(src, "CombinedSkillValue");
}
