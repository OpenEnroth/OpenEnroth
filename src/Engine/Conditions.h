#pragma once

#include <array>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"

#include "Utility/IndexedArray.h"

class Character;

// TODO(captainurist): Just hide all of this behind a sane interface.

class EquipmentPair {
 public:
    ITEM_TYPE m_ItemId = ITEM_NULL;
    ITEM_SLOT m_EquipSlot = ITEM_SLOT_INVALID;
    EquipmentPair(ITEM_TYPE type, ITEM_SLOT slot) {
        m_ItemId = type;
        m_EquipSlot = slot;
    }
    EquipmentPair() {}
};

class ConditionProcessor {
 public:
    bool m_IsBlockedByProtFromMagic = false;
    bool m_DoesNeedGmProtFromMagic = false;
    ITEM_ENCHANTMENT m_WorkingEnchantment = ITEM_ENCHANTMENT_NULL;
    std::array<EquipmentPair, 3> m_equipmentPairs = {{}};

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

    static bool IsPlayerAffected(Character *, Condition, int);
};

extern IndexedArray<ConditionProcessor, CONDITION_CURSED, CONDITION_ZOMBIE> conditionArray;

const std::array<Condition, 18> &conditionImportancyTable();
