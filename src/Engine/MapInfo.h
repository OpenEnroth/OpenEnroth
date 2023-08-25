#pragma once

#include <cstdint>
#include <array>
#include <string>
#include "Utility/IndexedArray.h"
#include "MapEnums.h"

class Blob;

struct MapInfo {
    std::string pName;
    std::string pFilename;
    std::string pEncounterMonster1Texture;
    std::string pEncounterMonster2Texture;
    std::string pEncounterMonster3Texture;
    unsigned int uNumResets;
    unsigned int uFirstVisitedAt;
    unsigned int uRespawnIntervalDays;
    int _alert_days;
    int _steal_perm;
    int _per;
    char field_2C;
    char LockX5;
    char Trap_D20;
    MAP_TREASURE_LEVEL Treasure_prob;
    char Encounter_percent;
    char EncM1percent;
    char EncM2percent;
    char EncM3percent;
    char Dif_M1;
    uint8_t uEncounterMonster1AtLeast;
    uint8_t uEncounterMonster1AtMost;
    char Dif_M2;
    uint8_t uEncounterMonster2AtLeast;
    uint8_t uEncounterMonster2AtMost;
    char Dif_M3;
    uint8_t uEncounterMonster3AtLeast;
    uint8_t uEncounterMonster3AtMost;
    char field_3D;
    char field_3E;
    char field_3F;
    uint8_t uRedbookTrackID;
    uint8_t uEAXEnv;
    char field_42;
    char field_43;
};

struct MapStats {
    void Initialize(const Blob &mapStats);
    MapId GetMapInfo(const std::string &Str2);
    IndexedArray<MapInfo, MAP_FIRST, MAP_LAST> pInfos;
};

extern struct MapStats *pMapStats;
