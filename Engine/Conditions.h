#pragma once
#include "Engine/Objects/Items.h"
#include <array>
enum Condition: unsigned __int32
{
  Condition_Cursed = 0,
  Condition_Weak = 1,
  Condition_Sleep = 2,
  Condition_Fear = 3,
  Condition_Drunk = 4,
  Condition_Insane = 5,
  Condition_Poison_Weak = 6,
  Condition_Disease_Weak = 7,
  Condition_Poison_Medium = 8,
  Condition_Disease_Medium = 9,
  Condition_Poison_Severe = 10,
  Condition_Disease_Severe = 11,
  Condition_Paralyzed = 12,
  Condition_Unconcious = 13,
  Condition_Dead = 14,
  Condition_Pertified = 15,
  Condition_Eradicated = 16,
  Condition_Zombie = 17,
  Condition_Good = 18
};


class EquipemntPair
{
public:
  ITEM_TYPE m_ItemId;
  ITEM_EQUIP_TYPE  m_EquipSlot;
  EquipemntPair(ITEM_TYPE type, ITEM_EQUIP_TYPE slot)
  {
    m_ItemId = type;
    m_EquipSlot = slot;
  }
  EquipemntPair()
  {
    m_ItemId = (ITEM_TYPE)0;
    m_EquipSlot = (ITEM_EQUIP_TYPE)0;
  }
};

struct Player;

class ConditionProcessor
{
public:
  bool m_IsBlockedByProtFromMagic;
  bool m_DoesNeedGmProtFromMagic;
  int m_WorkingEnchantment;
  std::array<EquipemntPair, 3> m_equipmentPairs;
  ConditionProcessor(int, bool isblocked, bool needsGm, int enchantment, 
    ITEM_TYPE itemtype1 = (ITEM_TYPE)0, ITEM_EQUIP_TYPE itemslot1 = (ITEM_EQUIP_TYPE)0, 
    ITEM_TYPE itemtype2 = (ITEM_TYPE)0, ITEM_EQUIP_TYPE itemslot2 = (ITEM_EQUIP_TYPE)0, 
    ITEM_TYPE itemtype3 = (ITEM_TYPE)0, ITEM_EQUIP_TYPE itemslot3 = (ITEM_EQUIP_TYPE)0)
  {
    m_IsBlockedByProtFromMagic = isblocked;
    m_DoesNeedGmProtFromMagic = needsGm;
    m_WorkingEnchantment = enchantment;
    m_equipmentPairs[0].m_ItemId = itemtype1;
    m_equipmentPairs[0].m_EquipSlot = itemslot1;
    m_equipmentPairs[1].m_ItemId = itemtype2;
    m_equipmentPairs[1].m_EquipSlot = itemslot2;
    m_equipmentPairs[2].m_ItemId = itemtype3;
    m_equipmentPairs[2].m_EquipSlot = itemslot3;
  }

  static bool IsPlayerAffected(Player*, int, int);
};

extern std::array<ConditionProcessor, 18> conditionArray;