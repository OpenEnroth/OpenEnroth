#include "Engine/Conditions.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/Party.h"

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
    std::array<ItemId, 3> items = {{}};

    constexpr ConditionTableEntry() = default;
    constexpr ConditionTableEntry(ConditionFlags flagsIn,
                                  ItemEnchantment enchantmentIn = ITEM_ENCHANTMENT_NULL,
                                  ItemId item1 = ITEM_NULL,
                                  ItemId item2 = ITEM_NULL,
                                  ItemId item3 = ITEM_NULL) { // NOLINT: we want an implicit constructor.
        flags = flagsIn;
        enchantment = enchantmentIn;
        items[0] = item1;
        items[1] = item2;
        items[2] = item3;
    }
};

static constexpr IndexedArray<ConditionTableEntry, CONDITION_CURSED, CONDITION_ZOMBIE> conditionArray = {
    // hint: condname, protfrommagic, enchantment, ...
    {CONDITION_CURSED,          {0}},
    {CONDITION_WEAK,            {AFFECTED_BY_PROTECTION_FROM_MAGIC}},
    {CONDITION_SLEEP,           {0, ITEM_ENCHANTMENT_OF_ALARMS, ITEM_ARTIFACT_YORUBA}},
    {CONDITION_FEAR,            {0}},
    {CONDITION_DRUNK,           {0}},
    {CONDITION_INSANE,          {0, ITEM_ENCHANTMENT_OF_SANITY, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_POISON_WEAK,     {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_DISEASE_WEAK,    {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_POISON_MEDIUM,   {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_DISEASE_MEDIUM,  {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_POISON_SEVERE,   {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_DISEASE_SEVERE,  {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP}},
    {CONDITION_PARALYZED,       {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_FREEDOM, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_ARTIFACT_GHOULSBANE}},
    {CONDITION_UNCONSCIOUS,     {0}},
    {CONDITION_DEAD,            {AFFECTED_BY_PROTECTION_FROM_MAGIC | REQUIRES_GM_PROTECTION_FROM_MAGIC}},
    {CONDITION_PETRIFIED,       {AFFECTED_BY_PROTECTION_FROM_MAGIC, ITEM_ENCHANTMENT_OF_MEDUSA, ITEM_ARTIFACT_YORUBA, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_RELIC_KELEBRIM}},
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

    if ((entry.flags & REQUIRES_GM_PROTECTION_FROM_MAGIC) && protectionBuff.skillMastery < MASTERY_GRANDMASTER)
        return false;

    assert(protectionBuff.power != 0); // Otherwise the decrement below will overflow.

    --protectionBuff.power;
    if (protectionBuff.power < 1)
        protectionBuff.Reset();
    return true;
}

bool blockCondition(Character *character, Condition condition) {
    assert(!character->conditions.has(condition)); // Expected to be checked externally.

    const ConditionTableEntry &entry = conditionArray[condition];
    if (blockConditionWithProtectionFromMagic(entry))
        return true;

    if (entry.enchantment != ITEM_ENCHANTMENT_NULL && character->wearsEnchantedItem(entry.enchantment))
        return true;

    for (ItemId itemId : entry.items)
        if (itemId != ITEM_NULL && character->wearsItem(itemId))
            return true;

    return false;
}

const std::array<Condition, 18> &conditionImportancyTable() {
    if (engine->config->gameplay.AlternativeConditionPriorities.value())
        return conditionImportancyTableAlternative;
    else
        return conditionImportancyTableDefault;
}
