#include <assert.h>
#include <string>
#include <string_view>
#include <vector>
#include <magic_enum.hpp> // TODO: temporary
#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventEnums.h"
#include "Engine/Events/Loader.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Engine.h"
#include "Utility/String.h"
#include "Utility/IndexedArray.h"
#include "Utility/Workaround/ToUnderlying.h"
#include "fmt/core.h"

enum class ChestFlag : uint16_t;
enum class FaceAttribute : uint32_t;
enum class ItemId : int32_t;
enum class ItemTreasureLevel : int8_t;

static std::string getVariableSetStr(VariableType type, int value) {
    if (type >= VAR_MapPersistentVariable_0 && type <= VAR_MapPersistentVariable_74) {
        return fmt::format("MapVars[{}], {}", std::to_underlying(type) - std::to_underlying(VAR_MapPersistentVariable_0), value);
    }

    if (type >= VAR_MapPersistentDecorVariable_0 && type <= VAR_MapPersistentDecorVariable_24) {
        return fmt::format("MapVarsDecor[{}], {}", std::to_underlying(type) - std::to_underlying(VAR_MapPersistentDecorVariable_0), value);
    }

    if (type >= VAR_Counter1 && type <= VAR_Counter10) {
        return fmt::format("Counter[{}], PlayingTime", std::to_underlying(type) - std::to_underlying(VAR_Counter1));
    }

    if (type >= VAR_UnknownTimeEvent0 && type <= VAR_UnknownTimeEvent19) {
        return fmt::format("UnkTimeEvent[{}], PlayingTime", std::to_underlying(type) - std::to_underlying(VAR_UnknownTimeEvent0));
    }

    if (type >= VAR_History_0 && type <= VAR_History_28) {
        return fmt::format("History[{}]", std::to_underlying(type) - std::to_underlying(VAR_History_0));
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
        default:
            return fmt::format("UNPROCESSED: [{}], {}", std::to_underlying(type), value);
    }
}

static std::string getVariableCompareStr(VariableType type, int value) {
    if (type >= VAR_MapPersistentVariable_0 && type <= VAR_MapPersistentVariable_74) {
        return fmt::format("MapVars[{}] >= {}", std::to_underlying(type) - std::to_underlying(VAR_MapPersistentVariable_0), value);
    }

    if (type >= VAR_MapPersistentDecorVariable_0 && type <= VAR_MapPersistentDecorVariable_24) {
        return fmt::format("MapVarsDecor[{}] >= {}", std::to_underlying(type) - std::to_underlying(VAR_MapPersistentDecorVariable_0), value);
    }

    if (type >= VAR_Counter1 && type <= VAR_Counter10) {
        return fmt::format("Counter[{}] + Hours({}) <= PlayingTime", std::to_underlying(type) - std::to_underlying(VAR_Counter1), value);
    }

    if (type >= VAR_UnknownTimeEvent0 && type <= VAR_UnknownTimeEvent19) {
        return fmt::format("ERROR: UnknownTimeEvent[{}], {}", std::to_underlying(type) - std::to_underlying(VAR_UnknownTimeEvent0), value);
    }

    if (type >= VAR_History_0 && type <= VAR_History_28) {
        return fmt::format("ERROR: History[{}], {}", std::to_underlying(type) - std::to_underlying(VAR_History_0), value);
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
        default:
            return fmt::format("UNPROCESSED: [{}] ? {}", std::to_underlying(type), value);
    }
}

std::string EventIR::toString() const {
    switch (type) {
        case EVENT_Exit:
            return fmt::format("{}: Exit", step);
        case EVENT_SpeakInHouse:
            if ((data.house_id - 1) < buildingTable.size() && !buildingTable[data.house_id].pName.empty()) {
                return fmt::format("{}: SpeakInHouse({}, \"{}\")", step, data.house_id, buildingTable[data.house_id].pName);
            } else {
                return fmt::format("{}: SpeakInHouse({})", step, data.house_id);
            }
        case EVENT_PlaySound:
            return fmt::format("{}: PlaySound({}, ({}, {}))", step, std::to_underlying(data.sound_descr.sound_id), data.sound_descr.x, data.sound_descr.y);
        case EVENT_MouseOver:
            if (data.text_id < engine->_levelStrings.size()) {
                return fmt::format("{}: MouseOver(\"{}\")", step, engine->_levelStrings[data.text_id]);
            } else {
                return fmt::format("{}: MouseOver({})", step, data.text_id);
            }
        case EVENT_LocationName:
            return fmt::format("{}: LocationName", step);
        case EVENT_MoveToMap:
            return fmt::format("{}: MoveToMap(({}, {}, {}), {}, {}, {}, {}, {}, \"{}\")", step, data.move_map_descr.x, data.move_map_descr.y, data.move_map_descr.z, data.move_map_descr.yaw, data.move_map_descr.pitch, data.move_map_descr.zspeed, std::to_underlying(data.move_map_descr.house_id), data.move_map_descr.exit_pic_id, str);
        case EVENT_OpenChest:
            return fmt::format("{}: OpenChest({})", step, data.chest_id);
        case EVENT_ShowFace:
            return fmt::format("{}: SetExpression({}, {})", step, std::to_underlying(who), std::to_underlying(data.expr_id));
        case EVENT_ReceiveDamage:
            return fmt::format("{}: ReceiveDamage({}, {})", step, data.damage_descr.damage, std::to_underlying(data.damage_descr.damage_type));
        case EVENT_SetSnow:
            return fmt::format("{}: SetSnow({}, {})", step, data.snow_descr.is_nop, data.snow_descr.is_enable);
        case EVENT_SetTexture:
            return fmt::format("{}: SetTexture({}, \"{}\")", step, data.sprite_texture_descr.cog, str);
        case EVENT_ShowMovie:
            return fmt::format("{}: ShowMovie(\"{}\", {})", step, trimRemoveQuotes(str), data.movie_unknown_field);
        case EVENT_SetSprite:
            return fmt::format("{}: SetSprite({}, {}, \"{}\")", step, data.sprite_texture_descr.cog, data.sprite_texture_descr.hide, str);
        case EVENT_Compare:
            return fmt::format("{}: If({}) -> {}", step, getVariableCompareStr(data.variable_descr.type, data.variable_descr.value), target_step);
        case EVENT_ChangeDoorState:
            return fmt::format("{}: ChangeDoorState({}, {})", step, data.door_descr.door_id, data.door_descr.door_new_state);
        case EVENT_Add:
            return fmt::format("{}: Add({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
        case EVENT_Substract:
            return fmt::format("{}: Sub({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
        case EVENT_Set:
            return fmt::format("{}: Set({})", step, getVariableSetStr(data.variable_descr.type, data.variable_descr.value));
        case EVENT_SummonMonsters:
            return fmt::format("{}: SummonMonsters({}, {}, {}, ({}, {}, {}), {}, {})", step, data.monster_descr.type, data.monster_descr.level, data.monster_descr.count, data.monster_descr.x, data.monster_descr.y, data.monster_descr.z, data.monster_descr.group, data.monster_descr.name_id);
        case EVENT_CastSpell:
            return fmt::format("{}: CastSpell({}, {}, {}, ({}, {}, {}), ({}, {}, {}))", step, std::to_underlying(data.spell_descr.spell_id), std::to_underlying(data.spell_descr.spell_mastery), data.spell_descr.spell_level, data.spell_descr.fromx, data.spell_descr.fromy, data.spell_descr.fromz, data.spell_descr.tox, data.spell_descr.toy, data.spell_descr.toz);
        case EVENT_SpeakNPC:
            return fmt::format("{}: SpeakNPC({})", step, data.npc_descr.npc_id);
        case EVENT_SetFacesBit:
            return fmt::format("{}: SetFacesBit({}, 0x{:x}, {})", step, data.faces_bit_descr.cog, (int)data.faces_bit_descr.face_bit, data.faces_bit_descr.is_on);
        case EVENT_ToggleActorFlag:
            return fmt::format("{}: ToggleActorFlag({}, 0x{:x}, {})", step, data.actor_flag_descr.id, (int)data.actor_flag_descr.attr, data.actor_flag_descr.is_set);
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
            }
        case EVENT_InputString:
            if (data.text_id < engine->_levelStrings.size()) {
                 return fmt::format("{}: InputString(\"{}\")", step, engine->_levelStrings[data.text_id]);
            } else {
                return fmt::format("{}: InputString({})", step, data.text_id);
            }
        case EVENT_StatusText:
            if (activeLevelDecoration) {
                return fmt::format("{}: StatusMessage(\"{}\")", step, pNPCTopics[data.text_id - 1].pText);
            } else if (data.text_id < engine->_levelStrings.size()) {
                return fmt::format("{}: StatusMessage(\"{}\")", step, engine->_levelStrings[data.text_id]);
            } else {
                return fmt::format("{}: StatusMessage({})", step, data.text_id);
            }
        case EVENT_ShowMessage:
            if (activeLevelDecoration) {
                return fmt::format("{}: ShowMessage(\"{}\")", step, pNPCTopics[data.text_id - 1].pText);
            } else if (data.text_id < engine->_levelStrings.size()) {
                return fmt::format("{}: ShowMessage(\"{}\")", step, engine->_levelStrings[data.text_id]);
            } else {
                return fmt::format("{}: ShowMessage({})", step, data.text_id);
            }
        case EVENT_OnTimer:
            return fmt::format("{}: OnTimer(Year({}), Month({}), Week({}), Day({}hr, {}min, {}sec), {})", step, data.timer_descr.is_yearly, data.timer_descr.is_monthly, data.timer_descr.is_weekly, data.timer_descr.daily_start_hour, data.timer_descr.daily_start_minute, data.timer_descr.daily_start_second, data.timer_descr.alt_halfmin_interval);
        case EVENT_ToggleIndoorLight:
            return fmt::format("{}: ToggleIndoorLight({}, {})", step, data.light_descr.light_id, data.light_descr.is_enable);
        case EVENT_PressAnyKey:
            return fmt::format("{}: PressAnyKey()", step);
        case EVENT_SummonItem:
            return fmt::format("{}: SummonItem({}, ({}, {}, {}), {}, {}, {})", step, std::to_underlying(data.summon_item_descr.sprite), data.summon_item_descr.x, data.summon_item_descr.y, data.summon_item_descr.z, data.summon_item_descr.speed, data.summon_item_descr.count, data.summon_item_descr.random_rotate);
        case EVENT_ForPartyMember:
            return fmt::format("{}: ForPartyMember({})", step, std::to_underlying(who));
        case EVENT_Jmp:
            return fmt::format("{}: Jmp -> {}", step, target_step);
        case EVENT_OnMapReload:
            return fmt::format("{}: OnMapReload", step);
        case EVENT_OnLongTimer:
            return fmt::format("{}: OnLongTimer(Year({}), Month({}), Week({}), Day({}hr, {}min, {}sec), {})", step, data.timer_descr.is_yearly, data.timer_descr.is_monthly, data.timer_descr.is_weekly, data.timer_descr.daily_start_hour, data.timer_descr.daily_start_minute, data.timer_descr.daily_start_second, data.timer_descr.alt_halfmin_interval);
        case EVENT_SetNPCTopic:
            return fmt::format("{}: SetNPCTopic({}, {}, {})", step, data.npc_topic_descr.npc_id, data.npc_topic_descr.index, data.npc_topic_descr.event_id);
        case EVENT_MoveNPC:
            return fmt::format("{}: MoveNPC({}, {})", step, data.npc_move_descr.npc_id, data.npc_move_descr.location_id);
        case EVENT_GiveItem:
            return fmt::format("{}: GiveItem({}, {}, {})", step, std::to_underlying(data.give_item_descr.treasure_level), data.give_item_descr.treasure_type, std::to_underlying(data.give_item_descr.item_id));
        case EVENT_ChangeEvent:
            return fmt::format("{}: ChangeEvent({})", step, data.event_id);
        case EVENT_CheckSkill:
             return fmt::format("{}: CheckSkill({}, {}, {}) -> {}", step, std::to_underlying(data.check_skill_descr.skill_type), std::to_underlying(data.check_skill_descr.skill_mastery), data.check_skill_descr.skill_level, target_step);
        case EVENT_OnCanShowDialogItemCmp:
            return fmt::format("{}: OnCanShowDialogItemCmp({}) -> {}", step, getVariableCompareStr(data.variable_descr.type, data.variable_descr.value), target_step);
        case EVENT_EndCanShowDialogItem:
            return fmt::format("{}: EndCanShowDialogItem", step);
        case EVENT_SetCanShowDialogItem:
            return fmt::format("{}: SetCanShowDialogItem({})", step, data.can_show_npc_dialogue);
        case EVENT_SetNPCGroupNews:
            return fmt::format("{}: SetNPCGroupNews({}, {})", step, data.npc_groups_descr.groups_id, data.npc_groups_descr.group);
        case EVENT_SetActorGroup:
            // TODO
            break;
        case EVENT_NPCSetItem:
            return fmt::format("{}: NPCSetItem({}, {}, {})", step, data.npc_item_descr.id, std::to_underlying(data.npc_item_descr.item), data.npc_item_descr.is_give);
        case EVENT_SetNPCGreeting:
            return fmt::format("{}: SetNpcGreeting({}, {})", step, data.npc_descr.npc_id, data.npc_descr.greeting);
        case EVENT_IsActorKilled:
            return fmt::format("{}: IsActorKilled({}, {}, {}) -> {}", step, std::to_underlying(data.actor_descr.policy), data.actor_descr.param, data.actor_descr.num, target_step);
        case EVENT_CanShowTopic_IsActorKilled:
            return fmt::format("{}: CanShowTopic_IsActorKilled({}, {}, {}) -> {}", step, std::to_underlying(data.actor_descr.policy), data.actor_descr.param, data.actor_descr.num, target_step);
        case EVENT_OnMapLeave:
            return fmt::format("{}: OnMapLeave", step);
        case EVENT_ChangeGroup:
            // TODO
            break;
        case EVENT_ChangeGroupAlly:
            // TODO
            break;
        case EVENT_CheckSeason:
            return fmt::format("{}: CheckSeason({}) -> {}", step, std::to_underlying(data.season), target_step);
        case EVENT_ToggleActorGroupFlag:
            return fmt::format("{}: ToggleActorGroupFlag({}, 0x{:x}, {})", step, data.actor_flag_descr.id, std::to_underlying(data.actor_flag_descr.attr), data.actor_flag_descr.is_set);
        case EVENT_ToggleChestFlag:
            return fmt::format("{}: ToggleChestFlag({}, 0x{:x}, {})", step, data.chest_flag_descr.chest_id, std::to_underlying(data.chest_flag_descr.flag), data.chest_flag_descr.is_set);
        case EVENT_CharacterAnimation:
            return fmt::format("{}: SetReaction({}, {})", step, std::to_underlying(who), std::to_underlying(data.speech_id));
        case EVENT_SetActorItem:
            return fmt::format("{}: SetActorItem({}, {}, {})", step, data.npc_item_descr.id, std::to_underlying(data.npc_item_descr.item), data.npc_item_descr.is_give);
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

EventIR EventIR::parse(const void *data, size_t maxSize) {
    _evt_raw *_evt = (_evt_raw*)data;
    int id = _evt->v1 + (_evt->v2 << 8);
    EventIR ir;

    assert(maxSize == sizeof(_evt_raw));

    ir.type = _evt->_e_type;
    ir.step = _evt->v3;

    switch (ir.type) {
        case EVENT_Exit:
            break;
        case EVENT_SpeakInHouse:
            ir.data.house_id = (HOUSE_ID)EVT_DWORD(_evt->v5);
            break;
        case EVENT_PlaySound:
            ir.data.sound_descr.sound_id = (SoundID)EVT_DWORD(_evt->v5);
            ir.data.sound_descr.x = (SoundID)EVT_DWORD(_evt->v9);
            ir.data.sound_descr.y = (SoundID)EVT_DWORD(_evt->v13);
            break;
        case EVENT_MouseOver:
            ir.data.text_id = _evt->v5;
            ir.step = -1; // Step duplicated for other command, so ignore it
            break;
        case EVENT_LocationName:
            ir.step = -1; // Step duplicated for other command, so ignore it
            break;
        case EVENT_MoveToMap:
            ir.data.move_map_descr.x = EVT_DWORD(_evt->v5);
            ir.data.move_map_descr.y = EVT_DWORD(_evt->v9);
            ir.data.move_map_descr.z = EVT_DWORD(_evt->v13);
            ir.data.move_map_descr.yaw = EVT_DWORD(_evt->v17);
            ir.data.move_map_descr.pitch = EVT_DWORD(_evt->v21);
            ir.data.move_map_descr.zspeed = EVT_DWORD(_evt->v25);
            ir.data.move_map_descr.house_id = (HOUSE_ID)_evt->v29;
            ir.data.move_map_descr.exit_pic_id = _evt->v30;
            ir.str = (char *)&_evt->v31;
            break;
        case EVENT_OpenChest:
            ir.data.chest_id = _evt->v5;
            break;
        case EVENT_ShowFace:
            ir.who = (CharacterChoosePolicy)_evt->v5;
            ir.data.expr_id = (CharacterExpressionID)_evt->v6;
            break;
        case EVENT_ReceiveDamage:
            ir.data.damage_descr.damage_type = (DAMAGE_TYPE)_evt->v6;
            ir.data.damage_descr.damage = EVT_DWORD(_evt->v7);
            break;
        case EVENT_SetSnow:
            ir.data.snow_descr.is_nop = _evt->v5;
            ir.data.snow_descr.is_enable = _evt->v6;
            break;
        case EVENT_SetTexture:
            ir.data.sprite_texture_descr.cog = EVT_DWORD(_evt->v5);
            ir.str = (char *)&_evt->v9;
            break;
        case EVENT_ShowMovie:
            ir.data.movie_unknown_field = _evt->v6;
            ir.str = (char *)&_evt->v7;
            break;
        case EVENT_SetSprite:
            ir.data.sprite_texture_descr.cog = EVT_DWORD(_evt->v5);
            ir.data.sprite_texture_descr.hide = _evt->v9;
            ir.str = (char *)&_evt->v10;
            break;
        case EVENT_Compare:
            ir.target_step = _evt->v11;
            ir.data.variable_descr.type = (VariableType)EVT_WORD(_evt->v5);
            ir.data.variable_descr.value = EVT_DWORD(_evt->v7);
            break;
        case EVENT_ChangeDoorState:
            ir.data.door_descr.door_id = _evt->v5;
            ir.data.door_descr.door_new_state = _evt->v6;
            break;
        case EVENT_Add:
        case EVENT_Substract:
        case EVENT_Set:
            ir.data.variable_descr.type = (VariableType)EVT_WORD(_evt->v5);
            ir.data.variable_descr.value = EVT_DWORD(_evt->v7);
            break;
        case EVENT_SummonMonsters:
            ir.data.monster_descr.type = _evt->v5;
            ir.data.monster_descr.level = _evt->v6;
            ir.data.monster_descr.count = _evt->v7;
            ir.data.monster_descr.x = EVT_DWORD(_evt->v8);
            ir.data.monster_descr.y = EVT_DWORD(_evt->v12);
            ir.data.monster_descr.z = EVT_DWORD(_evt->v16);
            ir.data.monster_descr.group = EVT_DWORD(_evt->v20);
            ir.data.monster_descr.name_id = EVT_DWORD(_evt->v24);
            break;
        case EVENT_CastSpell:
            ir.data.spell_descr.spell_id = (SPELL_TYPE)_evt->v5;
            ir.data.spell_descr.spell_mastery = (CharacterSkillMastery)(_evt->v6 + 1);
            ir.data.spell_descr.spell_level = _evt->v7;
            ir.data.spell_descr.fromx = EVT_DWORD(_evt->v8);
            ir.data.spell_descr.fromy = EVT_DWORD(_evt->v12);
            ir.data.spell_descr.fromz = EVT_DWORD(_evt->v16);
            ir.data.spell_descr.tox = EVT_DWORD(_evt->v20);
            ir.data.spell_descr.toy = EVT_DWORD(_evt->v24);
            ir.data.spell_descr.toz = EVT_DWORD(_evt->v28);
            break;
        case EVENT_SpeakNPC:
            ir.data.npc_descr.npc_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_SetFacesBit:
            ir.data.faces_bit_descr.cog = EVT_DWORD(_evt->v5);
            ir.data.faces_bit_descr.face_bit = (FaceAttribute)EVT_DWORD(_evt->v9);
            ir.data.faces_bit_descr.is_on = _evt->v13;
            break;
        case EVENT_ToggleActorFlag:
            ir.data.actor_flag_descr.id = EVT_DWORD(_evt->v5);
            ir.data.actor_flag_descr.attr = ActorAttribute(EVT_DWORD(_evt->v9));
            ir.data.actor_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_RandomGoTo:
            ir.data.random_goto_descr.random_goto[0] = _evt->v5;
            ir.data.random_goto_descr.random_goto[1] = _evt->v6;
            ir.data.random_goto_descr.random_goto[2] = _evt->v7;
            ir.data.random_goto_descr.random_goto[3] = _evt->v8;
            ir.data.random_goto_descr.random_goto[4] = _evt->v9;
            ir.data.random_goto_descr.random_goto[5] = _evt->v10;
            ir.data.random_goto_descr.random_goto_len = 1 + !!_evt->v6 + !!_evt->v7 + !!_evt->v8 + !!_evt->v9 + !!_evt->v10;
            break;
        case EVENT_InputString:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_StatusText:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_ShowMessage:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_OnTimer:
            ir.data.timer_descr.is_yearly = _evt->v5;
            ir.data.timer_descr.is_monthly = _evt->v6;
            ir.data.timer_descr.is_weekly = _evt->v7;
            ir.data.timer_descr.daily_start_hour = _evt->v8;
            ir.data.timer_descr.daily_start_minute = _evt->v9;
            ir.data.timer_descr.daily_start_second = _evt->v10;
            ir.data.timer_descr.alt_halfmin_interval = _evt->v11 + (_evt->v12 << 8);
            break;
        case EVENT_ToggleIndoorLight:
            ir.data.light_descr.light_id = EVT_DWORD(_evt->v5);
            ir.data.light_descr.is_enable = _evt->v9;
            break;
        case EVENT_PressAnyKey:
            // Nothing?
            break;
        case EVENT_SummonItem:
            ir.data.summon_item_descr.sprite = (SPRITE_OBJECT_TYPE)EVT_DWORD(_evt->v5);
            ir.data.summon_item_descr.x = EVT_DWORD(_evt->v9);
            ir.data.summon_item_descr.y = EVT_DWORD(_evt->v13);
            ir.data.summon_item_descr.z = EVT_DWORD(_evt->v17);
            ir.data.summon_item_descr.speed = EVT_DWORD(_evt->v21);
            ir.data.summon_item_descr.count = _evt->v25;
            ir.data.summon_item_descr.random_rotate = (bool)_evt->v26;
            break;
        case EVENT_ForPartyMember:
            ir.who = (CharacterChoosePolicy)_evt->v5;
            break;
        case EVENT_Jmp:
            ir.target_step = _evt->v5;
            break;
        case EVENT_OnMapReload:
            // Nothing?
            break;
        case EVENT_OnLongTimer:
            ir.data.timer_descr.is_yearly = _evt->v5;
            ir.data.timer_descr.is_monthly = _evt->v6;
            ir.data.timer_descr.is_weekly = _evt->v7;
            ir.data.timer_descr.daily_start_hour = _evt->v8;
            ir.data.timer_descr.daily_start_minute = _evt->v9;
            ir.data.timer_descr.daily_start_second = _evt->v10;
            ir.data.timer_descr.alt_halfmin_interval = _evt->v11 + (_evt->v12 << 8);
            break;
        case EVENT_SetNPCTopic:
            ir.data.npc_topic_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_topic_descr.index = _evt->v9;
            ir.data.npc_topic_descr.event_id = EVT_DWORD(_evt->v10);
            break;
        case EVENT_MoveNPC:
            ir.data.npc_move_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_move_descr.location_id = EVT_DWORD(_evt->v9);
            break;
        case EVENT_GiveItem:
            ir.data.give_item_descr.treasure_level = (ItemTreasureLevel)_evt->v5;
            ir.data.give_item_descr.treasure_type = _evt->v6;
            ir.data.give_item_descr.item_id = (ItemId)EVT_DWORD(_evt->v7);
            break;
        case EVENT_ChangeEvent:
            ir.data.event_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_CheckSkill:
            ir.data.check_skill_descr.skill_type = (CharacterSkillType)_evt->v5;
            ir.data.check_skill_descr.skill_mastery = (CharacterSkillMastery)_evt->v6;
            ir.data.check_skill_descr.skill_level = EVT_DWORD(_evt->v7);
            ir.target_step = _evt->v11;
            break;
        case EVENT_OnCanShowDialogItemCmp:
            ir.target_step = _evt->v11;
            ir.data.variable_descr.type = (VariableType)EVT_WORD(_evt->v5);
            ir.data.variable_descr.value = EVT_DWORD(_evt->v7);
            break;
        case EVENT_EndCanShowDialogItem:
            break;
        case EVENT_SetCanShowDialogItem:
            ir.data.can_show_npc_dialogue = EVT_BYTE(_evt->v5);
            break;
        case EVENT_SetNPCGroupNews:
            ir.data.npc_groups_descr.groups_id = EVT_DWORD(_evt->v5);
            ir.data.npc_groups_descr.group = EVT_DWORD(_evt->v9);
            break;
        case EVENT_SetActorGroup:
            // TODO
            break;
        case EVENT_NPCSetItem:
            ir.data.npc_item_descr.id = EVT_DWORD(_evt->v5);
            ir.data.npc_item_descr.item = (ItemId)EVT_DWORD(_evt->v9);
            ir.data.npc_item_descr.is_give = _evt->v13;
            break;
        case EVENT_SetNPCGreeting:
            ir.data.npc_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_descr.greeting = EVT_DWORD(_evt->v9);
            break;
        case EVENT_IsActorKilled:
            ir.data.actor_descr.policy = (ACTOR_KILL_CHECK_POLICY)_evt->v5;
            ir.data.actor_descr.param = EVT_DWORD(_evt->v6);
            ir.data.actor_descr.num = _evt->v10;
            ir.target_step = _evt->v11;
            break;
        case EVENT_CanShowTopic_IsActorKilled:
            ir.data.actor_descr.policy = (ACTOR_KILL_CHECK_POLICY)_evt->v5;
            ir.data.actor_descr.param = EVT_DWORD(_evt->v6);
            ir.data.actor_descr.num = _evt->v10;
            ir.target_step = _evt->v11;
            break;
        case EVENT_OnMapLeave:
            // Nothing?
            break;
        case EVENT_ChangeGroup:
            // TODO
            break;
        case EVENT_ChangeGroupAlly:
            // TODO
            break;
        case EVENT_CheckSeason:
            ir.data.season = (Season)_evt->v5;
            ir.target_step = _evt->v6;
            break;
        case EVENT_ToggleActorGroupFlag:
            ir.data.actor_flag_descr.id = EVT_DWORD(_evt->v5);
            ir.data.actor_flag_descr.attr = ActorAttribute(EVT_DWORD(_evt->v9));
            ir.data.actor_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_ToggleChestFlag:
            ir.data.chest_flag_descr.chest_id = EVT_DWORD(_evt->v5);
            ir.data.chest_flag_descr.flag = (ChestFlag)EVT_DWORD(_evt->v9);
            ir.data.chest_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_CharacterAnimation:
            ir.who = (CharacterChoosePolicy)_evt->v5;
            ir.data.speech_id = (CharacterSpeech)_evt->v6;
            break;
        case EVENT_SetActorItem:
            ir.data.npc_item_descr.id = EVT_DWORD(_evt->v5);
            ir.data.npc_item_descr.item = (ItemId)EVT_DWORD(_evt->v9);
            ir.data.npc_item_descr.is_give = _evt->v13;
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

    return ir;
}
