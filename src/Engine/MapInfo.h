#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Media/Audio/SoundEnums.h"

#include "Utility/IndexedArray.h"

#include "MapEnumFunctions.h"
#include "MapEnums.h"

class Blob;

struct MapInfo {
    std::string name; // Display name, e.g. "The Tularean Forest".
    std::string fileName; // E.g. "out02.odm".
    std::string encounter1MonsterTexture;
    std::string encounter2MonsterTexture;
    std::string encounter3MonsterTexture;
    unsigned int numResets; // Unused, always 0. Actual number of respawns is stored in `LocationInfo`.
    unsigned int firstVisitedAt; // Unused, always 0.
    unsigned int respawnIntervalDays;
    int alertDays; // Unused, always 7.
    int baseStealingFine; // Base fine for stealing, actual base fine will be 100x this number in gold.
    int perceptionDifficulty; // Difficulty level for perceptions checks, perception roll should be at least 2x this number to succeed.
    char field_2C;
    char disarmDifficulty; // Difficulty level for disarm trap checks, disarm roll should be at least 2x this number to succeed.
    char trapDamageD20DiceCount; // Traps will deal this number d20 total damage.
    MapTreasureLevel mapTreasureLevel;
    char encounterChance; // In [0, 100], chance for an encounter when resting.
    char encounter1Chance; //
    char encounter2Chance; // These three add up to 100, or are all 0.
    char encounter3Chance; //
    char Dif_M1;
    uint8_t encounter1MinCount;
    uint8_t encounter1MaxCount;
    char Dif_M2;
    uint8_t encounter2MinCount;
    uint8_t encounter2MaxCount;
    char Dif_M3;
    uint8_t encounter3MinCount;
    uint8_t encounter3MaxCount;
    char field_3D;
    char field_3E;
    char field_3F;
    MusicId musicId;
    uint8_t uEAXEnv;
    char field_42;
    char field_43;
};

struct MapStats {
    void Initialize(const Blob &mapStats);
    MapId GetMapInfo(std::string_view Str2);
    IndexedArray<MapInfo, MAP_FIRST, MAP_LAST> pInfos;
};

extern MapStats *pMapStats;
