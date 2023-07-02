#include <cstdlib>

#include "Engine/Conditions.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"

IndexedArray<ConditionProcessor, CONDITION_CURSED, CONDITION_ZOMBIE> conditionArray = {
    // hint: condname, protfrommagic, gmprot, enchantment, ...
    {CONDITION_CURSED, {false, false, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_WEAK, {true, false, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_SLEEP, {false, false, ITEM_ENCHANTMENT_OF_ALARMS, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR}},
    {CONDITION_FEAR, {false, false, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_DRUNK, {false, false, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_INSANE, {false, false, ITEM_ENCHANTMENT_OF_SANITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_WEAK, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_WEAK, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_MEDIUM, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_MEDIUM, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_POISON_SEVERE, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_DISEASE_SEVERE, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {CONDITION_PARALYZED, {false, false, ITEM_ENCHANTMENT_OF_FREEDOM, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK,
                       ITEM_ARTIFACT_GHOULSBANE, ITEM_SLOT_ANY}},
    {CONDITION_UNCONSCIOUS, {false, false, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_DEAD, {true, true, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_PETRIFIED, {true, false, ITEM_ENCHANTMENT_OF_MEDUSA, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK,
                       ITEM_RELIC_KELEBRIM, ITEM_SLOT_ANY}},
    {CONDITION_ERADICATED, {true, true, ITEM_ENCHANTMENT_NULL}},
    {CONDITION_ZOMBIE, {false, false, ITEM_ENCHANTMENT_NULL}}
};

std::array<Condition, 18> conditionImportancyTableDefault = {{
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

std::array<Condition, 18> conditionImportancyTableAlternative = {{
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

bool ConditionProcessor::IsPlayerAffected(Character *inPlayer, Condition condToCheck, int blockable) {
    if (!blockable) return true;
    ConditionProcessor *thisProc = &conditionArray[condToCheck];
    if (thisProc->m_IsBlockedByProtFromMagic &&
        pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Active()) {
        if (!(thisProc->m_DoesNeedGmProtFromMagic &&
              pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].skillMastery < CHARACTER_SKILL_MASTERY_GRANDMASTER)) {
            --pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].power;
            if (pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].power < 1)
                pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Reset();
            return false;
        }
    }
    if (thisProc->m_WorkingEnchantment != ITEM_ENCHANTMENT_NULL) {
        if (inPlayer->HasEnchantedItemEquipped(thisProc->m_WorkingEnchantment))
            return false;
    }
    for (unsigned int i = 0; i < thisProc->m_equipmentPairs.size(); i++) {
        if (thisProc->m_equipmentPairs[i].m_ItemId == (ITEM_TYPE)0)
            return true;
        ITEM_TYPE itemId = thisProc->m_equipmentPairs[i].m_ItemId;
        ITEM_SLOT slot = thisProc->m_equipmentPairs[i].m_EquipSlot;
        if (slot == ITEM_SLOT_ANY) {
            if (inPlayer->wearsItemAnywhere(itemId)) return false;
        } else {
            if (inPlayer->WearsItem(itemId, slot)) return false;
        }
    }
    return true;
}

const std::array<Condition, 18> &conditionImportancyTable() {
    if (engine->config->gameplay.AlternativeConditionPriorities.value())
        return conditionImportancyTableAlternative;
    else
        return conditionImportancyTableDefault;
}
