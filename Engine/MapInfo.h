#pragma once


enum MAP_TYPE: unsigned __int32
{
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

enum MapStartPoint: unsigned __int32
{
  MapStartPoint_Party = 0,
  MapStartPoint_North = 1,
  MapStartPoint_South = 2,
  MapStartPoint_East = 3,
  MapStartPoint_West = 4
};


/*  192 */
#pragma pack(push, 1)
struct MapInfo
{
  int SpawnRandomTreasure(struct SpawnPointMM7 *a2);

  char *pName;
  char *pFilename;
  char *pEncounterMonster1Texture;
  char *pEncounterMonster2Texture;
  char *pEncounterMonster3Texture;
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
  unsigned __int8 uEncounterMonster1AtLeast;
  unsigned __int8 uEncounterMonster1AtMost;
  char Dif_M2;
  unsigned __int8 uEncounterMonster2AtLeast;
  unsigned __int8 uEncounterMonster2AtMost;
  char Dif_M3;
  unsigned __int8 uEncounterMonster3AtLeast;
  unsigned __int8 uEncounterMonster3AtMost;
  char field_3D;
  char field_3E;
  char field_3F;
  unsigned __int8 uRedbookTrackID;
  unsigned __int8 uEAXEnv;
  char field_42;
  char field_43;
};
#pragma pack(pop)



/*  193 */
#pragma pack(push, 1)
struct MapStats
{
  void Initialize();
  MAP_TYPE GetMapInfo(const char *Str2);
  int sub_410D99_get_map_index(int a1);
  MapInfo pInfos[77];
  unsigned int uNumMaps;


};
#pragma pack(pop)




extern struct MapStats *pMapStats;


extern MapStartPoint uLevel_StartingPointType; // weak


void TeleportToStartingPoint(MapStartPoint point); // idb
