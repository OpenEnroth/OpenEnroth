#include "EvtEnums.h"

#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "Library/Serialization/CommonSerialization.h"
#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS(EvtOpcode)

namespace {

/**
 * Variables that MMExtension names with a prefix and a number, e.g. "MapVar0" to "MapVar99".
 */
struct NumberedVariables {
    EvtVariable first;
    EvtVariable last;
    std::string_view prefix;
    int firstNumber = 0;
};

constexpr std::array<NumberedVariables, 4> numberedVariables = {{
    {VAR_MapPersistentVariable_0, VAR_MapPersistentDecorVariable_24, "MapVar", 0}, // The decoration variables are the last 25 map variables.
    {VAR_Counter1, VAR_Counter10, "Counter", 1},
    {VAR_UnknownTimeEvent0, VAR_UnknownTimeEvent19, "SpecialDate", 1},
    {VAR_History_0, VAR_History_28, "History", 1},
}};

const detail::EnumSerializer<EvtVariable> namedVariables(CASE_SENSITIVE, {
    {VAR_Sex, "SexIs"},
    {VAR_Class, "ClassIs"},
    {VAR_CurrentHP, "HP"},
    {VAR_MaxHP, "HasFullHP"},
    {VAR_CurrentSP, "SP"},
    {VAR_MaxSP, "HasFullSP"},
    {VAR_ActualAC, "ArmorClass"},
    {VAR_ACModifier, "ArmorClassBonus"},
    {VAR_BaseLevel, "BaseLevel"},
    {VAR_LevelModifier, "LevelBonus"},
    {VAR_Age, "AgeBonus"},
    {VAR_Award, "Awards"},
    {VAR_Experience, "Experience"},
    {VAR_QBits_QuestsDone, "QBits"},
    {VAR_PlayerItemInHands, "Inventory"},
    {VAR_Hour, "HourIs"},
    {VAR_DayOfYear, "DayOfYearIs"},
    {VAR_DayOfWeek, "DayOfWeekIs"},
    {VAR_FixedGold, "Gold"},
    {VAR_RandomGold, "GoldAddRandom"},
    {VAR_FixedFood, "Food"},
    {VAR_RandomFood, "FoodAddRandom"},
    {VAR_MightBonus, "MightBonus"},
    {VAR_IntellectBonus, "IntellectBonus"},
    {VAR_PersonalityBonus, "PersonalityBonus"},
    {VAR_EnduranceBonus, "EnduranceBonus"},
    {VAR_SpeedBonus, "SpeedBonus"},
    {VAR_AccuracyBonus, "AccuracyBonus"},
    {VAR_LuckBonus, "LuckBonus"},
    {VAR_BaseMight, "BaseMight"},
    {VAR_BaseIntellect, "BaseIntellect"},
    {VAR_BasePersonality, "BasePersonality"},
    {VAR_BaseEndurance, "BaseEndurance"},
    {VAR_BaseSpeed, "BaseSpeed"},
    {VAR_BaseAccuracy, "BaseAccuracy"},
    {VAR_BaseLuck, "BaseLuck"},
    {VAR_ActualMight, "CurrentMight"},
    {VAR_ActualIntellect, "CurrentIntellect"},
    {VAR_ActualPersonality, "CurrentPersonality"},
    {VAR_ActualEndurance, "CurrentEndurance"},
    {VAR_ActualSpeed, "CurrentSpeed"},
    {VAR_ActualAccuracy, "CurrentAccuracy"},
    {VAR_ActualLuck, "CurrentLuck"},
    {VAR_FireResistance, "FireResistance"},
    {VAR_AirResistance, "AirResistance"},
    {VAR_WaterResistance, "WaterResistance"},
    {VAR_EarthResistance, "EarthResistance"},
    {VAR_SpiritResistance, "SpiritResistance"},
    {VAR_MindResistance, "MindResistance"},
    {VAR_BodyResistance, "BodyResistance"},
    {VAR_LightResistance, "LightResistance"},
    {VAR_DarkResistance, "DarkResistance"},
    {VAR_MagicResistance, "MagicResistance"},
    {VAR_FireResistanceBonus, "FireResBonus"},
    {VAR_AirResistanceBonus, "AirResBonus"},
    {VAR_WaterResistanceBonus, "WaterResBonus"},
    {VAR_EarthResistanceBonus, "EarthResBonus"},
    {VAR_SpiritResistanceBonus, "SpiritResBonus"},
    {VAR_MindResistanceBonus, "MindResBonus"},
    {VAR_BodyResistanceBonus, "BodyResBonus"},
    {VAR_LightResistanceBonus, "LightResBonus"},
    {VAR_DarkResistanceBonus, "DarkResBonus"},
    {VAR_MagicResistanceBonus, "MagicResBonus"},
    {VAR_StaffSkill, "StaffSkill"},
    {VAR_SwordSkill, "SwordSkill"},
    {VAR_DaggerSkill, "DaggerSkill"},
    {VAR_AxeSkill, "AxeSkill"},
    {VAR_SpearSkill, "SpearSkill"},
    {VAR_BowSkill, "BowSkill"},
    {VAR_MaceSkill, "MaceSkill"},
    {VAR_BlasterSkill, "BlasterSkill"},
    {VAR_ShieldSkill, "ShieldSkill"},
    {VAR_LeatherSkill, "LeatherSkill"},
    {VAR_SkillChain, "ChainSkill"},
    {VAR_PlateSkill, "PlateSkill"},
    {VAR_FireSkill, "FireSkill"},
    {VAR_AirSkill, "AirSkill"},
    {VAR_WaterSkill, "WaterSkill"},
    {VAR_EarthSkill, "EarthSkill"},
    {VAR_SpiritSkill, "SpiritSkill"},
    {VAR_MindSkill, "MindSkill"},
    {VAR_BodySkill, "BodySkill"},
    {VAR_LightSkill, "LightSkill"},
    {VAR_DarkSkill, "DarkSkill"},
    {VAR_IdentifyItemSkill, "IdentifyItemSkill"},
    {VAR_MerchantSkill, "MerchantSkill"},
    {VAR_RepairSkill, "RepairSkill"},
    {VAR_BodybuildingSkill, "BodybuildingSkill"},
    {VAR_MeditationSkill, "MeditationSkill"},
    {VAR_PerceptionSkill, "PerceptionSkill"},
    {VAR_DiplomacySkill, "DiplomacySkill"},
    {VAR_ThieverySkill, "ThieverySkill"},
    {VAR_DisarmTrapSkill, "DisarmTrapSkill"},
    {VAR_DodgeSkill, "DodgingSkill"},
    {VAR_UnarmedSkill, "UnarmedSkill"},
    {VAR_IdentifyMonsterSkill, "IdentifyMonsterSkill"},
    {VAR_ArmsmasterSkill, "ArmsmasterSkill"},
    {VAR_StealingSkill, "StealingSkill"},
    {VAR_AlchemySkill, "AlchemySkill"},
    {VAR_LearningSkill, "LearningSkill"},
    {VAR_Cursed, "Cursed"},
    {VAR_Weak, "Weak"},
    {VAR_Asleep, "Asleep"},
    {VAR_Afraid, "Afraid"},
    {VAR_Drunk, "Drunk"},
    {VAR_Insane, "Insane"},
    {VAR_PoisonedGreen, "PoisonedGreen"},
    {VAR_DiseasedGreen, "DiseasedGreen"},
    {VAR_PoisonedYellow, "PoisonedYellow"},
    {VAR_DiseasedYellow, "DiseasedYellow"},
    {VAR_PoisonedRed, "PoisonedRed"},
    {VAR_DiseasedRed, "DiseasedRed"},
    {VAR_Paralyzed, "Paralyzed"},
    {VAR_Unconsious, "Unconscious"},
    {VAR_Dead, "Dead"},
    {VAR_Stoned, "Stoned"},
    {VAR_Eradicated, "Eradicated"},
    {VAR_MajorCondition, "MainCondition"},
    {VAR_AutoNotes, "AutonotesBits"},
    {VAR_IsMightMoreThanBase, "IsMightMoreThanBase"},
    {VAR_IsIntellectMoreThanBase, "IsIntellectMoreThanBase"},
    {VAR_IsPersonalityMoreThanBase, "IsPersonalityMoreThanBase"},
    {VAR_IsEnduranceMoreThanBase, "IsEnduranceMoreThanBase"},
    {VAR_IsSpeedMoreThanBase, "IsSpeedMoreThanBase"},
    {VAR_IsAccuracyMoreThanBase, "IsAccuracyMoreThanBase"},
    {VAR_IsLuckMoreThanBase, "IsLuckMoreThanBase"},
    {VAR_PlayerBits, "PlayerBits"},
    {VAR_NPCs2, "NPCs"},
    {VAR_IsFlying, "Flying"},
    {VAR_HiredNPCHasSpeciality, "HasNPCProfession"},
    {VAR_CircusPrizes, "TotalCircusPrize"},
    {VAR_NumSkillPoints, "SkillPoints"},
    {VAR_MonthIs, "MonthIs"},
    {VAR_ReputationInCurrentLocation, "Reputation"},
    {VAR_AlertStatus, "MapAlert"},
    {VAR_GoldInBank, "BankGold"},
    {VAR_NumDeaths, "Deaths"},
    {VAR_NumBounties, "MontersHunted"}, // MMExtension's spelling.
    {VAR_PrisonTerms, "PrisonTerms"},
    {VAR_ArenaWinsPage, "ArenaWinsPage"},
    {VAR_ArenaWinsSquire, "ArenaWinsSquire"},
    {VAR_ArenaWinsKnight, "ArenaWinsKnight"},
    {VAR_ArenaWinsLord, "ArenaWinsLord"},
    {VAR_Invisible, "Invisible"},
    {VAR_ItemEquipped, "IsWearingItem"}
});

} // namespace

bool trySerialize(const EvtVariable &src, std::string *dst) {
    for (const NumberedVariables &variables : numberedVariables) {
        if (src >= variables.first && src <= variables.last) {
            *dst = std::string(variables.prefix) + std::to_string(variables.firstNumber + std::to_underlying(src) - std::to_underlying(variables.first));
            return true;
        }
    }
    return namedVariables.trySerialize(src, dst);
}

bool tryDeserialize(std::string_view src, EvtVariable *dst) {
    for (const NumberedVariables &variables : numberedVariables) {
        int number = 0;
        if (!src.starts_with(variables.prefix) || !tryDeserialize(src.substr(variables.prefix.size()), &number))
            continue;

        EvtVariable result = static_cast<EvtVariable>(std::to_underlying(variables.first) + number - variables.firstNumber);
        if (result < variables.first || result > variables.last || toString(result) != src) // "MapVar01" isn't a name.
            return false;
        *dst = result;
        return true;
    }
    return namedVariables.tryDeserialize(src, dst);
}

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_VIA_TRY_FUNCTIONS_I(EvtVariable)
