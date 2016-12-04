#pragma once

/*  296 */
enum BuildingType: unsigned short
{
  BuildingType_WeaponShop = 1,
  BuildingType_ArmorShop = 2,
  BuildingType_MagicShop = 3,
  BuildingType_AlchemistShop = 4,
  BuildingType_FireGuild = 5,
  BuildingType_AirGuild = 6,
  BuildingType_WaterGuild = 7,
  BuildingType_EarthGuild = 8,
  BuildingType_SpiritGuild = 9,
  BuildingType_MindGuild = 10,
  BuildingType_BodyGuild = 11,
  BuildingType_LightGuild = 12,
  BuildingType_DarkGuild = 13,
  BuildingType_ElementalGuild = 14,
  BuildingType_SelfGuild = 15,
  BuildingType_16 = 16,
  BuildingType_TownHall = 17,
  BuildingType_18 = 18,
  BuildingType_19 = 19,
  BuildingType_Throne_Room = 20,
  BuildingType_Tavern = 21,
  BuildingType_Bank = 22,
  BuildingType_Temple = 23,
  BuildingType_24 = 24,
  BuildingType_Unic = 25,
  BuildingType_1A = 26,
  BuildingType_Stables = 27,
  BuildingType_Boats = 28,
  BuildingType_House = 29,
  BuildingType_Training = 30,
  BuildingType_Jail = 31
};

/*  168 */
#pragma pack(push, 1)
struct _2devent
{
  BuildingType uType;
  unsigned __int16 uAnimationID;
  char *pName;
  const char *pProprieterName;
  const char *pEnterText;
  const char *pProprieterTitle;
  __int16 field_14;
  __int16 _state;
  __int16 _rep;
  __int16 _per;
  __int16 field_1C;
  __int16 field_1E;
  float fPriceMultiplier;
  float flt_24;
  unsigned __int16 uOpenTime;
  unsigned __int16 uCloseTime;
  __int16 uExitPicID;
  __int16 uExitMapID;
  __int16 _quest_related;
  __int16 field_32;
};
#pragma pack(pop)

extern _2devent p2DEvents[525];