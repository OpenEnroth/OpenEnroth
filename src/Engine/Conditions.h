#pragma once
#include <array>

#include "Engine/Objects/Items.h"

#include "Utility/IndexedArray.h"
#include "Utility/Segment.h"

struct Player;

enum class Condition : uint32_t {
    CONDITION_CURSED = 0,
    CONDITION_WEAK = 1,
    CONDITION_SLEEP = 2,
    CONDITION_FEAR = 3,
    CONDITION_DRUNK = 4,
    CONDITION_INSANE = 5,
    CONDITION_POISON_WEAK = 6,
    CONDITION_DISEASE_WEAK = 7,
    CONDITION_POISON_MEDIUM = 8,
    CONDITION_DISEASE_MEDIUM = 9,
    CONDITION_POISON_SEVERE = 10,
    CONDITION_DISEASE_SEVERE = 11,
    CONDITION_PARALYZED = 12,
    CONDITION_UNCONSCIOUS = 13,
    CONDITION_DEAD = 14,
    CONDITION_PETRIFIED = 15,
    CONDITION_ERADICATED = 16,
    CONDITION_ZOMBIE = 17,
    CONDITION_GOOD = 18,
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

extern IndexedArray<ConditionProcessor, CONDITION_CURSED, CONDITION_ZOMBIE> conditionArray;

const std::array<Condition, 18> &conditionImportancyTable();
