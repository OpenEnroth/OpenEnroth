#pragma once

#include <string>

#include "BuildingEnums.h"

#include "Engine/MapEnums.h"
#include "Engine/PartyEnums.h"

struct BuildingData {
    BuildingType uType;
    uint16_t uAnimationID;
    std::string name;
    std::string pProprieterName;
    std::string pEnterText;
    std::string pProprieterTitle;
    int16_t field_14;
    int16_t _state;
    int16_t _rep;
    int16_t _per;
    int16_t generation_interval_days;
    int16_t field_1E;
    float fPriceMultiplier; // shop price multiplier
    float flt_24; // skills price multiplier
    uint16_t uOpenTime;
    uint16_t uCloseTime;
    int16_t uExitPicID;
    MapId uExitMapID;
    QuestBit _quest_bit;
    int16_t field_32;
};
