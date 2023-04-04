#pragma once
#include <array>

#include "Engine/Objects/Items.h"

#include "Utility/IndexedArray.h"
#include "Utility/Segment.h"

struct Player;

enum class Condition : uint32_t {
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
    Condition_Unconscious = 13,
    Condition_Dead = 14,
    Condition_Petrified = 15,
    Condition_Eradicated = 16,
    Condition_Zombie = 17,
    Condition_Good = 18,
};
using enum Condition;

class ConditionProcessor {
 public:
    bool m_IsBlockedByProtFromMagic = false;
    bool m_DoesNeedGmProtFromMagic = false;
    ITEM_ENCHANTMENT m_WorkingEnchantment = ITEM_ENCHANTMENT_NULL;
    std::array<EquipemntPair, 3> m_equipmentPairs = {{}};

    ConditionProcessor() {}
    ConditionProcessor(bool isblocked, bool needsGm, ITEM_ENCHANTMENT enchantment,
                       ITEM_TYPE itemtype1 = ITEM_NULL,
                       ITEM_SLOT itemslot1 = ITEM_SLOT_INVALID,
                       ITEM_TYPE itemtype2 = ITEM_NULL,
                       ITEM_SLOT itemslot2 = ITEM_SLOT_INVALID,
                       ITEM_TYPE itemtype3 = ITEM_NULL,
                       ITEM_SLOT itemslot3 = ITEM_SLOT_INVALID) {
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

    static bool IsPlayerAffected(Player *, Condition, int);
};

extern IndexedArray<ConditionProcessor, Condition_Cursed, Condition_Zombie> conditionArray;

const std::array<Condition, 18> &conditionImportancyTable();
