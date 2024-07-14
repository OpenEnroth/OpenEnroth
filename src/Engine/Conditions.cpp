#include "Engine/Conditions.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/Party.h"

struct ConditionEquipment {
    ItemId item = ITEM_NULL;
    ItemSlot slot = ITEM_SLOT_INVALID;
};

enum class ConditionFlag {
    AFFECTED_BY_PROTECTION_FROM_MAGIC = 0x1,
    REQUIRES_GM_PROTECTION_FROM_MAGIC = 0x2
};
using enum ConditionFlag;
MM_DECLARE_FLAGS(ConditionFlags, ConditionFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(ConditionFlags)

struct ConditionTableEntry {
    ConditionFlags flags;
    ItemEnchantment enchantment = ITEM_ENCHANTMENT_NULL;
    std::array<ConditionEquipment, 3> equipment = {{}};

    constexpr ConditionTableEntry() = default;
    constexpr ConditionTableEntry(ConditionFlags flagsIn,
                                  ItemEnchantment enchantmentIn = ITEM_ENCHANTMENT_NULL,
                                  ItemId item1 = ITEM_NULL,
                                  ItemSlot slot1 = ITEM_SLOT_INVALID,
                                  ItemId item2 = ITEM_NULL,
                                  ItemSlot slot2 = ITEM_SLOT_INVALID,
                                  ItemId item3 = ITEM_NULL,
                                  ItemSlot slot3 = ITEM_SLOT_INVALID) { // NOLINT: we want an explicit constructor.
        flags = flagsIn;
        enchantment = enchantmentIn;
        equipment[0].item = item1;
        equipment[0].slot = slot1;
        equipment[1].item = item2;
        equipment[1].slot = slot2;
        equipment[2].item = item3;
        equipment[2].slot = slot3;
    }
};

static constexpr IndexedArray<ConditionTableEntry, CONDITION_CURSED, CONDITION_ZOMBIE> conditionArray = {
    // hint: condname, protfrommagic, enchantment, ...
    {CONDITION_CURSED,          {0}},
    {CONDITION_WEAK,            {AFFECTED_BY_PROTECTION_FROM_MAGIC}},
    {CONDITION_SLEEP,           {0, ITEM_ENCHANTMENT_OF_ALARMS,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR}},
    {CONDITION_FEAR,            {0}},
    {CONDITION_DRUNK,           {0}},
    {CONDITION_INSANE,          {0, ITEM_ENCHANTMENT_OF_SANITY,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_WEAK,     {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_WEAK,    {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_MEDIUM,   {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_MEDIUM,  {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_SEVERE,   {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_SEVERE,  {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_PARALYZED,       {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_FREEDOM,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK, ITEM_ARTIFACT_GHOULSBANE, ITEM_SLOT_ANY}},
    {CONDITION_UNCONSCIOUS,     {0}},
    {CONDITION_DEAD,            {AFFECTED_BY_PROTECTION_FROM_MAGIC | REQUIRES_GM_PROTECTION_FROM_MAGIC}},
    {CONDITION_PETRIFIED,       {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_MEDUSA,
                                 ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK, ITEM_RELIC_KELEBRIM, ITEM_SLOT_ANY}},
    {CONDITION_ERADICATED,      {AFFECTED_BY_PROTECTION_FROM_MAGIC | REQUIRES_GM_PROTECTION_FROM_MAGIC}},
    {CONDITION_ZOMBIE,          {0}}
};

static std::array<Condition, 18> conditionImportancyTableDefault = {{
    CONDITION_ERADICATED,
    CONDITION_PETRIFIED,
    CONDITION_DEAD,
    CONDITION_ZOMBIE,
    CONDITION_UNCONSCIOUS,
    CONDITION_SLEEP,
    CONDITION_PARALYZED,
    CONDITION_DISEASE_SEVERE,
    CONDITION_POISON_SEVERE,
    CONDITION_DISEASE_MEDIUM,
    CONDITION_POISON_MEDIUM,
    CONDITION_DISEASE_WEAK,
    CONDITION_POISON_WEAK,
    CONDITION_INSANE,
    CONDITION_DRUNK,
    CONDITION_FEAR,
    CONDITION_WEAK,
    CONDITION_CURSED
}};

static std::array<Condition, 18> conditionImportancyTableAlternative = {{
    CONDITION_ERADICATED,
    CONDITION_DEAD,
    CONDITION_PETRIFIED,
    CONDITION_UNCONSCIOUS,
    CONDITION_PARALYZED,
    CONDITION_SLEEP,
    CONDITION_WEAK,
    CONDITION_CURSED,
    CONDITION_DISEASE_SEVERE,
    CONDITION_POISON_SEVERE,
    CONDITION_DISEASE_MEDIUM,
    CONDITION_POISON_MEDIUM,
    CONDITION_DISEASE_WEAK,
    CONDITION_POISON_WEAK,
    CONDITION_INSANE,
    CONDITION_DRUNK,
    CONDITION_FEAR,
    CONDITION_ZOMBIE
}};

static bool blockConditionWithProtectionFromMagic(const ConditionTableEntry &entry) {
    if (!(entry.flags & AFFECTED_BY_PROTECTION_FROM_MAGIC))
        return false;

    SpellBuff &protectionBuff = pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC];
    if (!protectionBuff.Active())
        return false;

    if ((entry.flags & REQUIRES_GM_PROTECTION_FROM_MAGIC) && protectionBuff.skillMastery < CHARACTER_SKILL_MASTERY_GRANDMASTER)
        return false;

    assert(protectionBuff.power != 0); // Otherwise the decrement below will overflow.

    --protectionBuff.power;
    if (protectionBuff.power < 1)
        protectionBuff.Reset();
    return true;
}

bool blockCondition(Character *character, Condition condition) {
    assert(!character->conditions.Has(condition)); // Expected to be checked externally.

    const ConditionTableEntry &entry = conditionArray[condition];
    if (blockConditionWithProtectionFromMagic(entry))
        return true;

    if (entry.enchantment != ITEM_ENCHANTMENT_NULL && character->HasEnchantedItemEquipped(entry.enchantment))
        return true;

    for (const ConditionEquipment &pair : entry.equipment) {
        if (pair.item == ITEM_NULL)
            break;

        if (pair.slot == ITEM_SLOT_ANY) {
            if (character->wearsItemAnywhere(pair.item))
                return true;
        } else {
            if (character->WearsItem(pair.item, pair.slot))
                return true;
        }
    }

    return false;
}

const std::array<Condition, 18> &conditionImportancyTable() {
    if (engine->config->gameplay.AlternativeConditionPriorities.value())
        return conditionImportancyTableAlternative;
    else
        return conditionImportancyTableDefault;
}
