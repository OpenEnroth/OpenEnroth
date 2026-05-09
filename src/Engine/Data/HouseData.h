#pragma once

#include <string>

#include "HouseEnums.h"

#include "Engine/MapEnums.h"
#include "Engine/PartyEnums.h"

// TODO(captainurist): drop everything that's not used here.
struct HouseData {
    HouseType uType = HOUSE_TYPE_INVALID;
    uint16_t uAnimationID = 0;
    std::string name;
    std::string pProprieterName;
    std::string pEnterText;
    std::string pProprieterTitle;
    int16_t field_14 = 0;
    int16_t _state = 0;
    int16_t _rep = 0;
    int16_t _per = 0;
    int16_t generation_interval_days = 0;
    int16_t field_1E = 0;
    float fPriceMultiplier = 0; // shop price multiplier
    float flt_24 = 0; // skills price multiplier
    uint16_t uOpenTime = 0;
    uint16_t uCloseTime = 0;
    int16_t uExitPicID = 0;
    MapId uExitMapID = MAP_INVALID;
    QuestBit _quest_bit = QBIT_INVALID;
    int16_t field_32 = 0;
};
