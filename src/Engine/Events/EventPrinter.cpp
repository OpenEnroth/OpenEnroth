#include <string>
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

std::string EventIR::toString() {
    switch (type) {
        case EVENT_Exit:
            return fmt::format("{}: Exit", step);
            break;
        case EVENT_SpeakInHouse:
            if (p2DEvents[data.house_id - 1].pName != NULL) {
                return fmt::format("{}: SpeakInHouse({}, \"{}\")", step, data.house_id, p2DEvents[data.house_id - 1].pName);
            } else {
                return fmt::format("{}: SpeakInHouse({})", step, data.house_id);
            }
            break;
        case EVENT_PlaySound:
            return fmt::format("{}: PlaySound({}, ({}, {}))", step, (int)data.sound_descr.sound_id, data.sound_descr.x, data.sound_descr.y);
            break;
        case EVENT_MouseOver:
            return fmt::format("{}: MouseOver(\"{}\")", step, &pLevelStr[pLevelStrOffsets[data.text_id]]);
            break;
        case EVENT_LocationName:
            return fmt::format("{}: LocationName", step);
            break;
        case EVENT_MoveToMap:
            return fmt::format("{}: MoveToMap(({}, {}, {}), {}, {}, {}, {}, {}, \"{}\")", step, data.move_map_descr.x, data.move_map_descr.y, data.move_map_descr.z, data.move_map_descr.yaw, data.move_map_descr.pitch, data.move_map_descr.zspeed, data.move_map_descr.anim_id, data.move_map_descr.exit_pic_id, str);
            break;
        case EVENT_OpenChest:
            return fmt::format("{}: OpenChest({})", step, data.chest_id);
            break;
        case EVENT_ShowFace:
            return fmt::format("{}: SetExpression({}, {})", step, who, (int)data.expr_id);
            break;
        case EVENT_ReceiveDamage:
            return fmt::format("{}: ReceiveDamage({}, {})", step, data.damage_descr.damage, (int)data.damage_descr.damage_type);
            break;
        case EVENT_SetSnow:
            return fmt::format("{}: SetSnow({}, {})", step, data.snow_descr.is_nop, data.snow_descr.is_enable);
            break;
        case EVENT_SetTexture:
            return fmt::format("{}: SetTexture({}, \"{}\")", step, data.sprite_texture_descr.cog, str);
            break;
        case EVENT_ShowMovie:
            return fmt::format("{}: ShowMovie({}, {})", step, str, data.movie_unknown_field);
            break;
        case EVENT_SetSprite:
            return fmt::format("{}: SetSprite({}, {}, \"{}\")", step, data.sprite_texture_descr.cog, data.sprite_texture_descr.hide, str);
            break;
        case EVENT_Compare:
            return fmt::format("{}: If({}) -> {}", step, getVariableCompareStr(data.variable_descr.type, data.variable_descr.value), target_step);
            break;
        case EVENT_ChangeDoorState:
            return fmt::format("{}: ChangeDoorState({}, {})", step, data.door_descr.door_id, data.door_descr.door_new_state);
            break;
        case EVENT_Add:
            return fmt::format("{}: Add({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
            break;
        case EVENT_Substract:
            return fmt::format("{}: Sub({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
            break;
        case EVENT_Set:
            return fmt::format("{}: Set({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
            break;
        case EVENT_SummonMonsters:
            return fmt::format("{}: SummonMonster({}, {}, {}, ({}, {}, {}), {}, {})", step, data.monster_descr.type, data.monster_descr.level, data.monster_descr.count, data.monster_descr.x, data.monster_descr.y, data.monster_descr.z, data.monster_descr.group, data.monster_descr.name_id);
            break;
        case EVENT_CastSpell:
            return fmt::format("{}: CastSpell({}, {}, {}, ({}, {}, {}), ({}, {}, {}))", step, (int)data.spell_descr.spell_id, (int)data.spell_descr.spell_mastery, data.spell_descr.spell_level, data.spell_descr.fromx, data.spell_descr.fromy, data.spell_descr.fromz, data.spell_descr.tox, data.spell_descr.toy, data.spell_descr.toz);
            break;
        case EVENT_SpeakNPC:
            return fmt::format("{}: SpeakNPC({})", step, data.npc_descr.npc_id);
            break;
        case EVENT_SetFacesBit:
            return fmt::format("{}: SetFacesBit({}, 0x{:x}, {})", step, data.faces_bit_descr.cog, (int)data.faces_bit_descr.face_bit, data.faces_bit_descr.is_on);
            break;
        case EVENT_ToggleActorFlag:
            return fmt::format("{}: ToggleActorFlag({}, 0x{:x}, {})", step, data.actor_flag_descr.id, (int)data.actor_flag_descr.attr, data.actor_flag_descr.is_set); // TODO: print attr

            break;
        case EVENT_RandomGoTo:
            {
                std::string jmps;
                for (int i = 0 ; i < data.random_goto_descr.random_goto_len ; i++) {
                    if (i) {
                        jmps += fmt::format(", {}", data.random_goto_descr.random_goto[i]);
                    } else {
                        jmps += fmt::format("{}", data.random_goto_descr.random_goto[i]);
                    }
                }
                return fmt::format("{}: RandomJmp -> ({})", step, jmps);
                break;
            }
        case EVENT_InputString:
            return fmt::format("{}: InputString(\"{}\")", step, &pLevelStr[pLevelStrOffsets[data.text_id]]);
            break;
        case EVENT_StatusText:
            if (activeLevelDecoration) {
                return fmt::format("{}: StatusMessage(\"{}\")", step, pNPCTopics[data.text_id - 1].pText);
            } else {
                return fmt::format("{}: StatusMessage(\"{}\")", step, &pLevelStr[pLevelStrOffsets[data.text_id]]);
            }
            break;
        case EVENT_ShowMessage:
            if (activeLevelDecoration) {
                return fmt::format("{}: ShowMessage(\"{}\")", step, pNPCTopics[data.text_id - 1].pText);
            } else {
                return fmt::format("{}: ShowMessage(\"{}\")", step, &pLevelStr[pLevelStrOffsets[data.text_id]]);
            }
            break;
        case EVENT_OnTimer:
            return fmt::format("{}: OnTimer({}yr, {}m, {}w, {}hr, {}min, {}sec, {})", step, data.timer_descr.years, data.timer_descr.months, data.timer_descr.weeks, data.timer_descr.hours, data.timer_descr.minutes, data.timer_descr.seconds, data.timer_descr.alternative_interval);
            break;
        case EVENT_ToggleIndoorLight:
            return fmt::format("{}: ToggleIndoorLight({}, {})", step, data.light_descr.light_id, data.light_descr.is_enable);
            break;
        case EVENT_PressAnyKey:
            return fmt::format("{}: PressAnyKey()", step);
            break;
        case EVENT_SummonItem:
            return fmt::format("{}: SummonItem({}, ({}, {}, {}), {}, {}, {})", step, (int)data.summon_item_descr.sprite, data.summon_item_descr.x, data.summon_item_descr.y, data.summon_item_descr.z, data.summon_item_descr.speed, data.summon_item_descr.count, data.summon_item_descr.random_rotate);
            break;
        case EVENT_ForPartyMember:
            return fmt::format("{}: ForPartyMember({})", step, who);
            break;
        case EVENT_Jmp:
            return fmt::format("{}: Jmp -> {}", step, target_step);
            break;
        case EVENT_OnMapReload:
            return fmt::format("{}: OnMapReload", step);
            break;
        case EVENT_OnLongTimer:
            return fmt::format("{}: OnLongTimer({}yr, {}m, {}w, {}hr, {}min, {}sec, {})", step, data.timer_descr.years, data.timer_descr.months, data.timer_descr.weeks, data.timer_descr.hours, data.timer_descr.minutes, data.timer_descr.seconds, data.timer_descr.alternative_interval);
            break;
        case EVENT_SetNPCTopic:
            return fmt::format("{}: SetNPCTopic({}, {}, {})", step, data.npc_topic_descr.npc_id, data.npc_topic_descr.index, data.npc_topic_descr.event_id);
            break;
        case EVENT_MoveNPC:
            return fmt::format("{}: MoveNPC({}, {})", step, data.npc_move_descr.npc_id, data.npc_move_descr.location_id);
            break;
        case EVENT_GiveItem:
            return fmt::format("{}: GiveItem({}, {}, {})", step, (int)data.give_item_descr.treasure_level, data.give_item_descr.treasure_type, (int)data.give_item_descr.item_id);
            break;
        case EVENT_ChangeEvent:
            return fmt::format("{}: ChangeEvent({})", step, data.event_id);
            break;
        case EVENT_CheckSkill:
             return fmt::format("{}: CheckSkill({}, {}, {}) -> {}", step, (int)data.check_skill_descr.skill_type, (int)data.check_skill_descr.skill_mastery, (int)data.check_skill_descr.skill_level, target_step);
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
            return fmt::format("{}: SetNPCGroupNews({}, {})", step, data.npc_groups_descr.groups_id, data.npc_groups_descr.group);
            break;
        case EVENT_SetActorGroup:
            // TODO
            break;
        case EVENT_NPCSetItem:
            return fmt::format("{}: NPCSetItem({}, {}, {})", step, data.npc_item_descr.id, (int)data.npc_item_descr.item, data.npc_item_descr.is_give);
            break;
        case EVENT_SetNPCGreeting:
            return fmt::format("{}: SetNpcGreeting({}, {})", step, data.npc_descr.npc_id, data.npc_descr.greeting);
            break;
        case EVENT_IsActorAlive:
            return fmt::format("{}: IsActorAlive({}, {}, {}) -> {}", step, data.actor_descr.type, data.actor_descr.param, data.actor_descr.num, target_step);
            break;
        case EVENT_IsActorAssasinated:
            // TODO
            break;
        case EVENT_OnMapLeave:
            return fmt::format("{}: OnMapLeave", step);
            break;
        case EVENT_ChangeGroup:
            // TODO
            break;
        case EVENT_ChangeGroupAlly:
            // TODO
            break;
        case EVENT_CheckSeason:
            return fmt::format("{}: CheckSeason({}) -> {}", step, data.season, target_step);
            break;
        case EVENT_ToggleActorGroupFlag:
            return fmt::format("{}: ToggleActorGroupFlag({}, 0x{:x}, {})", step, data.actor_flag_descr.id, (int)data.actor_flag_descr.attr, data.actor_flag_descr.is_set); // TODO: print attr
            break;
        case EVENT_ToggleChestFlag:
            return fmt::format("{}: ToggleChestFlag({}, 0x{:x}, {})", step, data.chest_flag_descr.chest_id, (int)data.chest_flag_descr.flag, data.chest_flag_descr.is_set);
            break;
        case EVENT_CharacterAnimation:
            return fmt::format("{}: SetReaction({}, {})", step, who, (int)data.speech_id);
            break;
        case EVENT_SetActorItem:
            return fmt::format("{}: SetActorItem({}, {}, {})", step, data.npc_item_descr.id, (int)data.npc_item_descr.item, data.npc_item_descr.is_give);
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

    return fmt::format("{}: UNPROCESSED/{}", step, magic_enum::enum_name(type));
}

void EventMap::dump(int eventId) {
    if (_eventsById.contains(eventId)) {
        logger->verbose("Event: {}", eventId);
        for (EventIR &ir : _eventsById[eventId]) {
            logger->verbose("{}", ir.toString());
        }
    } else {
        logger->verbose("Event {} not found", eventId);
    }
}

void EventMap::dumpAll() {
    for (std::pair<int, std::vector<EventIR>> pair : _eventsById) {
        dump(pair.first);
    }
}
