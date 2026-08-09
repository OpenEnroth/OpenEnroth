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
    uint16_t uOpenTime = 0;
    uint16_t uCloseTime = 0;
    int16_t uExitPicID = 0;
    MapId uExitMapID = MAP_INVALID;
    QuestBit _quest_bit = QBIT_INVALID;
};
