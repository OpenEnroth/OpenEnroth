#include <string>
#include <map>
#include <utility>
#include <vector>

#include <magic_enum.hpp> // TODO: temporary

#include "Engine/Events/EventPrinter.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Engine.h"
#include "Engine/mm7_data.h"

static std::string getVariableSetStr(VariableType type, int value) {
    if (type >= VAR_MapPersistentVariable_0 && type <= VAR_MapPersistentVariable_74) {
        return fmt::format("MapVars[{}], {}", type - VAR_MapPersistentVariable_0, value);
    }

    if (type >= VAR_MapPersistentVariable_75 && type <= VAR_MapPersistentVariable_99) {
        return fmt::format("MapVarsDecor[{}], {}", type - VAR_MapPersistentVariable_75, value);
    }

    if (type >= VAR_Counter1 && type <= VAR_Counter10) {
        return fmt::format("Counter[{}], PlayingTime", type - VAR_Counter1);
    }

    if (type >= VAR_UnknownTimeEvent0 && type <= VAR_UnknownTimeEvent19) {
        return fmt::format("UnkTimeEvent[{}], PlayingTime", type - VAR_UnknownTimeEvent0);
    }

    if (type >= VAR_History_0 && type <= VAR_History_28) {
        return fmt::format("History[{}]", type - VAR_History_0);
    }

    switch (type) {
        case VAR_Sex:
            return fmt::format("Sex, {}", value);
        case VAR_Class:
            return fmt::format("Class, {}", value);
        case VAR_CurrentHP:
            return fmt::format("HP, {}", value);
        case VAR_MaxHP:
            return fmt::format("HP, MaxHP");
        case VAR_CurrentSP:
            return fmt::format("SP, {}", value);
        case VAR_MaxSP:
            return fmt::format("SP, MaxSP");
        case VAR_ActualAC:
            return fmt::format("ERROR: AC, {}", value);
        case VAR_ACModifier:
            return fmt::format("ACMod, {}", value);
        case VAR_BaseLevel:
            return fmt::format("Level, {}", value);
        case VAR_LevelModifier:
            return fmt::format("LevelMod, {}", value);
        case VAR_Age:
            return fmt::format("Age, {}", value);
        case VAR_Award:
            return fmt::format("Awards[{}]", value);
        case VAR_Experience:
            return fmt::format("Exp, {}", value);
        case VAR_Race:
            return fmt::format("ERROR: Race, {}", value);
        case VAR_QBits_QuestsDone:
            return fmt::format("QBits[{}]", value);
        case VAR_PlayerItemInHands:
            return fmt::format("ItemInHand, {}", value);
        case VAR_Hour:
            return fmt::format("ERROR: HourOfDay, {}", value);
        case VAR_DayOfYear:
            return fmt::format("ERROR: DayOfYear, {}", value);
        case VAR_DayOfWeek:
            return fmt::format("ERROR: DayOfWeek, {}", value);
        case VAR_FixedGold:
            return fmt::format("Gold, {}", value);
        case VAR_RandomGold:
            return fmt::format("Gold, Random({})", value);
        case VAR_FixedFood:
            return fmt::format("Food, {}", value);
        case VAR_RandomFood:
            return fmt::format("Food, Random({})", value);
        case VAR_MightBonus:
            return fmt::format("Bonus(Might), {}", value);
        case VAR_IntellectBonus:
            return fmt::format("Bonus(Intellect), {}", value);
        case VAR_PersonalityBonus:
            return fmt::format("Bonus(Personality), {}", value);
        case VAR_EnduranceBonus:
            return fmt::format("Bonus(Endurance), {}", value);
        case VAR_SpeedBonus:
            return fmt::format("Bonus(Speed), {}", value);
        case VAR_AccuracyBonus:
            return fmt::format("Bonus(Accuracy), {}", value);
        case VAR_LuckBonus:
            return fmt::format("Bonus(Luck), {}", value);
        case VAR_BaseMight:
            return fmt::format("Attibute(Might), {}", value);
        case VAR_BaseIntellect:
            return fmt::format("Attibute(Intellect), {}", value);
        case VAR_BasePersonality:
            return fmt::format("Attibute(Personality), {}", value);
        case VAR_BaseEndurance:
            return fmt::format("Attibute(Endurance), {}", value);
        case VAR_BaseSpeed:
            return fmt::format("Attibute(Speed), {}", value);
        case VAR_BaseAccuracy:
            return fmt::format("Attibute(Accuracy), {}", value);
        case VAR_BaseLuck:
            return fmt::format("Attibute(Luck), {}", value);
        case VAR_ActualMight:
            return fmt::format("Bonus*(Might), {}", value);
        case VAR_ActualIntellect:
            return fmt::format("Bonus*(Intellect), {}", value);
        case VAR_ActualPersonality:
            return fmt::format("Bonus*(Personality), {}", value);
        case VAR_ActualEndurance:
            return fmt::format("Bonus*(Endurance), {}", value);
        case VAR_ActualSpeed:
            return fmt::format("Bonus*(Speed), {}", value);
        case VAR_ActualAccuracy:
            return fmt::format("Bonus*(Accuracy), {}", value);
        case VAR_ActualLuck:
            return fmt::format("Bonus*(Luck), {}", value);
        case VAR_FireResistance:
            return fmt::format("Resist(Fire), {}", value);
        case VAR_AirResistance:
            return fmt::format("Resist(Air), {}", value);
        case VAR_WaterResistance:
            return fmt::format("Resist(Water), {}", value);
        case VAR_EarthResistance:
            return fmt::format("Resist(Earth), {}", value);
        case VAR_SpiritResistance:
            return fmt::format("Resist(Spirit), {}", value);
        case VAR_MindResistance:
            return fmt::format("Resist(Mind), {}", value);
        case VAR_BodyResistance:
            return fmt::format("Resist(Body), {}", value);
        case VAR_LightResistance:
            return fmt::format("Resist(Light), {}", value);
        case VAR_DarkResistance:
            return fmt::format("Resist(Dark), {}", value);
        case VAR_PhysicalResistance:
            return fmt::format("Resist(Phys), {}", value);
        case VAR_MagicResistance:
            return fmt::format("Resist(Magic), {}", value);
        case VAR_FireResistanceBonus:
            return fmt::format("BonusResist(Fire), {}", value);
        case VAR_AirResistanceBonus:
            return fmt::format("BonusResist(Air), {}", value);
        case VAR_WaterResistanceBonus:
            return fmt::format("BonusResist(Water), {}", value);
        case VAR_EarthResistanceBonus:
            return fmt::format("BonusResist(Earth), {}", value);
        case VAR_SpiritResistanceBonus:
            return fmt::format("BonusResist(Spirit), {}", value);
        case VAR_MindResistanceBonus:
            return fmt::format("BonusResist(Mind), {}", value);
        case VAR_BodyResistanceBonus:
            return fmt::format("BonusResist(Body), {}", value);
        case VAR_LightResistanceBonus:
            return fmt::format("BonusResist(Light), {}", value);
        case VAR_DarkResistanceBonus:
            return fmt::format("BonusResist(Dark), {}", value);
        case VAR_PhysicalResistanceBonus:
            return fmt::format("BonusResist(Phys), {}", value);
        case VAR_MagicResistanceBonus:
            return fmt::format("BonusResist(Magic), {}", value);
        case VAR_StaffSkill:
            return fmt::format("Skill(Staff), {}", value);
        case VAR_SwordSkill:
            return fmt::format("Skill(Sword), {}", value);
        case VAR_DaggerSkill:
            return fmt::format("Skill(Dagger), {}", value);
        case VAR_AxeSkill:
            return fmt::format("Skill(Axe), {}", value);
        case VAR_SpearSkill:
            return fmt::format("Skill(Spear), {}", value);
        case VAR_BowSkill:
            return fmt::format("Skill(Bow), {}", value);
        case VAR_MaceSkill:
            return fmt::format("Skill(Mace), {}", value);
        case VAR_BlasterSkill:
            return fmt::format("Skill(Blaster), {}", value);
        case VAR_ShieldSkill:
            return fmt::format("Skill(Shield), {}", value);
        case VAR_LeatherSkill:
            return fmt::format("Skill(Leather), {}", value);
        case VAR_SkillChain:
            return fmt::format("Skill(Chain), {}", value);
        case VAR_PlateSkill:
            return fmt::format("Skill(Plate), {}", value);
        case VAR_FireSkill:
            return fmt::format("Skill(Fire), {}", value);
        case VAR_AirSkill:
            return fmt::format("Skill(Air), {}", value);
        case VAR_WaterSkill:
            return fmt::format("Skill(Water), {}", value);
        case VAR_EarthSkill:
            return fmt::format("Skill(Earth), {}", value);
        case VAR_SpiritSkill:
            return fmt::format("Skill(Spirit), {}", value);
        case VAR_MindSkill:
            return fmt::format("Skill(Mind), {}", value);
        case VAR_BodySkill:
            return fmt::format("Skill(Body), {}", value);
        case VAR_LightSkill:
            return fmt::format("Skill(Light), {}", value);
        case VAR_DarkSkill:
            return fmt::format("Skill(Dark), {}", value);
        case VAR_IdentifyItemSkill:
            return fmt::format("Skill(IdentifyItem), {}", value);
        case VAR_MerchantSkill:
            return fmt::format("Skill(Merchant), {}", value);
        case VAR_RepairSkill:
            return fmt::format("Skill(Repair), {}", value);
        case VAR_BodybuildingSkill:
            return fmt::format("Skill(Bodybuilding), {}", value);
        case VAR_MeditationSkill:
            return fmt::format("Skill(Meditation), {}", value);
        case VAR_PerceptionSkill:
            return fmt::format("Skill(Perception), {}", value);
        case VAR_DiplomacySkill:
            return fmt::format("Skill(Diplomacy), {}", value);
        case VAR_ThieverySkill:
            return fmt::format("Skill(Thievery), {}", value);
        case VAR_DisarmTrapSkill:
            return fmt::format("Skill(DisarmTrap), {}", value);
        case VAR_DodgeSkill:
            return fmt::format("Skill(Dodge), {}", value);
        case VAR_UnarmedSkill:
            return fmt::format("Skill(Unarmed), {}", value);
        case VAR_IdentifyMonsterSkill:
            return fmt::format("Skill(IdentifyMonster), {}", value);
        case VAR_ArmsmasterSkill:
            return fmt::format("Skill(Armsmaster), {}", value);
        case VAR_StealingSkill:
            return fmt::format("Skill(Stealing), {}", value);
        case VAR_AlchemySkill:
            return fmt::format("Skill(Alchemy), {}", value);
        case VAR_LearningSkill:
            return fmt::format("Skill(Learning), {}", value);
        case VAR_Cursed:
            return fmt::format("Condition(Cursed)");
        case VAR_Weak:
            return fmt::format("Condition(Weak)");
        case VAR_Asleep:
            return fmt::format("Condition(Asleep)");
        case VAR_Afraid:
            return fmt::format("Condition(Afraid)");
        case VAR_Drunk:
            return fmt::format("Condition(Drunk)");
        case VAR_Insane:
            return fmt::format("Condition(Insane)");
        case VAR_PoisonedGreen:
            return fmt::format("Condition(PoisonWeak)");
        case VAR_DiseasedGreen:
            return fmt::format("Condition(DiseaseWeak)");
        case VAR_PoisonedYellow:
            return fmt::format("Condition(PoisonMedium)");
        case VAR_DiseasedYellow:
            return fmt::format("Condition(DiseaseMedium)");
        case VAR_PoisonedRed:
            return fmt::format("Condition(PoisonSevere)");
        case VAR_DiseasedRed:
            return fmt::format("Condition(DiseaseSevere)");
        case VAR_Paralyzed:
            return fmt::format("Condition(Paralyzed)");
        case VAR_Unconsious:
            return fmt::format("Condition(Unconsious)");
        case VAR_Dead:
            return fmt::format("Condition(Dead)");
        case VAR_Stoned:
            return fmt::format("Condition(Stoned)");
        case VAR_Eradicated:
            return fmt::format("Condition(Eradicated)");
        case VAR_MajorCondition:
            return fmt::format("MajorCondition");
        case VAR_AutoNotes:
            return fmt::format("AutoNote[{}]", value);
        case VAR_IsMightMoreThanBase:
            return fmt::format("ERROR: IsMightMoreThanBase");
        case VAR_IsIntellectMoreThanBase:
            return fmt::format("ERROR: IsIntellectMoreThanBase");
        case VAR_IsPersonalityMoreThanBase:
            return fmt::format("ERROR: IsPersonalityMoreThanBase");
        case VAR_IsEnduranceMoreThanBase:
            return fmt::format("ERROR: IsEnduranceMoreThanBase");
        case VAR_IsSpeedMoreThanBase:
            return fmt::format("ERROR: IsSpeedMoreThanBase");
        case VAR_IsAccuracyMoreThanBase:
            return fmt::format("ERROR: IsAccuracyMoreThanBase");
        case VAR_IsLuckMoreThanBase:
            return fmt::format("ERROR: IsLuckMoreThanBase");
        case VAR_PlayerBits:
            return fmt::format("PlayerBits[{}]", value);
        case VAR_NPCs2:
            return fmt::format("NPC[{}].Hired", value);
        case VAR_IsFlying:
            return fmt::format("ERROR: IsFlying, value");
        case VAR_HiredNPCHasSpeciality:
            return fmt::format("NPCProfession({})", value);
        case VAR_CircusPrises:
            return fmt::format("[{}], {}", (int)type, value); // TODO
        case VAR_NumSkillPoints:
            return fmt::format("SkillPoints, {}", value);
        case VAR_MonthIs:
            return fmt::format("ERROR: MonthIs, {}", value);
        case VAR_ReputationInCurrentLocation:
            return fmt::format("LocationReputation, {}", value);
        case VAR_Unknown1:
            return fmt::format("[{}], {}", (int)type, value); // TODO
        case VAR_GoldInBank:
            return fmt::format("GoldInBank, {}", value);
        case VAR_NumDeaths:
            return fmt::format("NumDeaths, {}", value);
        case VAR_NumBounties:
            return fmt::format("NumBounties, {}", value);
        case VAR_PrisonTerms:
            return fmt::format("PrisonTerms, {}", value);
        case VAR_ArenaWinsPage:
            return fmt::format("ArenaWinsPage, {}", value);
        case VAR_ArenaWinsSquire:
            return fmt::format("ArenaWinsSquire, {}", value);
        case VAR_ArenaWinsKnight:
            return fmt::format("ArenaWinsKnight, {}", value);
        case VAR_ArenaWinsLord:
            return fmt::format("ArenaWinsLord, {}", value);
        case VAR_Invisible:
            return fmt::format("ERROR: Invisible, value");
        case VAR_ItemEquipped:
            return fmt::format("ERROR: ItemEquipped, value");
    }

    return fmt::format("UNPROCESSED: [{}], {}", (int)type, value);
}

static std::string getVariableCompareStr(VariableType type, int value) {
    if (type >= VAR_MapPersistentVariable_0 && type <= VAR_MapPersistentVariable_74) {
        return fmt::format("MapVars[{}] >= {}", type - VAR_MapPersistentVariable_0, value);
    }

    if (type >= VAR_MapPersistentVariable_75 && type <= VAR_MapPersistentVariable_99) {
        return fmt::format("MapVarsDecor[{}] >= {}", type - VAR_MapPersistentVariable_75, value);
    }

    if (type >= VAR_Counter1 && type <= VAR_Counter10) {
        return fmt::format("Counter[{}] + Hours({}) <= PlayingTime", type - VAR_Counter1, value);
    }

    if (type >= VAR_UnknownTimeEvent0 && type <= VAR_UnknownTimeEvent19) {
        return fmt::format("ERROR: UnknownTimeEvent[{}], {}", type - VAR_UnknownTimeEvent0, value);
    }

    if (type >= VAR_History_0 && type <= VAR_History_28) {
        return fmt::format("ERROR: History[{}], {}", type - VAR_History_0, value);
    }

    switch (type) {
        case VAR_Sex:
            return fmt::format("Sex == {}", value);
        case VAR_Class:
            return fmt::format("Class == {}", value);
        case VAR_CurrentHP:
            return fmt::format("HP >= {}", value);
        case VAR_MaxHP:
            return fmt::format("HP >= MaxHP", value);
        case VAR_CurrentSP:
            return fmt::format("SP >= {}", value);
        case VAR_MaxSP:
            return fmt::format("SP >= MaxSP", value);
        case VAR_ActualAC:
            return fmt::format("AC >= {}", value);
        case VAR_ACModifier:
            return fmt::format("ACMod >= {}", value);
        case VAR_BaseLevel:
            return fmt::format("Level >= {}", value);
        case VAR_LevelModifier:
            return fmt::format("LevelMod >= {}", value);
        case VAR_Age:
            return fmt::format("Age >= {}", value);
        case VAR_Award:
            return fmt::format("Awards[{}]", value);
        case VAR_Experience:
            return fmt::format("Exp >= {}", value);
        case VAR_Race:
            return fmt::format("Race == {}", value);
        case VAR_QBits_QuestsDone:
            return fmt::format("QBits[{}]", value);
        case VAR_PlayerItemInHands:
            return fmt::format("HasItemInHand({})", value);
        case VAR_Hour:
            return fmt::format("HourOfDay == {}", value);
        case VAR_DayOfYear:
            return fmt::format("DayOfYear == {}", value);
        case VAR_DayOfWeek:
            return fmt::format("DayOfWeek == {}", value);
        case VAR_FixedGold:
            return fmt::format("Gold >= {}", value);
        case VAR_RandomGold:
            return fmt::format("ERROR: VAR_RandomGold, {}", value);
        case VAR_FixedFood:
            return fmt::format("Food >= {}", value);
        case VAR_RandomFood:
            return fmt::format("ERROR: VAR_RandomFood, {}", value);
        case VAR_MightBonus:
            return fmt::format("Bonus(Might) >= {}", value);
        case VAR_IntellectBonus:
            return fmt::format("Bonus(Intellect) >= {}", value);
        case VAR_PersonalityBonus:
            return fmt::format("Bonus(Personality) >= {}", value);
        case VAR_EnduranceBonus:
            return fmt::format("Bonus(Endurance) >= {}", value);
        case VAR_SpeedBonus:
            return fmt::format("Bonus(Speed) >= {}", value);
        case VAR_AccuracyBonus:
            return fmt::format("Bonus(Accuracy) >= {}", value);
        case VAR_LuckBonus:
            return fmt::format("Bonus(Luck) >= {}", value);
        case VAR_BaseMight:
            return fmt::format("Attibute(Might) >= {}", value);
        case VAR_BaseIntellect:
            return fmt::format("Attibute(Intellect) >= {}", value);
        case VAR_BasePersonality:
            return fmt::format("Attibute(Personality) >= {}", value);
        case VAR_BaseEndurance:
            return fmt::format("Attibute(Endurance) >= {}", value);
        case VAR_BaseSpeed:
            return fmt::format("Attibute(Speed) >= {}", value);
        case VAR_BaseAccuracy:
            return fmt::format("Attibute(Accuracy) >= {}", value);
        case VAR_BaseLuck:
            return fmt::format("Attibute(Luck) >= {}", value);
        case VAR_ActualMight:
            return fmt::format("Actual(Might) >= {}", value);
        case VAR_ActualIntellect:
            return fmt::format("Actual(Intellect) >= {}", value);
        case VAR_ActualPersonality:
            return fmt::format("Actual(Personality) >= {}", value);
        case VAR_ActualEndurance:
            return fmt::format("Actual(Endurance) >= {}", value);
        case VAR_ActualSpeed:
            return fmt::format("Actual(Speed) >= {}", value);
        case VAR_ActualAccuracy:
            return fmt::format("Actual(Accuracy) >= {}", value);
        case VAR_ActualLuck:
            return fmt::format("Actual(Luck) >= {}", value);
        case VAR_FireResistance:
            return fmt::format("Resist(Fire) >= {}", value);
        case VAR_AirResistance:
            return fmt::format("Resist(Air) >= {}", value);
        case VAR_WaterResistance:
            return fmt::format("Resist(Water) >= {}", value);
        case VAR_EarthResistance:
            return fmt::format("Resist(Earth) >= {}", value);
        case VAR_SpiritResistance:
            return fmt::format("Resist(Spirit) >= {}", value);
        case VAR_MindResistance:
            return fmt::format("Resist(Mind) >= {}", value);
        case VAR_BodyResistance:
            return fmt::format("Resist(Body) >= {}", value);
        case VAR_LightResistance:
            return fmt::format("Resist(Light) >= {}", value);
        case VAR_DarkResistance:
            return fmt::format("Resist(Dark) >= {}", value);
        case VAR_PhysicalResistance:
            return fmt::format("Resist(Phys) >= {}", value);
        case VAR_MagicResistance:
            return fmt::format("Resist(Magic) >= {}", value);
        case VAR_FireResistanceBonus:
            return fmt::format("BonusResist(Fire) >= {}", value);
        case VAR_AirResistanceBonus:
            return fmt::format("BonusResist(Air) >= {}", value);
        case VAR_WaterResistanceBonus:
            return fmt::format("BonusResist(Water) >= {}", value);
        case VAR_EarthResistanceBonus:
            return fmt::format("BonusResist(Earth) >= {}", value);
        case VAR_SpiritResistanceBonus:
            return fmt::format("BonusResist(Spirit) >= {}", value);
        case VAR_MindResistanceBonus:
            return fmt::format("BonusResist(Mind) >= {}", value);
        case VAR_BodyResistanceBonus:
            return fmt::format("BonusResist(Body) >= {}", value);
        case VAR_LightResistanceBonus:
            return fmt::format("BonusResist(Light) >= {}", value);
        case VAR_DarkResistanceBonus:
            return fmt::format("BonusResist(Dark) >= {}", value);
        case VAR_PhysicalResistanceBonus:
            return fmt::format("BonusResist(Phys) >= {}", value);
        case VAR_MagicResistanceBonus:
            return fmt::format("BonusResist(Magic) >= {}", value);
        case VAR_StaffSkill:
            return fmt::format("Skill(Staff) >= {}", value);
        case VAR_SwordSkill:
            return fmt::format("Skill(Sword) >= {}", value);
        case VAR_DaggerSkill:
            return fmt::format("Skill(Dagger) >= {}", value);
        case VAR_AxeSkill:
            return fmt::format("Skill(Axe) >= {}", value);
        case VAR_SpearSkill:
            return fmt::format("Skill(Spear) >= {}", value);
        case VAR_BowSkill:
            return fmt::format("Skill(Bow) >= {}", value);
        case VAR_MaceSkill:
            return fmt::format("Skill(Mace) >= {}", value);
        case VAR_BlasterSkill:
            return fmt::format("Skill(Blaster) >= {}", value);
        case VAR_ShieldSkill:
            return fmt::format("Skill(Shield) >= {}", value);
        case VAR_LeatherSkill:
            return fmt::format("Skill(Leather) >= {}", value);
        case VAR_SkillChain:
            return fmt::format("Skill(Chain) >= {}", value);
        case VAR_PlateSkill:
            return fmt::format("Skill(Plate) >= {}", value);
        case VAR_FireSkill:
            return fmt::format("Skill(Fire) >= {}", value);
        case VAR_AirSkill:
            return fmt::format("Skill(Air) >= {}", value);
        case VAR_WaterSkill:
            return fmt::format("Skill(Water) >= {}", value);
        case VAR_EarthSkill:
            return fmt::format("Skill(Earth) >= {}", value);
        case VAR_SpiritSkill:
            return fmt::format("Skill(Spirit) >= {}", value);
        case VAR_MindSkill:
            return fmt::format("Skill(Mind) >= {}", value);
        case VAR_BodySkill:
            return fmt::format("Skill(Body) >= {}", value);
        case VAR_LightSkill:
            return fmt::format("Skill(Light) >= {}", value);
        case VAR_DarkSkill:
            return fmt::format("Skill(Dark) >= {}", value);
        case VAR_IdentifyItemSkill:
            return fmt::format("Skill(IdentifyItem) >= {}", value);
        case VAR_MerchantSkill:
            return fmt::format("Skill(Merchant) >= {}", value);
        case VAR_RepairSkill:
            return fmt::format("Skill(Repair) >= {}", value);
        case VAR_BodybuildingSkill:
            return fmt::format("Skill(Bodybuilding) >= {}", value);
        case VAR_MeditationSkill:
            return fmt::format("Skill(Meditation) >= {}", value);
        case VAR_PerceptionSkill:
            return fmt::format("Skill(Perception) >= {}", value);
        case VAR_DiplomacySkill:
            return fmt::format("Skill(Diplomacy) >= {}", value);
        case VAR_ThieverySkill:
            return fmt::format("Skill(Thievery) >= {}", value);
        case VAR_DisarmTrapSkill:
            return fmt::format("Skill(DisarmTrap) >= {}", value);
        case VAR_DodgeSkill:
            return fmt::format("Skill(Dodge) >= {}", value);
        case VAR_UnarmedSkill:
            return fmt::format("Skill(Unarmed) >= {}", value);
        case VAR_IdentifyMonsterSkill:
            return fmt::format("Skill(IdentifyMonster) >= {}", value);
        case VAR_ArmsmasterSkill:
            return fmt::format("Skill(Armsmaster) >= {}", value);
        case VAR_StealingSkill:
            return fmt::format("Skill(Stealing) >= {}", value);
        case VAR_AlchemySkill:
            return fmt::format("Skill(Alchemy) >= {}", value);
        case VAR_LearningSkill:
            return fmt::format("Skill(Learning) >= {}", value);
        case VAR_Cursed:
            return fmt::format("Condition(Cursed)");
        case VAR_Weak:
            return fmt::format("Condition(Weak)");
        case VAR_Asleep:
            return fmt::format("Condition(Asleep)");
        case VAR_Afraid:
            return fmt::format("Condition(Afraid)");
        case VAR_Drunk:
            return fmt::format("Condition(Drunk)");
        case VAR_Insane:
            return fmt::format("Condition(Insane)");
        case VAR_PoisonedGreen:
            return fmt::format("Condition(PoisonWeak)");
        case VAR_DiseasedGreen:
            return fmt::format("Condition(DiseaseWeak)");
        case VAR_PoisonedYellow:
            return fmt::format("Condition(PoisonMedium)");
        case VAR_DiseasedYellow:
            return fmt::format("Condition(DiseaseMedium)");
        case VAR_PoisonedRed:
            return fmt::format("Condition(PoisonSevere)");
        case VAR_DiseasedRed:
            return fmt::format("Condition(DiseaseSevere)");
        case VAR_Paralyzed:
            return fmt::format("Condition(Paralyzed)");
        case VAR_Unconsious:
            return fmt::format("Condition(Unconsious)");
        case VAR_Dead:
            return fmt::format("Condition(Dead)");
        case VAR_Stoned:
            return fmt::format("Condition(Stoned)");
        case VAR_Eradicated:
            return fmt::format("Condition(Eradicated)");
        case VAR_MajorCondition:
            return fmt::format("Condition == Good || Condition >= {}", value);
        case VAR_AutoNotes:
            return fmt::format("AutoNote[{}]", value);
        case VAR_IsMightMoreThanBase:
            return fmt::format("Actual(Might) >= Attibute(Might)");
        case VAR_IsIntellectMoreThanBase:
            return fmt::format("Actual(Intellect) >= Attibute(Intellect)");
        case VAR_IsPersonalityMoreThanBase:
            return fmt::format("Actual(Personality) >= Attibute(Personality)");
        case VAR_IsEnduranceMoreThanBase:
            return fmt::format("Actual(Endurance) >= Attibute(Endurance)");
        case VAR_IsSpeedMoreThanBase:
            return fmt::format("Actual(Speed) >= Attibute(Speed)");
        case VAR_IsAccuracyMoreThanBase:
            return fmt::format("Actual(Accuracy) >= Attibute(Accuracy)");
        case VAR_IsLuckMoreThanBase:
            return fmt::format("Actual(Luck) >= Attibute(Luck)");
        case VAR_PlayerBits:
            return fmt::format("PlayerBits[{}]", value);
        case VAR_NPCs2:
            return fmt::format("NPC[{}].Hired", value);
        case VAR_IsFlying:
            return fmt::format("Flying");
        case VAR_HiredNPCHasSpeciality:
            return fmt::format("NPCProfession({})", value);
        case VAR_CircusPrises:
            return fmt::format("[{}] ? {}", (int)type, value); // TODO
        case VAR_NumSkillPoints:
            return fmt::format("SkillPoints >= {}", value);
        case VAR_MonthIs:
            return fmt::format("Month == {}", value);
        case VAR_ReputationInCurrentLocation:
            return fmt::format("LocationReputation >= {}", value);
        case VAR_Unknown1:
            return fmt::format("[{}] ? {}", (int)type, value); // TODO
        case VAR_GoldInBank:
            return fmt::format("GoldInBank >= {}", value);
        case VAR_NumDeaths:
            return fmt::format("NumDeaths >= {}", value);
        case VAR_NumBounties:
            return fmt::format("NumBounties >= {}", value);
        case VAR_PrisonTerms:
            return fmt::format("PrisonTerms >= {}", value);
        case VAR_ArenaWinsPage:
            return fmt::format("ArenaWinsPage >= {}", value);
        case VAR_ArenaWinsSquire:
            return fmt::format("ArenaWinsSquire >= {}", value);
        case VAR_ArenaWinsKnight:
            return fmt::format("ArenaWinsKnight >= {}", value);
        case VAR_ArenaWinsLord:
            return fmt::format("ArenaWinsLord >= {}", value);
        case VAR_Invisible:
            return fmt::format("Invisible");
        case VAR_ItemEquipped:
            return fmt::format("ItemEquipped({})", value);
    }

    return fmt::format("UNPROCESSED: [{}] ? {}", (int)type, value);
}

static void dumpEventIR(EventIR &ir, bool dynamic) {
    std::string outputString = fmt::format("{}: UNPROCESSED/{}", ir.step, magic_enum::enum_name(ir.type));

    switch (ir.type) {
        case EVENT_Exit:
            outputString = fmt::format("{}: Exit", ir.step);
            break;
        case EVENT_SpeakInHouse:
            if (p2DEvents[ir.data.house_id - 1].pName != NULL) {
                outputString = fmt::format("{}: SpeakInHouse({}, \"{}\")", ir.step, ir.data.house_id, p2DEvents[ir.data.house_id - 1].pName);
            } else {
                outputString = fmt::format("{}: SpeakInHouse({})", ir.step, ir.data.house_id);
            }
            break;
        case EVENT_PlaySound:
            outputString = fmt::format("{}: PlaySound({}, ({}, {}))", ir.step, (int)ir.data.sound_descr.sound_id, ir.data.sound_descr.x, ir.data.sound_descr.y);
            break;
        case EVENT_MouseOver:
            if (dynamic && pLevelStrOffsets[ir.data.text_id] < pLevelStr.size()) {
                outputString = fmt::format("{}: MouseOver(\"{}\")", ir.step, &pLevelStr[pLevelStrOffsets[ir.data.text_id]]);
            } else {
                outputString = fmt::format("{}: MouseOver({})", ir.step, ir.data.text_id);
            }
            break;
        case EVENT_LocationName:
            outputString = fmt::format("{}: LocationName", ir.step);
            break;
        case EVENT_MoveToMap:
            outputString = fmt::format("{}: MoveToMap(({}, {}, {}), {}, {}, {}, {}, {}, \"{}\")", ir.step, ir.data.move_map_descr.x, ir.data.move_map_descr.y, ir.data.move_map_descr.z, ir.data.move_map_descr.yaw, ir.data.move_map_descr.pitch, ir.data.move_map_descr.zspeed, ir.data.move_map_descr.anim_id, ir.data.move_map_descr.exit_pic_id, ir.str);
            break;
        case EVENT_OpenChest:
            outputString = fmt::format("{}: OpenChest({})", ir.step, ir.data.chest_id);
            break;
        case EVENT_ShowFace:
            outputString = fmt::format("{}: SetExpression({}, {})", ir.step, ir.who, (int)ir.data.expr_id);
            break;
        case EVENT_ReceiveDamage:
            outputString = fmt::format("{}: ReceiveDamage({}, {})", ir.step, ir.data.damage_descr.damage, (int)ir.data.damage_descr.damage_type);
            break;
        case EVENT_SetSnow:
            outputString = fmt::format("{}: SetSnow({}, {})", ir.step, ir.data.snow_descr.is_nop, ir.data.snow_descr.is_enable);
            break;
        case EVENT_SetTexture:
            outputString = fmt::format("{}: SetTexture({}, \"{}\")", ir.step, ir.data.sprite_texture_descr.cog, ir.str);
            break;
        case EVENT_ShowMovie:
            outputString = fmt::format("{}: ShowMovie({}, {})", ir.step, ir.str, ir.data.movie_unknown_field);
            break;
        case EVENT_SetSprite:
            outputString = fmt::format("{}: SetSprite({}, {}, \"{}\")", ir.step, ir.data.sprite_texture_descr.cog, ir.data.sprite_texture_descr.hide, ir.str);
            break;
        case EVENT_Compare:
            outputString = fmt::format("{}: If({}) -> {}", ir.step, getVariableCompareStr(ir.data.variable_descr.type, ir.data.variable_descr.value), ir.target_step);
            break;
        case EVENT_ChangeDoorState:
            outputString = fmt::format("{}: ChangeDoorState({}, {})", ir.step, ir.data.door_descr.door_id, ir.data.door_descr.door_new_state);
            break;
        case EVENT_Add:
            outputString = fmt::format("{}: Add({})", ir.step, getVariableSetStr(ir.data.variable_descr.type, ir.data.variable_descr.value));
            break;
        case EVENT_Substract:
            outputString = fmt::format("{}: Sub({})", ir.step, getVariableSetStr(ir.data.variable_descr.type, ir.data.variable_descr.value));
            break;
        case EVENT_Set:
            outputString = fmt::format("{}: Set({})", ir.step, getVariableSetStr(ir.data.variable_descr.type, ir.data.variable_descr.value));
            break;
        case EVENT_SummonMonsters:
            outputString = fmt::format("{}: SummonMonster({}, {}, {}, ({}, {}, {}), {}, {})", ir.step, ir.data.monster_descr.type, ir.data.monster_descr.level, ir.data.monster_descr.count, ir.data.monster_descr.x, ir.data.monster_descr.y, ir.data.monster_descr.z, ir.data.monster_descr.group, ir.data.monster_descr.name_id);
            break;
        case EVENT_CastSpell:
            outputString = fmt::format("{}: CastSpell({}, {}, {}, ({}, {}, {}), ({}, {}, {}))", ir.step, (int)ir.data.spell_descr.spell_id, (int)ir.data.spell_descr.spell_mastery, ir.data.spell_descr.spell_level, ir.data.spell_descr.fromx, ir.data.spell_descr.fromy, ir.data.spell_descr.fromz, ir.data.spell_descr.tox, ir.data.spell_descr.toy, ir.data.spell_descr.toz);
            break;
        case EVENT_SpeakNPC:
            outputString = fmt::format("{}: SpeakNPC({})", ir.step, ir.data.npc_descr.npc_id);
            break;
        case EVENT_SetFacesBit:
            outputString = fmt::format("{}: SetFacesBit({}, 0x{:x}, {})", ir.step, ir.data.faces_bit_descr.cog, (int)ir.data.faces_bit_descr.face_bit, ir.data.faces_bit_descr.is_on);
            break;
        case EVENT_ToggleActorFlag:
            outputString = fmt::format("{}: ToggleActorFlag({}, 0x{:x}, {})", ir.step, ir.data.actor_flag_descr.id, (int)ir.data.actor_flag_descr.attr, ir.data.actor_flag_descr.is_set); // TODO: print attr

            break;
        case EVENT_RandomGoTo:
            {
                std::string jmps;
                for (int i = 0 ; i < ir.data.random_goto_descr.random_goto_len ; i++) {
                    if (i) {
                        jmps += fmt::format(", {}", ir.data.random_goto_descr.random_goto[i]);
                    } else {
                        jmps += fmt::format("{}", ir.data.random_goto_descr.random_goto[i]);
                    }
                }
                outputString = fmt::format("{}: RandomJmp -> ({})", ir.step, jmps);
                break;
            }
        case EVENT_InputString:
            if (dynamic) {
                outputString = fmt::format("{}: InputString(\"{}\")", ir.step, &pLevelStr[pLevelStrOffsets[ir.data.text_id]]);
            } else {
                outputString = fmt::format("{}: InputString({})", ir.step, ir.data.text_id);
            }
            break;
        case EVENT_StatusText:
            if (dynamic) {
                if (activeLevelDecoration) {
                    outputString = fmt::format("{}: StatusMessage(\"{}\")", ir.step, pNPCTopics[ir.data.text_id - 1].pText);
                } else {
                    outputString = fmt::format("{}: StatusMessage(\"{}\")", ir.step, &pLevelStr[pLevelStrOffsets[ir.data.text_id]]);
                }
            } else {
                outputString = fmt::format("{}: StatusMessage({})", ir.step, ir.data.text_id);
            }
            break;
        case EVENT_ShowMessage:
            if (dynamic) {
                if (activeLevelDecoration) {
                    outputString = fmt::format("{}: ShowMessage(\"{}\")", ir.step, pNPCTopics[ir.data.text_id - 1].pText);
                } else {
                    outputString = fmt::format("{}: ShowMessage(\"{}\")", ir.step, &pLevelStr[pLevelStrOffsets[ir.data.text_id]]);
                }
            } else {
                outputString = fmt::format("{}: ShowMessage({})", ir.step, ir.data.text_id);
            }
            break;
        case EVENT_OnTimer:
            outputString = fmt::format("{}: OnTimer({}yr, {}m, {}w, {}hr, {}min, {}sec, {})", ir.step, ir.data.timer_descr.years, ir.data.timer_descr.months, ir.data.timer_descr.weeks, ir.data.timer_descr.hours, ir.data.timer_descr.minutes, ir.data.timer_descr.seconds, ir.data.timer_descr.alternative_interval);
            break;
        case EVENT_ToggleIndoorLight:
            outputString = fmt::format("{}: ToggleIndoorLight({}, {})", ir.step, ir.data.light_descr.light_id, ir.data.light_descr.is_enable);
            break;
        case EVENT_PressAnyKey:
            outputString = fmt::format("{}: PressAnyKey()", ir.step);
            break;
        case EVENT_SummonItem:
            outputString = fmt::format("{}: SummonItem({}, ({}, {}, {}), {}, {}, {})", ir.step, (int)ir.data.summon_item_descr.sprite, ir.data.summon_item_descr.x, ir.data.summon_item_descr.y, ir.data.summon_item_descr.z, ir.data.summon_item_descr.speed, ir.data.summon_item_descr.count, ir.data.summon_item_descr.random_rotate);
            break;
        case EVENT_ForPartyMember:
            outputString = fmt::format("{}: ForPartyMember({})", ir.step, ir.who);
            break;
        case EVENT_Jmp:
            outputString = fmt::format("{}: Jmp -> {}", ir.step, ir.target_step);
            break;
        case EVENT_OnMapReload:
            outputString = fmt::format("{}: OnMapReload", ir.step);
            break;
        case EVENT_OnLongTimer:
            outputString = fmt::format("{}: OnLongTimer({}yr, {}m, {}w, {}hr, {}min, {}sec, {})", ir.step, ir.data.timer_descr.years, ir.data.timer_descr.months, ir.data.timer_descr.weeks, ir.data.timer_descr.hours, ir.data.timer_descr.minutes, ir.data.timer_descr.seconds, ir.data.timer_descr.alternative_interval);
            break;
        case EVENT_SetNPCTopic:
            outputString = fmt::format("{}: SetNPCTopic({}, {}, {})", ir.step, ir.data.npc_topic_descr.npc_id, ir.data.npc_topic_descr.index, ir.data.npc_topic_descr.event_id);
            break;
        case EVENT_MoveNPC:
            outputString = fmt::format("{}: MoveNPC({}, {})", ir.step, ir.data.npc_move_descr.npc_id, ir.data.npc_move_descr.location_id);
            break;
        case EVENT_GiveItem:
            outputString = fmt::format("{}: GiveItem({}, {}, {})", ir.step, (int)ir.data.give_item_descr.treasure_level, ir.data.give_item_descr.treasure_type, (int)ir.data.give_item_descr.item_id);
            break;
        case EVENT_ChangeEvent:
            outputString = fmt::format("{}: ChangeEvent({})", ir.step, ir.data.event_id);
            break;
        case EVENT_CheckSkill:
             outputString = fmt::format("{}: CheckSkill({}, {}, {}) -> {}", ir.step, (int)ir.data.check_skill_descr.skill_type, (int)ir.data.check_skill_descr.skill_mastery, (int)ir.data.check_skill_descr.skill_level, ir.target_step);
            break;
        case EVENT_OnCanShowDialogItemCmp:
            // TODO
            break;
        case EVENT_EndCanShowDialogItem:
            // TODO
            break;
        case EVENT_SetCanShowDialogItem:
            // TODO
            break;
        case EVENT_SetNPCGroupNews:
            outputString = fmt::format("{}: SetNPCGroupNews({}, {})", ir.step, ir.data.npc_groups_descr.groups_id, ir.data.npc_groups_descr.group);
            break;
        case EVENT_SetActorGroup:
            // TODO
            break;
        case EVENT_NPCSetItem:
            outputString = fmt::format("{}: NPCSetItem({}, {}, {})", ir.step, ir.data.npc_item_descr.id, (int)ir.data.npc_item_descr.item, ir.data.npc_item_descr.is_give);
            break;
        case EVENT_SetNPCGreeting:
            outputString = fmt::format("{}: SetNpcGreeting({}, {})", ir.step, ir.data.npc_descr.npc_id, ir.data.npc_descr.greeting);
            break;
        case EVENT_IsActorAlive:
            outputString = fmt::format("{}: IsActorAlive({}, {}, {}) -> {}", ir.step, ir.data.actor_descr.type, ir.data.actor_descr.param, ir.data.actor_descr.num, ir.target_step);
            break;
        case EVENT_IsActorAssasinated:
            // TODO
            break;
        case EVENT_OnMapLeave:
            outputString = fmt::format("{}: OnMapLeave", ir.step);
            break;
        case EVENT_ChangeGroup:
            // TODO
            break;
        case EVENT_ChangeGroupAlly:
            // TODO
            break;
        case EVENT_CheckSeason:
            outputString = fmt::format("{}: CheckSeason({}) -> {}", ir.step, ir.data.season, ir.target_step);
            break;
        case EVENT_ToggleActorGroupFlag:
            outputString = fmt::format("{}: ToggleActorGroupFlag({}, 0x{:x}, {})", ir.step, ir.data.actor_flag_descr.id, (int)ir.data.actor_flag_descr.attr, ir.data.actor_flag_descr.is_set); // TODO: print attr
            break;
        case EVENT_ToggleChestFlag:
            outputString = fmt::format("{}: ToggleChestFlag({}, 0x{:x}, {})", ir.step, ir.data.chest_flag_descr.chest_id, (int)ir.data.chest_flag_descr.flag, ir.data.chest_flag_descr.is_set);
            break;
        case EVENT_CharacterAnimation:
            outputString = fmt::format("{}: SetReaction({}, {})", ir.step, ir.who, (int)ir.data.speech_id);
            break;
        case EVENT_SetActorItem:
            outputString = fmt::format("{}: SetActorItem({}, {}, {})", ir.step, ir.data.npc_item_descr.id, (int)ir.data.npc_item_descr.item, ir.data.npc_item_descr.is_give);
            break;
        case EVENT_OnDateTimer:
            // TODO
            break;
        case EVENT_EnableDateTimer:
            // TODO
            break;
        case EVENT_StopAnimation:
            // TODO
            break;
        case EVENT_CheckItemsCount:
            // TODO
            break;
        case EVENT_RemoveItems:
            // TODO
            break;
        case EVENT_SpecialJump:
            // TODO
            break;
        case EVENT_IsTotalBountyHuntingAwardInRange:
            // TODO
            break;
        case EVENT_IsNPCInParty:
            // TODO
            break;
        default:
            break;
    }

    logger->verbose("{}", outputString);
}

void dumpEvent(int eventId, bool isGlobal) {
    std::map<int, std::vector<EventIR>> &eventsMap = isGlobal ? globalEventsMap : localEventsMap;

    if (eventsMap.contains(eventId)) {
        logger->verbose("Event: {}", eventId);
        for (EventIR &ir : eventsMap[eventId]) {
            dumpEventIR(ir, true);
        }
    } else {
        logger->verbose("Event {} not found", eventId);
    }
}

void dumpAllEvents(bool isGlobal) {
    for (std::pair<int, std::vector<EventIR>> pair : (isGlobal ? globalEventsMap : localEventsMap)) {
        dumpEventLocal(pair.first);
    }
}
