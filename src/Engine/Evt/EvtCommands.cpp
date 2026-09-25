#include "EvtCommands.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Utility/Exception.h"
#include "Utility/MapAccess.h"

/**
 * @param type                          Type of a field.
 * @return                              Smallest and largest value that a script can pass for a field of the given
 *                                      type that is stored as `T`.
 */
template<class T>
static std::pair<int64_t, int64_t> fieldRange(EvtFieldType type) {
    if (type == EVT_FIELD_MASTERY)
        return {std::to_underlying(MASTERY_NOVICE), std::to_underlying(MASTERY_GRANDMASTER)};
    if (type == EVT_FIELD_PLAYER)
        return {std::to_underlying(CHOOSE_PLAYER1), std::to_underlying(CHOOSE_RANDOM)};
    if (type == EVT_FIELD_BOOL)
        return {0, 1};

    using Stored = typename std::conditional_t<std::is_enum_v<T>, std::underlying_type<T>, std::type_identity<T>>::type;
    if constexpr (std::is_same_v<Stored, bool>) {
        return {0, 1};
    } else {
        return {std::numeric_limits<Stored>::min(), std::numeric_limits<Stored>::max()};
    }
}

/**
 * @param name                          MMExtension's name of the field.
 * @param type                          What scripts pass for it.
 * @param access                        Takes an `EvtInstruction`, const or not, and returns the member that holds the
 *                                      field.
 * @param constGroup                    Group of constants that the field's values are named by.
 * @return                              The field.
 */
template<class Access>
static EvtFieldInfo field(std::string_view name, EvtFieldType type, Access access, EvtConstGroup constGroup = EVT_CONST_NONE) {
    using T = std::remove_cvref_t<decltype(access(std::declval<EvtInstruction &>()))>;

    EvtFieldInfo result;
    result.name = name;
    result.type = type;
    result.constGroup = constGroup;
    result.get = [access](const EvtInstruction &ir) -> EvtFieldValue {
        const T &value = access(ir);
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_enum_v<T>) {
            return static_cast<int64_t>(std::to_underlying(value));
        } else {
            return static_cast<int64_t>(value);
        }
    };
    result.set = [access, name, type](EvtInstruction *ir, const EvtFieldValue &value) {
        T &target = access(*ir);
        if constexpr (std::is_same_v<T, std::string>) {
            const std::string *string = std::get_if<std::string>(&value);
            if (!string)
                throw Exception("field {} takes a string", name);
            target = *string;
        } else {
            const int64_t *number = std::get_if<int64_t>(&value);
            if (!number)
                throw Exception("field {} takes a number", name);
            auto [min, max] = fieldRange<T>(type);
            if (*number < min || *number > max)
                throw Exception("field {} takes {} to {}, got {}", name, min, max, *number);
            target = static_cast<T>(*number);
        }
    };
    return result;
}

/**
 * @param name                          MMExtension's name of a field that OpenEnroth doesn't keep.
 * @return                              The field. Scripts can pass it, and it's dropped.
 */
static EvtFieldInfo ignoredField(std::string_view name) {
    EvtFieldInfo result;
    result.name = name;
    return result;
}

#define EVT_ACCESS(member) [](auto &ir) -> auto & { return ir.member; }

static std::vector<EvtCommandInfo> makeCommands() {
    const EvtFieldInfo who = field("Player", EVT_FIELD_PLAYER, EVT_ACCESS(who));
    const EvtFieldInfo variable = field("VarNum", EVT_FIELD_VARIABLE, EVT_ACCESS(data.variable_descr.type));
    const EvtFieldInfo value = field("Value", EVT_FIELD_INT, EVT_ACCESS(data.variable_descr.value));
    const EvtFieldInfo name = field("Name", EVT_FIELD_STRING, EVT_ACCESS(str));

    return {
        {EVENT_SpeakInHouse, "EnterHouse", false, {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.house_id))}},
        {EVENT_PlaySound, "PlaySound", false,
         {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.sound_descr.sound_id)), field("X", EVT_FIELD_INT, EVT_ACCESS(data.sound_descr.x)),
          field("Y", EVT_FIELD_INT, EVT_ACCESS(data.sound_descr.y))}},
        {EVENT_MoveToMap, "MoveToMap", false,
         {field("X", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.x)), field("Y", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.y)),
          field("Z", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.z)), field("Direction", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.yaw)),
          field("LookAngle", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.pitch)),
          field("SpeedZ", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.zspeed)),
          field("HouseId", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.house_id)),
          field("Icon", EVT_FIELD_INT, EVT_ACCESS(data.move_map_descr.exit_pic_id)), name}},
        {EVENT_OpenChest, "OpenChest", false, {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.chest_id))}},
        {EVENT_ShowFace, "FaceExpression", false, {who, field("Frame", EVT_FIELD_INT, EVT_ACCESS(data.portrait_id))}},
        {EVENT_ReceiveDamage, "DamagePlayer", false,
         {who, field("DamageType", EVT_FIELD_INT, EVT_ACCESS(data.damage_descr.damage_type)),
          field("Damage", EVT_FIELD_INT, EVT_ACCESS(data.damage_descr.damage))}},
        {EVENT_SetSnow, "SetSnow", false,
         {field("EffectId", EVT_FIELD_INT, EVT_ACCESS(data.snow_descr.is_nop)), field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.snow_descr.is_enable))}},
        {EVENT_SetTexture, "SetTexture", false, {field("Facet", EVT_FIELD_INT, EVT_ACCESS(data.sprite_texture_descr.cog)), name}},
        {EVENT_ShowMovie, "ShowMovie", false,
         {ignoredField("DoubleSize"), field("ExitCurrentScreen", EVT_FIELD_BOOL, EVT_ACCESS(data.movie_unknown_field)), name}},
        {EVENT_SetSprite, "SetSprite", false,
         {field("SpriteId", EVT_FIELD_INT, EVT_ACCESS(data.sprite_texture_descr.cog)),
          field("Visible", EVT_FIELD_INT, EVT_ACCESS(data.sprite_texture_descr.hide)), name}},
        {EVENT_Compare, "Cmp", true, {variable, value}},
        {EVENT_ChangeDoorState, "SetDoorState", false,
         {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.door_descr.door_id)), field("State", EVT_FIELD_INT, EVT_ACCESS(data.door_descr.door_action))}},
        {EVENT_Add, "Add", false, {variable, value}},
        {EVENT_Subtract, "Subtract", false, {variable, value}},
        {EVENT_Set, "Set", false, {variable, value}},
        {EVENT_SummonMonsters, "SummonMonsters", false,
         {field("TypeIndexInMapStats", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.type)),
          field("Level", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.level)), field("Count", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.count)),
          field("X", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.x)), field("Y", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.y)),
          field("Z", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.z)), field("NPCGroup", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.group)),
          field("unk", EVT_FIELD_INT, EVT_ACCESS(data.monster_descr.name_id))}},
        {EVENT_CastSpell, "CastSpell", false,
         {field("Spell", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.spell_id)),
          field("Mastery", EVT_FIELD_MASTERY, EVT_ACCESS(data.spell_descr.spell_mastery)),
          field("Skill", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.spell_level)), field("FromX", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.fromx)),
          field("FromY", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.fromy)), field("FromZ", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.fromz)),
          field("ToX", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.tox)), field("ToY", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.toy)),
          field("ToZ", EVT_FIELD_INT, EVT_ACCESS(data.spell_descr.toz))}},
        {EVENT_SpeakNPC, "SpeakNPC", false, {field("NPC", EVT_FIELD_INT, EVT_ACCESS(data.npc_descr.npc_id))}},
        {EVENT_SetFacesBit, "SetFacetBit", false,
         {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.faces_bit_descr.cog)),
          field("Bit", EVT_FIELD_INT, EVT_ACCESS(data.faces_bit_descr.face_bit), EVT_CONST_FACET_BITS),
          field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.faces_bit_descr.is_on))}},
        {EVENT_ToggleActorFlag, "SetMonsterBit", false,
         {field("Monster", EVT_FIELD_INT, EVT_ACCESS(data.actor_flag_descr.id)),
          field("Bit", EVT_FIELD_INT, EVT_ACCESS(data.actor_flag_descr.attr), EVT_CONST_MONSTER_BITS),
          field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.actor_flag_descr.is_set))}},
        {EVENT_StatusText, "StatusText", false, {field("Str", EVT_FIELD_INT, EVT_ACCESS(data.text_id))}},
        {EVENT_ShowMessage, "SetMessage", false, {field("Str", EVT_FIELD_INT, EVT_ACCESS(data.text_id))}},
        {EVENT_ToggleIndoorLight, "SetLight", false,
         {field("Id", EVT_FIELD_INT, EVT_ACCESS(data.light_descr.light_id)), field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.light_descr.is_enable))}},
        {EVENT_SummonItem, "SummonObject", false,
         {field("Type", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.sprite)), field("X", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.x)),
          field("Y", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.y)), field("Z", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.z)),
          field("Speed", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.speed)),
          field("Count", EVT_FIELD_INT, EVT_ACCESS(data.summon_item_descr.count)),
          field("RandomAngle", EVT_FIELD_BOOL, EVT_ACCESS(data.summon_item_descr.random_rotate))}},
        {EVENT_SetNPCTopic, "SetNPCTopic", false,
         {field("NPC", EVT_FIELD_INT, EVT_ACCESS(data.npc_topic_descr.npc_id)), field("Index", EVT_FIELD_INT, EVT_ACCESS(data.npc_topic_descr.index)),
          field("Event", EVT_FIELD_INT, EVT_ACCESS(data.npc_topic_descr.event_id))}},
        {EVENT_MoveNPC, "MoveNPC", false,
         {field("NPC", EVT_FIELD_INT, EVT_ACCESS(data.npc_move_descr.npc_id)),
          field("HouseId", EVT_FIELD_INT, EVT_ACCESS(data.npc_move_descr.location_id))}},
        {EVENT_GiveItem, "GiveItem", false,
         {field("Strength", EVT_FIELD_INT, EVT_ACCESS(data.give_item_descr.treasure_level)),
          field("Type", EVT_FIELD_INT, EVT_ACCESS(data.give_item_descr.treasure_type)),
          field("Id", EVT_FIELD_INT, EVT_ACCESS(data.give_item_descr.item_id))}},
        {EVENT_ChangeEvent, "ChangeEvent", false, {field("NewEvent", EVT_FIELD_INT, EVT_ACCESS(data.event_id))}},
        {EVENT_CheckSkill, "CheckSkill", true,
         {field("Skill", EVT_FIELD_INT, EVT_ACCESS(data.check_skill_descr.skill_type)),
          field("Mastery", EVT_FIELD_MASTERY, EVT_ACCESS(data.check_skill_descr.skill_mastery)),
          field("Level", EVT_FIELD_INT, EVT_ACCESS(data.check_skill_descr.skill_level))}},
        {EVENT_SetNPCGroupNews, "SetNPCGroupNews", false,
         {field("NPCGroup", EVT_FIELD_INT, EVT_ACCESS(data.npc_groups_descr.groups_id)),
          field("NPCNews", EVT_FIELD_INT, EVT_ACCESS(data.npc_groups_descr.group))}},
        {EVENT_NPCSetItem, "SetNPCItem", false,
         {field("NPC", EVT_FIELD_INT, EVT_ACCESS(data.npc_item_descr.id)), field("Item", EVT_FIELD_INT, EVT_ACCESS(data.npc_item_descr.item)),
          field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.npc_item_descr.is_give))}},
        {EVENT_SetNPCGreeting, "SetNPCGreeting", false,
         {field("NPC", EVT_FIELD_INT, EVT_ACCESS(data.npc_descr.npc_id)), field("Greeting", EVT_FIELD_INT, EVT_ACCESS(data.npc_descr.greeting))}},
        {EVENT_IsActorKilled, "CheckMonstersKilled", true,
         {field("CheckType", EVT_FIELD_INT, EVT_ACCESS(data.actor_descr.policy)), field("Id", EVT_FIELD_INT, EVT_ACCESS(data.actor_descr.param)),
          field("Count", EVT_FIELD_INT, EVT_ACCESS(data.actor_descr.num))}},
        {EVENT_CheckSeason, "CheckSeason", true, {field("Season", EVT_FIELD_INT, EVT_ACCESS(data.season))}},
        {EVENT_ToggleActorGroupFlag, "SetMonGroupBit", false,
         {field("NPCGroup", EVT_FIELD_INT, EVT_ACCESS(data.actor_flag_descr.id)),
          field("Bit", EVT_FIELD_INT, EVT_ACCESS(data.actor_flag_descr.attr), EVT_CONST_MONSTER_BITS),
          field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.actor_flag_descr.is_set))}},
        {EVENT_ToggleChestFlag, "SetChestBit", false,
         {field("ChestId", EVT_FIELD_INT, EVT_ACCESS(data.chest_flag_descr.chest_id)),
          field("Bit", EVT_FIELD_INT, EVT_ACCESS(data.chest_flag_descr.flag), EVT_CONST_CHEST_BITS),
          field("On", EVT_FIELD_BOOL, EVT_ACCESS(data.chest_flag_descr.is_set))}},
        {EVENT_CharacterAnimation, "FaceAnimation", false, {who, field("Animation", EVT_FIELD_INT, EVT_ACCESS(data.speech_id))}},
        {EVENT_SetActorItem, "SetMonsterItem", false,
         {field("Monster", EVT_FIELD_INT, EVT_ACCESS(data.npc_item_descr.id)), field("Item", EVT_FIELD_INT, EVT_ACCESS(data.npc_item_descr.item)),
          field("Has", EVT_FIELD_BOOL, EVT_ACCESS(data.npc_item_descr.is_give))}},
    };
}

#undef EVT_ACCESS

static const std::vector<EvtCommandInfo> &commands() {
    static const std::vector<EvtCommandInfo> result = makeCommands();
    return result;
}

/**
 * MMExtension's names of the evt variables, both ways.
 */
struct VariableNames {
    std::unordered_map<int, std::string> nameByVariable; // The first name given to a variable is the one it's shown with.
    std::unordered_map<std::string, int> variableByName;

    void add(int variable, std::string name) {
        nameByVariable.try_emplace(variable, name);
        [[maybe_unused]] bool isNew = variableByName.emplace(std::move(name), variable).second;
        assert(isNew);
    }

    void addRange(int firstVariable, std::string_view prefix, int firstIndex, int count) {
        for (int i = 0; i < count; i++)
            add(firstVariable + i, fmt::format("{}{}", prefix, firstIndex + i));
    }
};

static VariableNames makeVariableNames() {
    VariableNames result;

    int variable = std::to_underlying(VAR_Sex);
    for (std::string_view name : {"SexIs", "ClassIs", "HP", "HasFullHP", "SP", "HasFullSP", "ArmorClass", "ArmorClassBonus", "BaseLevel", "LevelBonus",
                                  "AgeBonus", "Awards", "Experience"})
        result.add(variable++, std::string(name));

    variable = std::to_underlying(VAR_QBits_QuestsDone);
    for (std::string_view name : {"QBits", "Inventory", "HourIs", "DayOfYearIs", "DayOfWeekIs", "Gold", "GoldAddRandom", "Food", "FoodAddRandom",
                                  "MightBonus", "IntellectBonus", "PersonalityBonus", "EnduranceBonus", "SpeedBonus", "AccuracyBonus", "LuckBonus",
                                  "BaseMight", "BaseIntellect", "BasePersonality", "BaseEndurance", "BaseSpeed", "BaseAccuracy", "BaseLuck",
                                  "CurrentMight", "CurrentIntellect", "CurrentPersonality", "CurrentEndurance", "CurrentSpeed", "CurrentAccuracy",
                                  "CurrentLuck", "FireResistance", "AirResistance", "WaterResistance", "EarthResistance", "SpiritResistance",
                                  "MindResistance", "BodyResistance", "LightResistance", "DarkResistance"})
        result.add(variable++, std::string(name));
    result.add(std::to_underlying(VAR_MagicResistance), "MagicResistance");

    variable = std::to_underlying(VAR_FireResistanceBonus);
    for (std::string_view name : {"FireResBonus", "AirResBonus", "WaterResBonus", "EarthResBonus", "SpiritResBonus", "MindResBonus", "BodyResBonus",
                                  "LightResBonus", "DarkResBonus"})
        result.add(variable++, std::string(name));
    result.add(std::to_underlying(VAR_MagicResistanceBonus), "MagicResBonus");

    variable = std::to_underlying(VAR_StaffSkill);
    for (std::string_view name : {"StaffSkill", "SwordSkill", "DaggerSkill", "AxeSkill", "SpearSkill", "BowSkill", "MaceSkill", "BlasterSkill",
                                  "ShieldSkill", "LeatherSkill", "ChainSkill", "PlateSkill", "FireSkill", "AirSkill", "WaterSkill", "EarthSkill",
                                  "SpiritSkill", "MindSkill", "BodySkill", "LightSkill", "DarkSkill", "IdentifyItemSkill", "MerchantSkill",
                                  "RepairSkill", "BodybuildingSkill", "MeditationSkill", "PerceptionSkill", "DiplomacySkill", "ThieverySkill",
                                  "DisarmTrapSkill", "DodgingSkill", "UnarmedSkill", "IdentifyMonsterSkill", "ArmsmasterSkill", "StealingSkill",
                                  "AlchemySkill", "LearningSkill", "Cursed", "Weak", "Asleep", "Afraid", "Drunk", "Insane", "PoisonedGreen",
                                  "DiseasedGreen", "PoisonedYellow", "DiseasedYellow", "PoisonedRed", "DiseasedRed", "Paralyzed", "Unconscious",
                                  "Dead", "Stoned", "Eradicated", "MainCondition"})
        result.add(variable++, std::string(name));

    result.addRange(std::to_underlying(VAR_MapPersistentVariable_0), "MapVar", 0, 100); // The last 25 are OpenEnroth's decoration variables.

    variable = std::to_underlying(VAR_AutoNotes);
    for (std::string_view name : {"AutonotesBits", "IsMightMoreThanBase", "IsIntellectMoreThanBase", "IsPersonalityMoreThanBase",
                                  "IsEnduranceMoreThanBase", "IsSpeedMoreThanBase", "IsAccuracyMoreThanBase", "IsLuckMoreThanBase", "PlayerBits",
                                  "NPCs", "ReputationIs"})
        result.add(variable++, std::string(name));
    result.addRange(variable, "DaysCounter", 1, 6);

    variable = std::to_underlying(VAR_IsFlying);
    for (std::string_view name : {"Flying", "HasNPCProfession", "TotalCircusPrize", "SkillPoints", "MonthIs"})
        result.add(variable++, std::string(name));
    result.addRange(std::to_underlying(VAR_Counter1), "Counter", 1, 10);
    result.addRange(std::to_underlying(VAR_UnknownTimeEvent0), "SpecialDate", 1, 20);
    result.add(std::to_underlying(VAR_ReputationInCurrentLocation), "Reputation");
    result.addRange(std::to_underlying(VAR_History_0), "History", 1, 29);

    variable = std::to_underlying(VAR_AlertStatus);
    for (std::string_view name : {"MapAlert", "BankGold", "Deaths", "MontersHunted", "PrisonTerms", "ArenaWinsPage", "ArenaWinsSquire",
                                  "ArenaWinsKnight", "ArenaWinsLord", "Invisible", "IsWearingItem"}) // "MontersHunted" is MMExtension's spelling.
        result.add(variable++, std::string(name));

    for (auto [alias, name] : std::initializer_list<std::pair<std::string_view, std::string_view>>{
             {"AC", "ArmorClass"}, {"ACBonus", "ArmorClassBonus"}, {"Exp", "Experience"}, {"Items", "Inventory"}, {"RepairItemSkill", "RepairSkill"},
             {"DisarmTrapsSkill", "DisarmTrapSkill"}, {"Poison1", "PoisonedGreen"}, {"Disease1", "DiseasedGreen"}, {"Poison2", "PoisonedYellow"},
             {"Disease2", "DiseasedYellow"}, {"Poison3", "PoisonedRed"}, {"Disease3", "DiseasedRed"}, {"Paralysed", "Paralyzed"}})
        result.add(result.variableByName.at(std::string(name)), std::string(alias));

    return result;
}

static const VariableNames &variableNames() {
    static const VariableNames result = makeVariableNames();
    return result;
}

std::span<const EvtCommandInfo> evtCommands() {
    return commands();
}

const EvtCommandInfo *evtCommand(EvtOpcode opcode) {
    auto pos = std::ranges::find(commands(), opcode, &EvtCommandInfo::opcode);
    return pos == commands().end() ? nullptr : &*pos;
}

const EvtCommandInfo *evtCommand(std::string_view name) {
    if (name == "Sub")
        name = "Subtract";
    auto pos = std::ranges::find(commands(), name, &EvtCommandInfo::name);
    return pos == commands().end() ? nullptr : &*pos;
}

std::string evtVariableName(EvtVariable variable) {
    return valueOr(variableNames().nameByVariable, std::to_underlying(variable), std::string());
}

std::optional<EvtVariable> evtVariableByName(std::string_view name) {
    const int *result = valuePtr(variableNames().variableByName, std::string(name));
    return result ? std::optional(static_cast<EvtVariable>(*result)) : std::nullopt;
}

std::string_view evtConstGroupName(EvtConstGroup group) {
    switch (group) {
        case EVT_CONST_FACET_BITS: return "FacetBits";
        case EVT_CONST_MONSTER_BITS: return "MonsterBits";
        case EVT_CONST_CHEST_BITS: return "ChestBits";
        default: return {};
    }
}

std::span<const std::pair<std::string_view, int64_t>> evtConstants(EvtConstGroup group) {
    static const std::vector<std::pair<std::string_view, int64_t>> facetBits = {
        {"IsPortal", 0x1}, {"IsSecret", 0x2}, {"ScrollDown", 0x4}, {"AlignTop", 0x8}, {"IsWater", 0x10}, {"ScrollUp", 0x20}, {"ScrollLeft", 0x40},
        {"ProjectToXY", 0x100}, {"ProjectToXZ", 0x200}, {"ProjectToYZ", 0x400}, {"ScrollRight", 0x800}, {"AlignLeft", 0x1000}, {"Invisible", 0x2000},
        {"AnimatedTFT", 0x4000}, {"AlignRight", 0x8000}, {"AlignBottom", 0x20000}, {"MoveByDoor", 0x40000}, {"IsEventJustHint", 0x100000},
        {"AlternativeSound", 0x200000}, {"IsSky", 0x400000}, {"FlipU", 0x800000}, {"FlipV", 0x1000000}, {"TriggerByClick", 0x2000000},
        {"TriggerByStep", 0x4000000}, {"TriggerByMonster", 0x8000000}, {"TriggerByObject", 0x10000000}, {"Untouchable", 0x20000000},
        {"IsLava", 0x40000000}, {"HasData", 0x80000000}};
    static const std::vector<std::pair<std::string_view, int64_t>> monsterBits = {
        {"Active", 0x400}, {"ShowOnMap", 0x8000}, {"Invisible", 0x10000}, {"NoFlee", 0x20000}, {"Hostile", 0x80000}, {"OnAlertMap", 0x100000},
        {"TreasureGenerated", 0x800000}, {"ShowAsHostile", 0x1000000}};
    static const std::vector<std::pair<std::string_view, int64_t>> chestBits = {{"Trapped", 1}, {"ItemsPlaced", 2}, {"Identified", 4}};

    switch (group) {
        case EVT_CONST_FACET_BITS: return facetBits;
        case EVT_CONST_MONSTER_BITS: return monsterBits;
        case EVT_CONST_CHEST_BITS: return chestBits;
        default: return {};
    }
}

std::string_view evtPlayerName(EvtTargetCharacter target) {
    switch (target) {
        case CHOOSE_PLAYER1: return "0";
        case CHOOSE_PLAYER2: return "1";
        case CHOOSE_PLAYER3: return "2";
        case CHOOSE_PLAYER4: return "3";
        case CHOOSE_ACTIVE: return "Current";
        case CHOOSE_PARTY: return "All";
        case CHOOSE_RANDOM: return "Random";
    }
    return {};
}

std::optional<EvtTargetCharacter> evtPlayerByName(std::string_view name) {
    if (name == "Current" || name == "current")
        return CHOOSE_ACTIVE;
    if (name == "All" || name == "all")
        return CHOOSE_PARTY;
    if (name == "Random" || name == "random")
        return CHOOSE_RANDOM;
    return std::nullopt;
}
