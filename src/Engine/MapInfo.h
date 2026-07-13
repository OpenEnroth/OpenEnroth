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
    std::string encounter1MonsterInternalName; //
    std::string encounter2MonsterInternalName; // E.g. "Angel", w/o the A/B/C suffix.
    std::string encounter3MonsterInternalName; //
    unsigned int numResets = 0; // Unused, always 0. Actual number of respawns is stored in `LocationInfo`.
    unsigned int firstVisitedAt = 0; // Unused, always 0.
    unsigned int respawnIntervalDays = 0;
    int alertDays = 0; // Unused, always 7.
    int baseStealingFine = 0; // Base fine for stealing, actual base fine will be 100x this number in gold.
    int perceptionDifficulty = 0; // Difficulty level for perceptions checks, perception roll should be at least 2x this number to succeed.
    char field_2C = 0;
    char disarmDifficulty = 0; // Difficulty level for disarm trap checks, disarm roll should be at least 2x this number to succeed.
    char trapDamageD20DiceCount = 0; // Traps will deal this number d20 total damage.
    MapTreasureLevel mapTreasureLevel = MAP_TREASURE_LEVEL_1;
    char encounterChance = 0; // In [0, 100], chance for an encounter when resting.
    char encounter1Chance = 0; //
    char encounter2Chance = 0; // These three add up to 100, or are all 0.
    char encounter3Chance = 0; //
    char Dif_M1 = 0;
    uint8_t encounter1MinCount = 0;
    uint8_t encounter1MaxCount = 0;
    char Dif_M2 = 0;
    uint8_t encounter2MinCount = 0;
    uint8_t encounter2MaxCount = 0;
    char Dif_M3 = 0;
    uint8_t encounter3MinCount = 0;
    uint8_t encounter3MaxCount = 0;
    char field_3D = 0;
    char field_3E = 0;
    char field_3F = 0;
    MusicId musicId = MUSIC_INVALID;
    uint8_t uEAXEnv = 0; // TODO(captainurist): EAX audio reverb preset (0-26); set per-map - wire up to the audio backend.
    char field_42 = 0;
    char field_43 = 0;
};

struct MapStats {
    void Initialize(const Blob &mapStats);
    MapId GetMapInfo(std::string_view Str2);
    IndexedArray<MapInfo, MAP_FIRST, MAP_LAST> pInfos;
};

extern MapStats *pMapStats;
