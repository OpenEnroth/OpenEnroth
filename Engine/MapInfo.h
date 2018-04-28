#pragma once

#include "Engine/Strings.h"

enum MAP_TYPE : uint32_t {
    MAP_INVALID = 0,
    MAP_EMERALD_ISLE = 1,
    MAP_HARMONDALE = 2,
    MAP_STEADWICK = 3,
    MAP_PIERPONT = 4,
    MAP_DEYJA = 5,
    MAP_BRAKADA_DESERT = 6,
    MAP_CELESTIA = 7,
    MAP_THE_PIT = 8,
    MAP_EVENMORN_ISLE = 9,
    MAP_MOUNT_NIGHON = 10,
    MAP_BARROW_DOWNS = 11,
    MAP_LAND_OF_GIANTS = 12,
    MAP_TATALIA = 13,
    MAP_AVLEE = 14,
    MAP_SHOALS = 15,
    //...
    MAP_ARENA = 76,
    //...
};

enum MapStartPoint : uint32_t {
    MapStartPoint_Party = 0,
    MapStartPoint_North = 1,
    MapStartPoint_South = 2,
    MapStartPoint_East = 3,
    MapStartPoint_West = 4
};

struct MapInfo {
    int SpawnRandomTreasure(struct SpawnPointMM7 *a2);

    String pName;
    String pFilename;
    String pEncounterMonster1Texture;
    String pEncounterMonster2Texture;
    String pEncounterMonster3Texture;
    unsigned int uNumResets;
    unsigned int uFirstVisitedAt;
    unsigned int uRespawnIntervalDays;
    int _alert_days;
    int _steal_perm;
    int _per;
    char field_2C;
    char LockX5;
    char Trap_D20;
    char Treasure_prob;
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
    void Initialize();
    MAP_TYPE GetMapInfo(const String &Str2);
    int sub_410D99_get_map_index(int a1);
    MapInfo pInfos[77];
    unsigned int uNumMaps;
};

extern struct MapStats *pMapStats;

extern MapStartPoint uLevel_StartingPointType;

void TeleportToStartingPoint(MapStartPoint point);  // idb
