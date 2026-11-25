#pragma once

#include <string>

#include "HouseEnums.h"

#include "Engine/MapEnums.h"
#include "Engine/PartyEnums.h"

struct HouseData {
    HouseType uType = HOUSE_TYPE_INVALID;
    uint16_t uAnimationID = 0;
    std::string name;
    std::string pProprieterName;
    std::string pProprieterTitle;
    int16_t generation_interval_days = 0;
    float fPriceMultiplier = 0; // shop price multiplier
    float flt_24 = 0; // skills price multiplier
    // Real MM8 `2dEvents.txt` gives its "The Adventurer's Inn" houses (185/186/188/189/190)
    // a `fPriceMultiplier` of 1.5 - the same small-multiplier scale MM8 uses for ship fares on this
    // shared column, not the 6-60 "quality tier" scale every genuinely-named MM8 tavern uses (and that
    // MM7's cubed/squared tavern formulas were written for). Set from the raw `Type` string at load
    // time (not per house id) so the tavern formulas can fall back to a sane food quantity instead of
    // one that rounds down to a day nobody's packs are ever below.
    bool isGenericAdventurersInn = false;
    uint16_t uOpenTime = 0;
    uint16_t uCloseTime = 0;
    int16_t uExitPicID = 0;
    MapId uExitMapID = MAP_INVALID;
    QuestBit _quest_bit = QBIT_INVALID;
};
