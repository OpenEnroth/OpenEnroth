#include <cstdlib>

#include "Engine/Conditions.h"

#include "Engine/Party.h"

IndexedArray<ConditionProcessor, Condition_Cursed, Condition_Zombie> conditionArray = {
    // hint: condname, protfrommagic, gmprot, enchantment, ...
    {Condition_Cursed, {false, false, ITEM_ENCHANTMENT_NULL}},
    {Condition_Weak, {true, false, ITEM_ENCHANTMENT_NULL}},
    {Condition_Sleep, {false, false, ITEM_ENCHANTMENT_OF_ALARMS, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR}},
    {Condition_Fear, {false, false, ITEM_ENCHANTMENT_NULL}},
    {Condition_Drunk, {false, false, ITEM_ENCHANTMENT_NULL}},
    {Condition_Insane, {false, false, ITEM_ENCHANTMENT_OF_SANITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Poison_Weak, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Disease_Weak, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Poison_Medium, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Disease_Medium, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Poison_Severe, {true, false, ITEM_ENCHANTMENT_OF_ANTIDOTES, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Disease_Severe, {true, false, ITEM_ENCHANTMENT_OF_IMMUNITY, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK}},
    {Condition_Paralyzed, {false, false, ITEM_ENCHANTMENT_OF_FREEDOM, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK,
                       ITEM_ARTIFACT_GHOULSBANE, ITEM_SLOT_ANY}},
    {Condition_Unconscious, {false, false, ITEM_ENCHANTMENT_NULL}},
    {Condition_Dead, {true, true, ITEM_ENCHANTMENT_NULL}},
    {Condition_Petrified, {true, false, ITEM_ENCHANTMENT_OF_MEDUSA, ITEM_ARTIFACT_YORUBA, ITEM_SLOT_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SLOT_CLOAK,
                       ITEM_RELIC_KELEBRIM, ITEM_SLOT_ANY}},
    {Condition_Eradicated, {true, true, ITEM_ENCHANTMENT_NULL}},
    {Condition_Zombie, {false, false, ITEM_ENCHANTMENT_NULL}}
};

std::array<Condition, 18> conditionImportancyTableDefault = {{
    Condition_Eradicated,
    Condition_Petrified,
    Condition_Dead,
    Condition_Zombie,
    Condition_Unconscious,
    Condition_Sleep,
    Condition_Paralyzed,
    Condition_Disease_Severe,
    Condition_Poison_Severe,
    Condition_Disease_Medium,
    Condition_Poison_Medium,
    Condition_Disease_Weak,
    Condition_Poison_Weak,
    Condition_Insane,
    Condition_Drunk,
    Condition_Fear,
    Condition_Weak,
    Condition_Cursed
}};

std::array<Condition, 18> conditionImportancyTableAlternative = {{
    Condition_Eradicated,
    Condition_Dead,
    Condition_Petrified,
    Condition_Unconscious,
    Condition_Paralyzed,
    Condition_Sleep,
    Condition_Weak,
    Condition_Cursed,
    Condition_Disease_Severe,
    Condition_Poison_Severe,
    Condition_Disease_Medium,
    Condition_Poison_Medium,
    Condition_Disease_Weak,
    Condition_Poison_Weak,
    Condition_Insane,
    Condition_Drunk,
    Condition_Fear,
    Condition_Zombie
}};

bool ConditionProcessor::IsPlayerAffected(Player *inPlayer, Condition condToCheck, int blockable) {
    if (!blockable) return true;
    ConditionProcessor *thisProc = &conditionArray[condToCheck];
    if (thisProc->m_IsBlockedByProtFromMagic &&
        pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].expireTime) {
        if (!(thisProc->m_DoesNeedGmProtFromMagic &&
              pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].skillMastery < PLAYER_SKILL_MASTERY_GRANDMASTER)) {
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
    for (unsigned int i = 0; i < thisProc->m_equipmentPairs.size() / 2; i++) {
        if (thisProc->m_equipmentPairs[i * 2].m_ItemId == (ITEM_TYPE)0)
            return true;
        ITEM_TYPE itemId = thisProc->m_equipmentPairs[i * 2].m_ItemId;
        ITEM_SLOT slot = thisProc->m_equipmentPairs[i * 2 + 1].m_EquipSlot;
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
