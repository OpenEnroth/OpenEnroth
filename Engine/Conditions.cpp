#include "Engine/Conditions.h"

#include <cstdlib>

#include "Party.h"

std::array<ConditionProcessor, 18> conditionArray = {{
    // hint: condname, protfrommagic, gmprot, enchantment, ...
    ConditionProcessor(Condition_Cursed, false, false, 0),
    ConditionProcessor(Condition_Weak, true, false, 0),
    ConditionProcessor(Condition_Sleep, false, false, 22, ITEM_ARTIFACT_YORUBA,
                       EQUIP_ARMOUR),
    ConditionProcessor(Condition_Fear, false, false, 0),
    ConditionProcessor(Condition_Drunk, false, false, 0),
    ConditionProcessor(Condition_Insane, false, false, 19, ITEM_ARTIFACT_YORUBA,
                       EQUIP_ARMOUR, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP,
                       EQUIP_CLOAK),
    ConditionProcessor(Condition_Poison_Weak, true, false, 21,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Disease_Weak, true, false, 18,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Poison_Medium, true, false, 21,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Disease_Medium, true, false, 18,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Poison_Severe, true, false, 21,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Disease_Severe, true, false, 18,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK),
    ConditionProcessor(Condition_Paralyzed, false, false, 20,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK,
                       ITEM_ARTIFACT_GHOULSBANE, EQIUP_ANY),
    ConditionProcessor(Condition_Unconcious, false, false, 0),
    ConditionProcessor(Condition_Dead, true, true, 0),
    ConditionProcessor(Condition_Pertified, true, false, 23,
                       ITEM_ARTIFACT_YORUBA, EQUIP_ARMOUR,
                       ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, EQUIP_CLOAK,
                       ITEM_RELIC_KELEBRIM, EQIUP_ANY),
    ConditionProcessor(Condition_Eradicated, true, true, 0),
    ConditionProcessor(Condition_Zombie, false, false, 0)
}};

bool ConditionProcessor::IsPlayerAffected(Player* inPlayer, int condToCheck,
                                          int blockable) {
    if (!blockable) return true;
    ConditionProcessor* thisProc = &conditionArray[condToCheck];
    if (thisProc->m_IsBlockedByProtFromMagic &&
        pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].expire_time) {
        if (!(thisProc->m_DoesNeedGmProtFromMagic &&
              pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].uSkill <
                  4)) {
            --pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].uPower;
            if (pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].uPower <
                1)
                pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Reset();
            return false;
        }
    }
    if (thisProc->m_WorkingEnchantment != 0) {
        if (inPlayer->HasEnchantedItemEquipped(thisProc->m_WorkingEnchantment))
            return false;
    }
    for (unsigned int i = 0; i < thisProc->m_equipmentPairs.size() / 2; i++) {
        if (thisProc->m_equipmentPairs[i * 2].m_ItemId == (ITEM_TYPE)0)
            return true;
        ITEM_TYPE itemId = thisProc->m_equipmentPairs[i * 2].m_ItemId;
        ITEM_EQUIP_TYPE slot =
            thisProc->m_equipmentPairs[i * 2 + 1].m_EquipSlot;
        if (slot == EQIUP_ANY) {
            if (inPlayer->WearsItemAnyWhere(itemId)) return false;
        } else {
            if (inPlayer->WearsItem(itemId, slot)) return false;
        }
    }
    return true;
}
