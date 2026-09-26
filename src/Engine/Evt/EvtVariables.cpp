#include "Engine/Evt/EvtVariables.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "Engine/Engine.h"
#include "Engine/Evt/EvtEnumFunctions.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/HistoryTable.h"
#include "Engine/Tables/QuestTable.h"

#include "GUI/UI/Books/AutonotesBook.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"


#include "Library/Logger/Logger.h"

static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

static bool isInRange(int value, auto first, auto last) {
    return value >= static_cast<int>(first) && value <= static_cast<int>(last);
}

bool isEvtVariableValueValid(EvtOpcode opcode, EvtVariable var, int value) {
    if (var >= VAR_FIRST_SKILL && var <= VAR_LAST_SKILL) {
        if (!isInRange(value, 0, std::numeric_limits<uint16_t>::max()))
            return false;
        if (opcode != EVENT_Set)
            return true; // Compare and add give meaning to mastery bits without a level, 0x80 is "master or better".
        auto [level, mastery] = CombinedSkillValue::fromJoinedUnchecked(value);
        return CombinedSkillValue::isValid(level, mastery);
    }

    switch (var) {
        case VAR_Sex:
            return isInRange(value, SEX_FIRST, SEX_LAST);
        case VAR_Class:
            return isInRange(value, CLASS_FIRST, CLASS_LAST);
        case VAR_Race:
            return isInRange(value, RACE_FIRST, RACE_LAST);
        case VAR_HiredNPCHasSpeciality:
            return isInRange(value, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST);
        case VAR_Award:
            return isInRange(value, AWARD_FIRST, AWARD_LAST);
        case VAR_QBits_QuestsDone:
            return isInRange(value, QBIT_FIRST, QBIT_LAST);
        case VAR_PlayerItemInHands:
        case VAR_ItemEquipped:
            return isInRange(value, ITEM_FIRST_VALID, ITEM_LAST_VALID);
        case VAR_RandomGold:
        case VAR_RandomFood:
            return value > 0;
        case VAR_AutoNotes:
            return pParty->_autonoteBits.indices().contains(value);
        case VAR_PlayerBits:
            return pParty->pCharacters[0]._characterEventBits.indices().contains(value);
        case VAR_NPCs2:
            return isInRange(value, 0, std::ssize(pNPCStats->pNPCData) - 1);
        default:
            return true;
    }
}

static bool CmpSkillValue(int valToCompare, CombinedSkillValue skillValue) {
    int val;
    if (valToCompare <= 63)
        val = skillValue.level();
    else
        val = skillValue.joined();
    return val >= valToCompare;
}

//----- (00449BB4) --------------------------------------------------------
bool compareEvtVariable(Character &character, EvtVariable VarNum, int pValue) {
    signed int v4;                         // edi@1
    uint8_t test_bit_value;        // eax@25
    uint8_t byteWithRequestedBit;  // cl@25
    LocationInfo *v19;                   // eax@122
    LocationInfo *v21;                   // eax@126
    int actStat;                           // ebx@161
    int baseStat;                          // eax@161

    if (VarNum >= VAR_MapPersistentVariable_0 && VarNum <= VAR_MapPersistentVariable_74)
        return (uint8_t)engine->_persistentVariables.mapVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentVariable_0)] >= pValue;

    // not really sure whether the number gets up to 99, but can't ignore the possibility
    if (VarNum >= VAR_MapPersistentDecorVariable_0 && VarNum <= VAR_MapPersistentDecorVariable_24)
        return (uint8_t)engine->_persistentVariables.decorVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] >= pValue;

    switch (VarNum) {
        case VAR_Sex:
            return pValue == std::to_underlying(character.uSex);
        case VAR_Class:
            return pValue == std::to_underlying(character.classType);
        case VAR_Race:
            return pValue == std::to_underlying(character.GetRace());
        case VAR_CurrentHP:
            return character.health >= pValue;
        case VAR_MaxHP:
            return (character.health >= character.GetMaxHealth());
        case VAR_CurrentSP:
            return character.mana >= pValue;
        case VAR_MaxSP:
            return (character.mana >= character.GetMaxMana());
        case VAR_ActualAC:
            return character.GetActualAC() >= pValue;
        case VAR_ACModifier:
            return character.sACModifier >= pValue;
        case VAR_BaseLevel:
            return character.uLevel >= pValue;
        case VAR_LevelModifier:
            return character.sLevelModifier >= pValue;
        case VAR_Age:
            return character.GetActualAge() >= pValue;
        case VAR_Award:
            return character._achievedAwardsBits[static_cast<AwardId>(pValue)];
        case VAR_Experience:
            return character.experience >= pValue;
        case VAR_QBits_QuestsDone:
            return pParty->_questBits[static_cast<QuestBit>(pValue)];
        case VAR_PlayerItemInHands:
            // for (int i = 0; i < 138; i++)
            if (character.inventory.find(static_cast<ItemId>(pValue)))
                return true;
            return pParty->pPickedItem.itemId == static_cast<ItemId>(pValue);

        case VAR_Hour:
            return pParty->GetPlayingTime().toCivilTime().hour == pValue;

        case VAR_DayOfYear:
            return pParty->GetPlayingTime().toDays() % 336 + 1 == pValue;

        case VAR_DayOfWeek:
            return pParty->GetPlayingTime().toDays() % 7 == pValue;

        case VAR_FixedGold:
            return pParty->GetGold() >= pValue;
        case VAR_FixedFood:
            return pParty->GetFood() >= pValue;
        case VAR_MightBonus:
            return character._statBonuses[ATTRIBUTE_MIGHT] >= pValue;
        case VAR_IntellectBonus:
            return character._statBonuses[ATTRIBUTE_INTELLIGENCE] >= pValue;
        case VAR_PersonalityBonus:
            return character._statBonuses[ATTRIBUTE_PERSONALITY] >= pValue;
        case VAR_EnduranceBonus:
            return character._statBonuses[ATTRIBUTE_ENDURANCE] >= pValue;
        case VAR_SpeedBonus:
            return character._statBonuses[ATTRIBUTE_SPEED] >= pValue;
        case VAR_AccuracyBonus:
            return character._statBonuses[ATTRIBUTE_ACCURACY] >= pValue;
        case VAR_LuckBonus:
            return character._statBonuses[ATTRIBUTE_LUCK] >= pValue;
        case VAR_BaseMight:
            return character._stats[ATTRIBUTE_MIGHT] >= pValue;
        case VAR_BaseIntellect:
            return character._stats[ATTRIBUTE_INTELLIGENCE] >= pValue;
        case VAR_BasePersonality:
            return character._stats[ATTRIBUTE_PERSONALITY] >= pValue;
        case VAR_BaseEndurance:
            return character._stats[ATTRIBUTE_ENDURANCE] >= pValue;
        case VAR_BaseSpeed:
            return character._stats[ATTRIBUTE_SPEED] >= pValue;
        case VAR_BaseAccuracy:
            return character._stats[ATTRIBUTE_ACCURACY] >= pValue;
        case VAR_BaseLuck:
            return character._stats[ATTRIBUTE_LUCK] >= pValue;
        case VAR_ActualMight:
            return character.GetActualMight() >= pValue;
        case VAR_ActualIntellect:
            return character.GetActualIntelligence() >= pValue;
        case VAR_ActualPersonality:
            return character.GetActualPersonality() >= pValue;
        case VAR_ActualEndurance:
            return character.GetActualEndurance() >= pValue;
        case VAR_ActualSpeed:
            return character.GetActualSpeed() >= pValue;
        case VAR_ActualAccuracy:
            return character.GetActualAccuracy() >= pValue;
        case VAR_ActualLuck:
            return character.GetActualLuck() >= pValue;
        case VAR_FireResistance:
            return character.sResFireBase >= pValue;
        case VAR_AirResistance:
            return character.sResAirBase >= pValue;
        case VAR_WaterResistance:
            return character.sResWaterBase >= pValue;
        case VAR_EarthResistance:
            return character.sResEarthBase >= pValue;
        case VAR_SpiritResistance:
            return character.sResSpiritBase >= pValue;
        case VAR_MindResistance:
            return character.sResMindBase >= pValue;
        case VAR_BodyResistance:
            return character.sResBodyBase >= pValue;
        case VAR_LightResistance:
            return character.sResLightBase >= pValue;
        case VAR_DarkResistance:
            return character.sResDarkBase >= pValue;
        case VAR_PhysicalResistance:
            MM_ERROR("Physical resistance isn't used in events");
            return false;
        case VAR_MagicResistance:
            return character.sResMagicBase >= pValue;
        case VAR_FireResistanceBonus:
            return character.sResFireBonus >= pValue;
        case VAR_AirResistanceBonus:
            return character.sResAirBonus >= pValue;
        case VAR_WaterResistanceBonus:
            return character.sResWaterBonus >= pValue;
        case VAR_EarthResistanceBonus:
            return character.sResEarthBonus >= pValue;
        case VAR_SpiritResistanceBonus:
            return character.sResSpiritBonus >= pValue;
        case VAR_MindResistanceBonus:
            return character.sResMindBonus >= pValue;
        case VAR_BodyResistanceBonus:
            return character.sResBodyBonus >= pValue;
        case VAR_LightResistanceBonus:
            return character.sResLightBonus >= pValue;
        case VAR_DarkResistanceBonus:
            return character.sResDarkBonus >= pValue;
        case VAR_MagicResistanceBonus:
            return character.sResMagicBonus >= pValue;
        case VAR_StaffSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_STAFF]);
        case VAR_SwordSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_SWORD]);
        case VAR_DaggerSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_DAGGER]);
        case VAR_AxeSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_AXE]);
        case VAR_SpearSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_SPEAR]);
        case VAR_BowSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_BOW]);
        case VAR_MaceSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_MACE]);
        case VAR_BlasterSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_BLASTER]);
        case VAR_ShieldSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_SHIELD]);
        case VAR_LeatherSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_LEATHER]);
        case VAR_SkillChain:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_CHAIN]);
        case VAR_PlateSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_PLATE]);
        case VAR_FireSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_FIRE]);
        case VAR_AirSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_AIR]);
        case VAR_WaterSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_WATER]);
        case VAR_EarthSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_EARTH]);
        case VAR_SpiritSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_SPIRIT]);
        case VAR_MindSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_MIND]);
        case VAR_BodySkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_BODY]);
        case VAR_LightSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_LIGHT]);
        case VAR_DarkSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_DARK]);
        case VAR_IdentifyItemSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_ITEM_ID]);
        case VAR_MerchantSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_MERCHANT]);
        case VAR_RepairSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_REPAIR]);
        case VAR_BodybuildingSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_BODYBUILDING]);
        case VAR_MeditationSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_MEDITATION]);
        case VAR_PerceptionSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_PERCEPTION]);
        case VAR_DiplomacySkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_DIPLOMACY]);
        case VAR_ThieverySkill:
            // Original binary had this:
            // Error("Thievery isn't used in events");
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_THIEVERY]);
        case VAR_DisarmTrapSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_TRAP_DISARM]);
        case VAR_DodgeSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_DODGE]);
        case VAR_UnarmedSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_UNARMED]);
        case VAR_IdentifyMonsterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_MONSTER_ID]);
        case VAR_ArmsmasterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_ARMSMASTER]);
        case VAR_StealingSkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_STEALING]);
        case VAR_AlchemySkill:  // wasn't in the original
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_ALCHEMY]);
        case VAR_LearningSkill:
            return CmpSkillValue(pValue, character.pActiveSkills[SKILL_LEARNING]);
        case VAR_Cursed:
            return character.conditions.has(CONDITION_CURSED);
        case VAR_Weak:
            return character.conditions.has(CONDITION_WEAK);
        case VAR_Asleep:
            return character.conditions.has(CONDITION_SLEEP);
        case VAR_Afraid:
            return character.conditions.has(CONDITION_FEAR);
        case VAR_Drunk:
            return character.conditions.has(CONDITION_DRUNK);
        case VAR_Insane:
            return character.conditions.has(CONDITION_INSANE);
        case VAR_PoisonedGreen:
            return character.conditions.has(CONDITION_POISON_WEAK);
        case VAR_DiseasedGreen:
            return character.conditions.has(CONDITION_DISEASE_WEAK);
        case VAR_PoisonedYellow:
            return character.conditions.has(CONDITION_POISON_MEDIUM);
        case VAR_DiseasedYellow:
            return character.conditions.has(CONDITION_DISEASE_MEDIUM);
        case VAR_PoisonedRed:
            return character.conditions.has(CONDITION_POISON_SEVERE);
        case VAR_DiseasedRed:
            return character.conditions.has(CONDITION_DISEASE_SEVERE);
        case VAR_Paralyzed:
            return character.conditions.has(CONDITION_PARALYZED);
        case VAR_Unconsious:
            return character.conditions.has(CONDITION_UNCONSCIOUS);
        case VAR_Dead:
            return character.conditions.has(CONDITION_DEAD);
        case VAR_Stoned:
            return character.conditions.has(CONDITION_PETRIFIED);
        case VAR_Eradicated:
            return character.conditions.has(CONDITION_ERADICATED);
        case VAR_MajorCondition: {
            Condition condition = character.GetMajorConditionIdx();
            if (condition != CONDITION_GOOD) {
                return std::to_underlying(condition) >= pValue;
            }
            return true;
        }
        case VAR_AutoNotes:
            return pParty->_autonoteBits[pValue];
        case VAR_IsMightMoreThanBase:
            actStat = character.GetActualMight();
            baseStat = character.GetBaseMight();
            return (actStat >= baseStat);
        case VAR_IsIntellectMoreThanBase:
            actStat = character.GetActualIntelligence();
            baseStat = character.GetBaseIntelligence();
            return (actStat >= baseStat);
        case VAR_IsPersonalityMoreThanBase:
            actStat = character.GetActualPersonality();
            baseStat = character.GetBasePersonality();
            return (actStat >= baseStat);
        case VAR_IsEnduranceMoreThanBase:
            actStat = character.GetActualEndurance();
            baseStat = character.GetBaseEndurance();
            return (actStat >= baseStat);
        case VAR_IsSpeedMoreThanBase:
            actStat = character.GetActualSpeed();
            baseStat = character.GetBaseSpeed();
            return (actStat >= baseStat);
        case VAR_IsAccuracyMoreThanBase:
            actStat = character.GetActualAccuracy();
            baseStat = character.GetBaseAccuracy();
            return (actStat >= baseStat);
        case VAR_IsLuckMoreThanBase:
            actStat = character.GetActualLuck();
            baseStat = character.GetBaseLuck();
            return (actStat >= baseStat);
        case VAR_PlayerBits:
            return character._characterEventBits[pValue];
        case VAR_NPCs2:
            return pNPCStats->pNPCData[pValue].Hired();
        case VAR_IsFlying:
            if (pParty->bFlying && pParty->pPartyBuffs[PARTY_BUFF_FLY].Active())
                return true;
            return false;
        case VAR_HiredNPCHasSpeciality:
            return CheckHiredNPCSpeciality((NpcProfession)pValue);
        case VAR_CircusPrizes:  // isn't used in MM6 since 0x1D6u is a book of
                                // regeneration
            v4 = 0;
            for (Character &character : pParty->pCharacters) {
                for (InventoryEntry entry : character.inventory.entries()) {
                    ItemId itemId = entry->itemId;

                    switch (itemId) {
                        case ITEM_SPELLBOOK_REGENERATION:
                            ++v4;
                            break;
                        case ITEM_SPELLBOOK_CURE_POISON:
                            v4 += 3;
                            break;
                        case ITEM_SPELLBOOK_LIGHT_BOLT:
                            v4 += 5;
                            break;
                        default:
                            break;
                    }
                }
            }
            return v4 >= pValue;
        case VAR_NumSkillPoints:
            return character.uSkillPoints >= pValue;
        case VAR_MonthIs:
            return pParty->uCurrentMonth == pValue;
        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
        {
            int idx = std::to_underlying(VarNum) - std::to_underlying(VAR_Counter1);
            if (pParty->PartyTimes.CounterEventValues[idx].isValid()) {
                return (pParty->PartyTimes.CounterEventValues[idx] + Duration::fromHours(pValue)) <= pParty->GetPlayingTime();
            }
            return false;
        }

        case VAR_ReputationInCurrentLocation:
            v19 = &currentLocationInfo();
            return (v19->reputation >= pValue);

        case VAR_AlertStatus:
            v21 = &currentLocationInfo();
            return v21->alertStatus == pValue;  // yes, equality, not >=

        case VAR_GoldInBank:
            return pParty->uNumGoldInBank >= pValue;

        case VAR_NumDeaths:
            return pParty->uNumDeaths >= pValue;

        case VAR_NumBounties:
            return pParty->uNumBountiesCollected >= pValue;

        case VAR_PrisonTerms:
            return pParty->uNumPrisonTerms >= pValue;
        case VAR_ArenaWinsPage:
            return pParty->uNumArenaWins[ARENA_LEVEL_PAGE] >= pValue;
        case VAR_ArenaWinsSquire:
            return pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] >= pValue;
        case VAR_ArenaWinsKnight:
            return pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] >= pValue;
        case VAR_ArenaWinsLord:
            return pParty->uNumArenaWins[ARENA_LEVEL_LORD] >= pValue;
        case VAR_Invisible:
            return pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active();
        case VAR_ItemEquipped:
            return character.wearsItem(static_cast<ItemId>(pValue));
        default:
            MM_ERROR("Comparing unsupported evt variable {}", std::to_underlying(VarNum));
            return false;
    }
}

//----- (0044A5CB) --------------------------------------------------------
void setEvtVariable(Character &character, EvtVariable var_type, int var_value) {
    int gold = 0, food = 0;
    LocationInfo *ddm;
    Item item;

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)]) {
            pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)] = pParty->GetPlayingTime();
            if (!pHistoryTable->historyLines[1 + historyIndex(var_type)].pText.empty()) {
                bFlashHistoryBook = true;
                character.PlayAwardSound();
            }
        }
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 && var_type <= VAR_MapPersistentVariable_74) {
        engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] = var_value;
        return;
    }

    // not really sure whether the number gets up to 99, but can't ignore the possibility
    if (var_type >= VAR_MapPersistentDecorVariable_0 && var_type <= VAR_MapPersistentDecorVariable_24) {
        engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] = var_value;
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 && var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[std::to_underlying(var_type) - std::to_underlying(VAR_UnknownTimeEvent0)] = pParty->GetPlayingTime();
        character.PlayAwardSound();
        return;
    }

    switch (var_type) {
        case VAR_Sex:
            character.uSex = (Sex)var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_Class:
            character.classType = (Class)var_value;
            if ((Class)var_value == CLASS_LICH) {
                for (InventoryEntry entry : character.inventory.entries()) {
                    if (entry->itemId == ITEM_QUEST_LICH_JAR_EMPTY) {
                        entry->itemId = ITEM_QUEST_LICH_JAR_FULL;
                        entry->lichJarCharacterIndex = character.characterIndex();
                    }
                }
                if (character.sResFireBase < 20) character.sResFireBase = 20;
                if (character.sResAirBase < 20) character.sResAirBase = 20;
                if (character.sResWaterBase < 20) character.sResWaterBase = 20;
                if (character.sResEarthBase < 20) character.sResEarthBase = 20;
                character.sResMindBase = 200;
                character.sResBodyBase = 200;
                Sex sex = character.GetSexByVoice();
                character.uPrevVoiceID = character.uVoiceID;
                character.uPrevFace = character.uCurrentFace;
                if (sex == SEX_FEMALE) {
                    character.uCurrentFace = 21;
                    character.uVoiceID = 21;
                } else {
                    character.uCurrentFace = 20;
                    character.uVoiceID = 20;
                }
                GameUI_ReloadPlayerPortraits(character.characterIndex(),
                                             character.uCurrentFace);
            }
            character.PlayAwardSound_Anim();
            return;
        case VAR_CurrentHP:
            character.health = var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_MaxHP:
            character.health = character.GetMaxHealth();
            return;
        case VAR_CurrentSP:
            character.mana = var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_MaxSP:
            character.mana = character.GetMaxMana();
            return;
        case VAR_ACModifier:
            character.sACModifier = (uint8_t)var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_BaseLevel:
            character.uLevel = (uint8_t)var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_LevelModifier:
            character.sLevelModifier = (uint8_t)var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_Age:
            character.sAgeModifier = var_value;
            return;
        case VAR_Award:
            character.giveAward(static_cast<AwardId>(var_value));
            return;
        case VAR_Experience:
            character.experience = var_value;
            character.PlayAwardSound_Anim();
            return;
        case VAR_QBits_QuestsDone:
            if (!pParty->_questBits[static_cast<QuestBit>(var_value)] && !pQuestTable[static_cast<QuestBit>(var_value)].empty()) {
                bFlashQuestBook = true;
                spell_fx_renderer->SetPlayerBuffAnim(BECOME_MAGIC_GUILD_MEMBER, character.characterIndex());
                character.PlayAwardSound();
                character.playReaction(SPEECH_QUEST_GOT);
            }
            pParty->_questBits.set(static_cast<QuestBit>(var_value));
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.itemId = ItemId(var_value);
            item.flags = ITEM_IDENTIFIED;
            pParty->setHoldingItem(item);
            if (isSpawnableArtifact(ItemId(var_value)))
                pParty->isArtifactGenerated[ItemId(var_value)] = true;
            return;
        case VAR_FixedGold:
            pParty->SetGold(var_value);
            return;
        case VAR_RandomGold:
            gold = grng->random(var_value) + 1;
            pParty->SetGold(gold);
            engine->_statusBar->setEvent(LSTR_YOU_HAVE_LU_GOLD, gold);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_FixedFood:
            pParty->SetFood(var_value);
            character.PlayAwardSound_Anim();
            return;
        case VAR_RandomFood:
            food = grng->random(var_value) + 1;
            pParty->SetFood(food);
            engine->_statusBar->setEvent(LSTR_YOU_HAVE_LU_FOOD, food);
            GameUI_DrawFoodAndGold();
            character.PlayAwardSound_Anim();
            return;
        case VAR_BaseMight:
            character._stats[ATTRIBUTE_MIGHT] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseIntellect:
            character._stats[ATTRIBUTE_INTELLIGENCE] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BasePersonality:
            character._stats[ATTRIBUTE_PERSONALITY] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseEndurance:
            character._stats[ATTRIBUTE_ENDURANCE] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseSpeed:
            character._stats[ATTRIBUTE_SPEED] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseAccuracy:
            character._stats[ATTRIBUTE_ACCURACY] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseLuck:
            character._stats[ATTRIBUTE_LUCK] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            character._statBonuses[ATTRIBUTE_MIGHT] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            character._statBonuses[ATTRIBUTE_INTELLIGENCE] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            character._statBonuses[ATTRIBUTE_PERSONALITY] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            character._statBonuses[ATTRIBUTE_ENDURANCE] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            character._statBonuses[ATTRIBUTE_SPEED] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            character._statBonuses[ATTRIBUTE_ACCURACY] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            character._statBonuses[ATTRIBUTE_LUCK] = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistance:
            character.sResFireBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistance:
            character.sResAirBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistance:
            character.sResWaterBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_EarthResistance:
            character.sResEarthBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_SpiritResistance:
            character.sResSpiritBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MindResistance:
            character.sResMindBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BodyResistance:
            character.sResBodyBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LightResistance:
            character.sResLightBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_DarkResistance:
            character.sResDarkBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MagicResistance:
            character.sResMagicBase = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistanceBonus:
            character.sResFireBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AirResistanceBonus:
            character.sResAirBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_WaterResistanceBonus:
            character.sResWaterBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EarthResistanceBonus:
            character.sResEarthBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpiritResistanceBonus:
            character.sResSpiritBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MindResistanceBonus:
            character.sResMindBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BodyResistanceBonus:
            character.sResBodyBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LightResistanceBonus:
            character.sResLightBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_DarkResistanceBonus:
            character.sResDarkBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PhysicalResistanceBonus:
            MM_ERROR("Physical res. bonus not used");
            return;
        case VAR_MagicResistanceBonus:
            character.sResMagicBonus = (uint8_t)var_value;
            character.PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_Cursed:
            character.SetCondition(CONDITION_CURSED, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Weak:
            character.SetCondition(CONDITION_WEAK, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Asleep:
            character.SetCondition(CONDITION_SLEEP, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Afraid:
            character.SetCondition(CONDITION_FEAR, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Drunk:
            character.SetCondition(CONDITION_DRUNK, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Insane:
            character.SetCondition(CONDITION_INSANE, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_PoisonedGreen:
            character.SetCondition(CONDITION_POISON_WEAK, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_DiseasedGreen:
            character.SetCondition(CONDITION_DISEASE_WEAK, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_PoisonedYellow:
            character.SetCondition(CONDITION_POISON_MEDIUM, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_DiseasedYellow:
            character.SetCondition(CONDITION_DISEASE_MEDIUM, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_PoisonedRed:
            character.SetCondition(CONDITION_POISON_SEVERE, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_DiseasedRed:
            character.SetCondition(CONDITION_DISEASE_SEVERE, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Paralyzed:
            character.SetCondition(CONDITION_PARALYZED, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Unconsious:
            character.SetCondition(CONDITION_UNCONSCIOUS, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Dead:
            character.SetCondition(CONDITION_DEAD, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Stoned:
            character.SetCondition(CONDITION_PETRIFIED, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_Eradicated:
            character.SetCondition(CONDITION_ERADICATED, 1);
            character.PlayAwardSound_Anim();
            return;
        case VAR_MajorCondition:
            character.conditions.resetAll();
            character.PlayAwardSound_Anim();
            return;
        case VAR_AutoNotes:
            character.giveAutonote(var_value);
            return;
        case VAR_PlayerBits:
            character._characterEventBits.set(var_value);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNPCData[var_value].flags |= NPC_HIRED;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            character.uSkillPoints = var_value;
            return;

        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
            pParty->PartyTimes.CounterEventValues[std::to_underlying(var_type) - std::to_underlying(VAR_Counter1)] = pParty->GetPlayingTime();
            return;

        case VAR_ReputationInCurrentLocation:
            ddm = &currentLocationInfo();
            ddm->reputation = var_value;
            if (var_value > 10000)
                ddm->reputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank = var_value;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths = var_value;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected = var_value;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms = var_value;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] = var_value;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] = var_value;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] = var_value;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] = var_value;
            return;
        case VAR_StaffSkill:
            character.pActiveSkills[SKILL_STAFF] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_SwordSkill:
            character.pActiveSkills[SKILL_SWORD] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_DaggerSkill:
            character.pActiveSkills[SKILL_DAGGER] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_AxeSkill:
            character.pActiveSkills[SKILL_AXE] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_SpearSkill:
            character.pActiveSkills[SKILL_SPEAR] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_BowSkill:
            character.pActiveSkills[SKILL_BOW] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_MaceSkill:
            character.pActiveSkills[SKILL_MACE] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_BlasterSkill:
            character.pActiveSkills[SKILL_BLASTER] = CombinedSkillValue::fromJoined(var_value);;
            character.SetSkillReaction();
            return;
        case VAR_ShieldSkill:
            character.pActiveSkills[SKILL_SHIELD] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_LeatherSkill:
            character.pActiveSkills[SKILL_LEATHER] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_SkillChain:
            character.pActiveSkills[SKILL_CHAIN] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_PlateSkill:
            character.pActiveSkills[SKILL_PLATE] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_FireSkill:
            character.pActiveSkills[SKILL_FIRE] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_AirSkill:
            character.pActiveSkills[SKILL_AIR] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_WaterSkill:
            character.pActiveSkills[SKILL_WATER] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_EarthSkill:
            character.pActiveSkills[SKILL_EARTH] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_SpiritSkill:
            character.pActiveSkills[SKILL_SPIRIT] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_MindSkill:
            character.pActiveSkills[SKILL_MIND] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_BodySkill:
            character.pActiveSkills[SKILL_BODY] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_LightSkill:
            character.pActiveSkills[SKILL_LIGHT] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_DarkSkill:
            character.pActiveSkills[SKILL_DARK] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_IdentifyItemSkill:
            character.pActiveSkills[SKILL_ITEM_ID] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_MerchantSkill:
            character.pActiveSkills[SKILL_MERCHANT] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_RepairSkill:
            character.pActiveSkills[SKILL_REPAIR] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_BodybuildingSkill:
            character.pActiveSkills[SKILL_BODYBUILDING] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_MeditationSkill:
            character.pActiveSkills[SKILL_MEDITATION] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_PerceptionSkill:
            character.pActiveSkills[SKILL_PERCEPTION] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_DiplomacySkill:
            character.pActiveSkills[SKILL_DIPLOMACY] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_ThieverySkill:
            MM_ERROR("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            character.pActiveSkills[SKILL_TRAP_DISARM] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_DodgeSkill:
            character.pActiveSkills[SKILL_DODGE] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_UnarmedSkill:
            character.pActiveSkills[SKILL_UNARMED] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_IdentifyMonsterSkill:
            character.pActiveSkills[SKILL_MONSTER_ID] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_ArmsmasterSkill:
            character.pActiveSkills[SKILL_ARMSMASTER] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_StealingSkill:
            character.pActiveSkills[SKILL_STEALING] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_AlchemySkill:
            character.pActiveSkills[SKILL_ALCHEMY] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        case VAR_LearningSkill:
            character.pActiveSkills[SKILL_LEARNING] = CombinedSkillValue::fromJoined(var_value);
            character.SetSkillReaction();
            return;
        default:
            MM_ERROR("Setting unsupported evt variable {}", std::to_underlying(var_type));
            return;
    }
}

//----- (0044AFFB) --------------------------------------------------------
void addEvtVariable(Character &character, EvtVariable var_type, signed int val) {
    int food = 0;
    LocationInfo *ddm;
    Item item;

    if (var_type >= VAR_Counter1 && var_type <= VAR_Counter10) {
        pParty->PartyTimes.CounterEventValues[std::to_underlying(var_type) - std::to_underlying(VAR_Counter1)] = pParty->GetPlayingTime();
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 && var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[std::to_underlying(var_type) - std::to_underlying(VAR_UnknownTimeEvent0)] = pParty->GetPlayingTime();
        character.PlayAwardSound();
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 && var_type <= VAR_MapPersistentVariable_74) {
        if (255 - val > engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)]) {
            engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] += val;
        } else {
            engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] = 255;
        }
        return;
    }
    if (var_type >= VAR_MapPersistentDecorVariable_0 && var_type <= VAR_MapPersistentDecorVariable_24) {
        if (255 - val > engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)]) {
            engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] += val;
        } else {
            engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] = 255;
        }
        return;
    }

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)]) {
            pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)] = pParty->GetPlayingTime();
            if (!pHistoryTable->historyLines[1 + historyIndex(var_type)].pText.empty()) {
                bFlashHistoryBook = true;
                character.PlayAwardSound();
            }
        }
        return;
    }

    switch (var_type) {
        case VAR_RandomGold:
            pParty->partyFindsGold(grng->random(val) + 1, GOLD_RECEIVE_NOSHARE_MSG);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_RandomFood:
            food = grng->random(val) + 1;
            pParty->GiveFood(food);
            engine->_statusBar->setEvent(LSTR_YOU_FIND_LU_FOOD, food);
            GameUI_DrawFoodAndGold();
            character.PlayAwardSound();
            return;
        case VAR_Sex:
            character.uSex = (Sex)val;
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Class:
            character.classType = (Class)val;
            character.PlayAwardSound_Anim97();
            return;
        case VAR_CurrentHP:
            character.health = std::min(character.health + val, character.GetMaxHealth());
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MaxHP:
            character._health_related = 0;
            character.uFullHealthBonus = 0;
            character.health = character.GetMaxHealth();
            return;
        case VAR_CurrentSP:
            character.mana = std::min(character.mana + val, character.GetMaxMana());
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MaxSP:
            character._mana_related = 0;
            character.uFullManaBonus = 0;
            character.mana = character.GetMaxMana();
            return;
        case VAR_ACModifier:
            character.sACModifier = std::min(character.sACModifier + val, 255);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_BaseLevel:
            character.uLevel = std::min(character.uLevel + val, 255);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_LevelModifier:
            character.sLevelModifier = std::min(character.sLevelModifier + val, 255);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Age:
            character.sAgeModifier += val;
            return;
        case VAR_Award:
            if (character._achievedAwardsBits[static_cast<AwardId>(val)] && !pAwards[static_cast<AwardId>(val)].pText.empty()) {
                character.PlayAwardSound_Anim97_Face(SPEECH_AWARD_GOT);
            }
            character._achievedAwardsBits.set(static_cast<AwardId>(val));
            return;
        case VAR_Experience:
            character.experience = std::min((uint64_t)(character.experience + val), UINT64_C(4000000000));
            character.PlayAwardSound_Anim97();
            return;
        case VAR_QBits_QuestsDone:
            if (!pParty->_questBits[static_cast<QuestBit>(val)] && !pQuestTable[static_cast<QuestBit>(val)].empty()) {
                bFlashQuestBook = true;
                character.PlayAwardSound_Anim97_Face(SPEECH_QUEST_GOT);
            }
            pParty->_questBits.set(static_cast<QuestBit>(val));
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.flags = ITEM_IDENTIFIED;
            item.itemId = ItemId(val);
            item.postGenerate(ITEM_SOURCE_SCRIPT);

            if (isSpawnableArtifact(ItemId(val)))
                pParty->isArtifactGenerated[ItemId(val)] = true;
            pParty->setHoldingItem(item);
            return;
        case VAR_FixedGold:
            pParty->partyFindsGold(val, GOLD_RECEIVE_NOSHARE_MSG);
            return;
        case VAR_BaseMight:
            character._stats[ATTRIBUTE_MIGHT] = std::min(character._stats[ATTRIBUTE_MIGHT] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseIntellect:
            character._stats[ATTRIBUTE_INTELLIGENCE] = std::min(character._stats[ATTRIBUTE_INTELLIGENCE] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BasePersonality:
            character._stats[ATTRIBUTE_PERSONALITY] = std::min(character._stats[ATTRIBUTE_PERSONALITY] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseEndurance:
            character._stats[ATTRIBUTE_ENDURANCE] = std::min(character._stats[ATTRIBUTE_ENDURANCE] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseSpeed:
            character._stats[ATTRIBUTE_SPEED] = std::min(character._stats[ATTRIBUTE_SPEED] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseAccuracy:
            character._stats[ATTRIBUTE_ACCURACY] = std::min(character._stats[ATTRIBUTE_ACCURACY] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseLuck:
            character._stats[ATTRIBUTE_LUCK] = std::min(character._stats[ATTRIBUTE_LUCK] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FixedFood:
            pParty->GiveFood(val);
            engine->_statusBar->setEvent(LSTR_YOU_FIND_LU_FOOD, val);
            character.PlayAwardSound();
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            character._statBonuses[ATTRIBUTE_MIGHT] = std::min(character._statBonuses[ATTRIBUTE_MIGHT] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            character._statBonuses[ATTRIBUTE_INTELLIGENCE] = std::min(character._statBonuses[ATTRIBUTE_INTELLIGENCE] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            character._statBonuses[ATTRIBUTE_PERSONALITY] = std::min(character._statBonuses[ATTRIBUTE_PERSONALITY] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            character._statBonuses[ATTRIBUTE_ENDURANCE] = std::min(character._statBonuses[ATTRIBUTE_ENDURANCE] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            character._statBonuses[ATTRIBUTE_SPEED] = std::min(character._statBonuses[ATTRIBUTE_SPEED] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            character._statBonuses[ATTRIBUTE_ACCURACY] = std::min(character._statBonuses[ATTRIBUTE_ACCURACY] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            character._statBonuses[ATTRIBUTE_LUCK] = std::min(character._statBonuses[ATTRIBUTE_LUCK] + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_FireResistance:
            character.sResFireBase = std::min(character.sResFireBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistance:
            character.sResAirBase = std::min(character.sResAirBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistance:
            character.sResWaterBase = std::min(character.sResWaterBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_EarthResistance:
            character.sResEarthBase = std::min(character.sResEarthBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_SpiritResistance:
            character.sResSpiritBase = std::min(character.sResSpiritBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MindResistance:
            character.sResMindBase = std::min(character.sResMindBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BodyResistance:
            character.sResBodyBase = std::min(character.sResBodyBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LightResistance:
            character.sResLightBase = std::min(character.sResLightBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_DarkResistance:
            character.sResDarkBase = std::min(character.sResDarkBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MagicResistance:
            character.sResMagicBase = std::min(character.sResMagicBase + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistanceBonus:
            character.sResFireBonus = std::min(character.sResFireBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AirResistanceBonus:
            character.sResAirBonus = std::min(character.sResAirBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_WaterResistanceBonus:
            character.sResWaterBonus = std::min(character.sResWaterBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EarthResistanceBonus:
            character.sResEarthBonus = std::min(character.sResEarthBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpiritResistanceBonus:
            character.sResSpiritBonus = std::min(character.sResSpiritBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MindResistanceBonus:
            character.sResMindBonus = std::min(character.sResMindBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BodyResistanceBonus:
            character.sResBodyBonus = std::min(character.sResBodyBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LightResistanceBonus:
            character.sResLightBonus = std::min(character.sResLightBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_DarkResistanceBonus:
            character.sResDarkBonus = std::min(character.sResDarkBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MagicResistanceBonus:
            character.sResMagicBonus = std::min(character.sResMagicBonus + val, 255);
            character.PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_Cursed:
            character.SetCondition(CONDITION_CURSED, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Weak:
            character.SetCondition(CONDITION_WEAK, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Asleep:
            character.SetCondition(CONDITION_SLEEP, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Afraid:
            character.SetCondition(CONDITION_FEAR, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Drunk:
            character.SetCondition(CONDITION_DRUNK, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Insane:
            character.SetCondition(CONDITION_INSANE, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedGreen:
            character.SetCondition(CONDITION_POISON_WEAK, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedGreen:
            character.SetCondition(CONDITION_DISEASE_WEAK, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedYellow:
            character.SetCondition(CONDITION_POISON_MEDIUM, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedYellow:
            character.SetCondition(CONDITION_DISEASE_MEDIUM, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedRed:
            character.SetCondition(CONDITION_POISON_SEVERE, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedRed:
            character.SetCondition(CONDITION_DISEASE_SEVERE, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Paralyzed:
            character.SetCondition(CONDITION_PARALYZED, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Unconsious:
            character.SetCondition(CONDITION_UNCONSCIOUS, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Dead:
            character.SetCondition(CONDITION_DEAD, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Stoned:
            character.SetCondition(CONDITION_PETRIFIED, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_Eradicated:
            character.SetCondition(CONDITION_ERADICATED, 1);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MajorCondition:
            character.conditions.resetAll();
            character.PlayAwardSound_Anim97();
            return;
        case VAR_AutoNotes:
            if (!pParty->_autonoteBits[val] && !pAutonoteTxt[val].pText.empty()) {
                character.playReaction(SPEECH_AWARD_GOT);
                bFlashAutonotesBook = true;
                autonoteBookDisplayType = pAutonoteTxt[val].eType;
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, character.characterIndex());
            }
            pParty->_autonoteBits.set(val);
            character.PlayAwardSound();
            return;
        case VAR_PlayerBits:
            character._characterEventBits.set(val);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNPCData[val].flags |= NPC_HIRED;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            character.uSkillPoints += val;
            return;
        case VAR_ReputationInCurrentLocation:
            ddm = &currentLocationInfo();
            ddm->reputation += val;
            if (ddm->reputation > 10000)
                ddm->reputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank += val;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths += val;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected += val;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms += val;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] += val;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] += val;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] += val;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] += val;
            return;
        case VAR_StaffSkill:
            character.AddSkillByEvent(SKILL_STAFF, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_SwordSkill:
            character.AddSkillByEvent(SKILL_SWORD, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DaggerSkill:
            character.AddSkillByEvent(SKILL_DAGGER, val);;
            character.PlayAwardSound_Anim97();
            return;
        case VAR_AxeSkill:
            character.AddSkillByEvent(SKILL_AXE, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_SpearSkill:
            character.AddSkillByEvent(SKILL_SPEAR, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_BowSkill:
            character.AddSkillByEvent(SKILL_BOW, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MaceSkill:
            character.AddSkillByEvent(SKILL_MACE, val);;
            character.PlayAwardSound_Anim97();
            return;
        case VAR_BlasterSkill:
            character.AddSkillByEvent(SKILL_BLASTER, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_ShieldSkill:
            character.AddSkillByEvent(SKILL_SHIELD, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_LeatherSkill:
            character.AddSkillByEvent(SKILL_LEATHER, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_SkillChain:
            character.AddSkillByEvent(SKILL_CHAIN, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_PlateSkill:
            character.AddSkillByEvent(SKILL_PLATE, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_FireSkill:
            character.AddSkillByEvent(SKILL_FIRE, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_AirSkill:
            character.AddSkillByEvent(SKILL_AIR, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_WaterSkill:
            character.AddSkillByEvent(SKILL_WATER, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_EarthSkill:
            character.AddSkillByEvent(SKILL_EARTH, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_SpiritSkill:
            character.AddSkillByEvent(SKILL_SPIRIT, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MindSkill:
            character.AddSkillByEvent(SKILL_MIND, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_BodySkill:
            character.AddSkillByEvent(SKILL_BODY, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_LightSkill:
            character.AddSkillByEvent(SKILL_LIGHT, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DarkSkill:
            character.AddSkillByEvent(SKILL_DARK, val);;
            character.PlayAwardSound_Anim97();
            return;
        case VAR_IdentifyItemSkill:
            character.AddSkillByEvent(SKILL_ITEM_ID, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MerchantSkill:
            character.AddSkillByEvent(SKILL_MERCHANT, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_RepairSkill:
            character.AddSkillByEvent(SKILL_REPAIR, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_BodybuildingSkill:
            character.AddSkillByEvent(SKILL_BODYBUILDING, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_MeditationSkill:
            character.AddSkillByEvent(SKILL_MEDITATION, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_PerceptionSkill:
            character.AddSkillByEvent(SKILL_PERCEPTION, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DiplomacySkill:
            character.AddSkillByEvent(SKILL_DIPLOMACY, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_ThieverySkill:
            MM_ERROR("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            character.AddSkillByEvent(SKILL_TRAP_DISARM, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_DodgeSkill:
            character.AddSkillByEvent(SKILL_DODGE, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_UnarmedSkill:
            character.AddSkillByEvent(SKILL_UNARMED, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_IdentifyMonsterSkill:
            character.AddSkillByEvent(SKILL_MONSTER_ID, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_ArmsmasterSkill:
            character.AddSkillByEvent(SKILL_ARMSMASTER, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_StealingSkill:
            character.AddSkillByEvent(SKILL_STEALING, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_AlchemySkill:
            character.AddSkillByEvent(SKILL_ALCHEMY, val);
            character.PlayAwardSound_Anim97();
            return;
        case VAR_LearningSkill:
            character.AddSkillByEvent(SKILL_LEARNING, val);
            character.PlayAwardSound_Anim97();
            return;
        default:
            MM_ERROR("Adding unsupported evt variable {}", std::to_underlying(var_type));
            return;
    }
}

//----- (0044B9C4) --------------------------------------------------------
bool subtractEvtVariable(Character &character, EvtVariable VarNum, signed int pValue) {
    LocationInfo *locationHeader;  // eax@90
    int randGold;
    int randFood;

    if (VarNum >= VAR_MapPersistentVariable_0 && VarNum <= VAR_MapPersistentVariable_74) {
        engine->_persistentVariables.mapVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentVariable_0)] -= pValue;
        return true;
    }
    if (VarNum >= VAR_MapPersistentDecorVariable_0 && VarNum <= VAR_MapPersistentDecorVariable_24) {
        engine->_persistentVariables.decorVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] -= pValue;
        return true;
    }

    switch (VarNum) {
        case VAR_CurrentHP:
            character.receiveDamage((signed int)pValue, DAMAGE_PHYSICAL);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_CurrentSP:
            character.mana = std::max(character.mana - pValue, 0);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_ACModifier:
            character.sACModifier -= (uint8_t)pValue;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_BaseLevel:
            character.uLevel -= (uint8_t)pValue;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_LevelModifier:
            character.sLevelModifier -= (uint8_t)pValue;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Age:
            character.sAgeModifier -= (int16_t)pValue;
            return true;
        case VAR_Award:
            character._achievedAwardsBits.reset(static_cast<AwardId>(pValue));
            return true;
        case VAR_Experience:
            character.experience -= pValue;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_QBits_QuestsDone:
            pParty->_questBits.reset(static_cast<QuestBit>(pValue));
            character.playReaction(SPEECH_AWARD_GOT);
            return true;
        case VAR_PlayerItemInHands:
            for (InventoryEntry entry : character.inventory.entries()) {
                if (entry->itemId == static_cast<ItemId>(pValue)) {
                    character.inventory.take(entry);
                    return true;
                }
            }
            if (pParty->pPickedItem.itemId == static_cast<ItemId>(pValue)) {
                pParty->takeHoldingItem();
                return true;
            }
            return true;
        case VAR_FixedGold:
            if (pValue > pParty->GetGold())
                return false;
            pParty->TakeGold(pValue);
            return true;
        case VAR_RandomGold:
            randGold = grng->random(pValue) + 1;
            if (randGold > pParty->GetGold())
                randGold = pParty->GetGold();
            pParty->TakeGold(randGold);
            engine->_statusBar->setEvent(LSTR_YOU_LOSE_LU_GOLD, randGold);
            GameUI_DrawFoodAndGold();
            return true;
        case VAR_FixedFood:
            pParty->TakeFood(pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_RandomFood:
            randFood = grng->random(pValue) + 1;
            if (randFood > pParty->GetFood())
                randFood = pParty->GetFood();
            pParty->TakeFood(randFood);
            engine->_statusBar->setEvent(LSTR_YOU_LOSE_LU_FOOD, randFood);
            GameUI_DrawFoodAndGold();
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_MightBonus:
        case VAR_ActualMight:
            character._statBonuses[ATTRIBUTE_MIGHT] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            character._statBonuses[ATTRIBUTE_INTELLIGENCE] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            character._statBonuses[ATTRIBUTE_PERSONALITY] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            character._statBonuses[ATTRIBUTE_ENDURANCE] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            character._statBonuses[ATTRIBUTE_SPEED] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            character._statBonuses[ATTRIBUTE_ACCURACY] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            character._statBonuses[ATTRIBUTE_LUCK] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_BaseMight:
            character._stats[ATTRIBUTE_MIGHT] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BaseIntellect:
            character._stats[ATTRIBUTE_INTELLIGENCE] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BasePersonality:
            character._stats[ATTRIBUTE_PERSONALITY] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BaseEndurance:
            character._stats[ATTRIBUTE_ENDURANCE] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BaseSpeed:
            character._stats[ATTRIBUTE_SPEED] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BaseAccuracy:
            character._stats[ATTRIBUTE_ACCURACY] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BaseLuck:
            character._stats[ATTRIBUTE_LUCK] -= (uint16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_FireResistance:
            character.sResFireBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_AirResistance:
            character.sResAirBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_WaterResistance:
            character.sResWaterBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_EarthResistance:
            character.sResEarthBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_SpiritResistance:
            character.sResSpiritBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_MindResistance:
            character.sResMindBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_BodyResistance:
            character.sResBodyBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_LightResistance:
            character.sResLightBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_DarkResistance:
            character.sResDarkBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_MagicResistance:
            character.sResMagicBase -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_FireResistanceBonus:
            character.sResFireBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_AirResistanceBonus:
            character.sResAirBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return true;
        case VAR_WaterResistanceBonus:
            character.sResWaterBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_EarthResistanceBonus:
            character.sResEarthBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_SpiritResistanceBonus:
            character.sResSpiritBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_MindResistanceBonus:
            character.sResMindBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_BodyResistanceBonus:
            character.sResBodyBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_LightResistanceBonus:
            character.sResLightBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_DarkResistanceBonus:
            character.sResDarkBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_MagicResistanceBonus:
            character.sResMagicBonus -= (int16_t)pValue;
            character.PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return true;
        case VAR_StaffSkill:
            character.SubtractSkillByEvent(SKILL_STAFF, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_SwordSkill:
            character.SubtractSkillByEvent(SKILL_SWORD, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DaggerSkill:
            character.SubtractSkillByEvent(SKILL_DAGGER, pValue);;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_AxeSkill:
            character.SubtractSkillByEvent(SKILL_AXE, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_SpearSkill:
            character.SubtractSkillByEvent(SKILL_BOW, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_BowSkill:
            character.SubtractSkillByEvent(SKILL_BOW, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_MaceSkill:
            character.SubtractSkillByEvent(SKILL_MACE, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_BlasterSkill:
            character.SubtractSkillByEvent(SKILL_BLASTER, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_ShieldSkill:
            character.SubtractSkillByEvent(SKILL_SHIELD, pValue);;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_LeatherSkill:
            character.SubtractSkillByEvent(SKILL_LEATHER, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_SkillChain:
            character.SubtractSkillByEvent(SKILL_CHAIN, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_PlateSkill:
            character.SubtractSkillByEvent(SKILL_PLATE, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_FireSkill:
            character.SubtractSkillByEvent(SKILL_FIRE, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_AirSkill:
            character.SubtractSkillByEvent(SKILL_AIR, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_WaterSkill:
            character.SubtractSkillByEvent(SKILL_WATER, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_EarthSkill:
            character.SubtractSkillByEvent(SKILL_EARTH, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_SpiritSkill:
            character.SubtractSkillByEvent(SKILL_SPIRIT, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_MindSkill:
            character.SubtractSkillByEvent(SKILL_MIND, pValue);;
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_BodySkill:
            character.SubtractSkillByEvent(SKILL_BODY, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_LightSkill:
            character.SubtractSkillByEvent(SKILL_LIGHT, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DarkSkill:
            character.SubtractSkillByEvent(SKILL_DARK, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_IdentifyItemSkill:
            character.SubtractSkillByEvent(SKILL_ITEM_ID, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_MerchantSkill:
            character.SubtractSkillByEvent(SKILL_MERCHANT, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_RepairSkill:
            character.SubtractSkillByEvent(SKILL_REPAIR, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_BodybuildingSkill:
            character.SubtractSkillByEvent(SKILL_BODYBUILDING, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_MeditationSkill:
            character.SubtractSkillByEvent(SKILL_MEDITATION, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_PerceptionSkill:
            character.SubtractSkillByEvent(SKILL_PERCEPTION, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DiplomacySkill:
            character.SubtractSkillByEvent(SKILL_DIPLOMACY, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_ThieverySkill:
            MM_ERROR("Thieving unsupported");
            return true;
        case VAR_DisarmTrapSkill:
            character.SubtractSkillByEvent(SKILL_TRAP_DISARM, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DodgeSkill:
            character.SubtractSkillByEvent(SKILL_DODGE, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_UnarmedSkill:
            character.SubtractSkillByEvent(SKILL_UNARMED, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_IdentifyMonsterSkill:
            character.SubtractSkillByEvent(SKILL_MONSTER_ID, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_ArmsmasterSkill:
            character.SubtractSkillByEvent(SKILL_ARMSMASTER, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_StealingSkill:
            character.SubtractSkillByEvent(SKILL_STEALING, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_AlchemySkill:
            character.SubtractSkillByEvent(SKILL_ALCHEMY, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_LearningSkill:
            character.SubtractSkillByEvent(SKILL_LEARNING, pValue);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Cursed:
            character.conditions.reset(CONDITION_CURSED);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Weak:
            character.conditions.reset(CONDITION_WEAK);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Asleep:
            character.conditions.reset(CONDITION_SLEEP);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Afraid:
            character.conditions.reset(CONDITION_FEAR);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Drunk:
            character.conditions.reset(CONDITION_DRUNK);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Insane:
            character.conditions.reset(CONDITION_INSANE);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_PoisonedGreen:
            character.conditions.reset(CONDITION_POISON_WEAK);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DiseasedGreen:
            character.conditions.reset(CONDITION_DISEASE_WEAK);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_PoisonedYellow:
            character.conditions.reset(CONDITION_POISON_MEDIUM);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DiseasedYellow:
            character.conditions.reset(CONDITION_DISEASE_MEDIUM);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_PoisonedRed:
            character.conditions.reset(CONDITION_POISON_SEVERE);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_DiseasedRed:
            character.conditions.reset(CONDITION_DISEASE_SEVERE);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Paralyzed:
            character.conditions.reset(CONDITION_PARALYZED);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Unconsious:
            character.conditions.reset(CONDITION_UNCONSCIOUS);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Dead:
            character.conditions.reset(CONDITION_DEAD);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Stoned:
            character.conditions.reset(CONDITION_PETRIFIED);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_Eradicated:
            character.conditions.reset(CONDITION_ERADICATED);
            character.PlayAwardSound_AnimSubtract();
            return true;
        case VAR_AutoNotes:
            // TODO(Nik-RE-dev): decreasing 1 seems wrong, also bits indexing was changed
            assert(false);
            //pParty->_autonoteBits.reset(pValue - 1);
            return true;
        case VAR_PlayerBits:
            character._characterEventBits.reset(pValue);
            return true;
        case VAR_NPCs2:
            if (getNPCType(speakingNpcId) == NPC_TYPE_QUEST && speakingNpcId == pValue) {
                npcIdToDismissAfterDialogue = pValue;
            } else {
                npcIdToDismissAfterDialogue = 0;
                pParty->hirelingScrollPosition = 0;
                pNPCStats->pNPCData[(int)pValue].flags &= ~NPC_HIRED;
                pParty->CountHirelings();
            }
            return true;
        case VAR_HiredNPCHasSpeciality:
            for (int i = 0; i < pNPCStats->uNumNewNPCs; i++) {
                if (pNPCStats->pNPCData[i].profession == (NpcProfession)pValue) {
                    pNPCStats->pNPCData[(int)pValue].flags &= ~NPC_HIRED;
                }
            }
            if (pParty->pHirelings[0].profession == (NpcProfession)pValue) {
                pParty->pHirelings[0] = NPCData();
            }
            if (pParty->pHirelings[1].profession == (NpcProfession)pValue) {
                pParty->pHirelings[1] = NPCData();
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            return true;
        case VAR_NumSkillPoints:
            if (pValue <= character.uSkillPoints) {
                character.uSkillPoints -= pValue;
            } else {
                character.uSkillPoints = 0;
            }
            return true;
        case VAR_ReputationInCurrentLocation:
            locationHeader = &currentLocationInfo();
            locationHeader->reputation -= pValue;
            if (locationHeader->reputation < -10000)
                locationHeader->reputation = -10000;
            return true;
        case VAR_GoldInBank:
            if (pValue > pParty->uNumGoldInBank)
                return false;
            pParty->uNumGoldInBank -= pValue;
            return true;
        case VAR_NumDeaths:
            pParty->uNumDeaths -= pValue;
            return true;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected -= pValue;
            return true;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms -= pValue;
            return true;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] -= pValue;
            return true;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] -= pValue;
            return true;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] -= pValue;
            return true;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] -= pValue;
            return true;
        default:
            MM_ERROR("Subtracting unsupported evt variable {}", std::to_underlying(VarNum));
            return true;
    }
}
