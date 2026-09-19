#include "EvtCommands.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Utility/Exception.h"
#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/MemoryInputStream.h"

static std::vector<EvtCommandInfo> makeCommands() {
    const EvtFieldInfo x = {"X", EVT_FIELD_I32}, y = {"Y", EVT_FIELD_I32}, z = {"Z", EVT_FIELD_I32};
    const EvtFieldInfo jump = {"jump", EVT_FIELD_JUMP}, unk = {"unk", EVT_FIELD_U8};
    const std::vector<EvtFieldInfo> variable = {{"VarNum", EVT_FIELD_VARIABLE}, {"Value", EVT_FIELD_I32}};
    const std::vector<EvtFieldInfo> variableJump = {{"VarNum", EVT_FIELD_VARIABLE}, {"Value", EVT_FIELD_I32}, jump};
    const std::vector<EvtFieldInfo> monstersKilled = {{"CheckType", EVT_FIELD_U8}, {"Id", EVT_FIELD_I32}, {"Count", EVT_FIELD_U8}, jump};
    const std::vector<EvtFieldInfo> timer = {{"IsYearly", EVT_FIELD_U8}, {"IsMonthly", EVT_FIELD_U8}, {"IsWeekly", EVT_FIELD_U8},
                                             {"Hour", EVT_FIELD_U8}, {"Minute", EVT_FIELD_U8}, {"Second", EVT_FIELD_U8},
                                             {"HalfMinutes", EVT_FIELD_U16}, {"unk", EVT_FIELD_U16}};

    return {
        {EVENT_Exit, "Exit", EVT_COMMAND_STRUCTURAL, {unk}},
        {EVENT_SpeakInHouse, "EnterHouse", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_I32}}},
        {EVENT_PlaySound, "PlaySound", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_I32}, x, y}},
        {EVENT_MouseOver, "MouseOver", EVT_COMMAND_STRUCTURAL, {{"Str", EVT_FIELD_U8}}},
        {EVENT_LocationName, "MazeInfo", EVT_COMMAND_UNSUPPORTED, {{"Str", EVT_FIELD_U8}}},
        {EVENT_MoveToMap, "MoveToMap", EVT_COMMAND_ACTION,
         {x, y, z, {"Direction", EVT_FIELD_I32}, {"LookAngle", EVT_FIELD_I32}, {"SpeedZ", EVT_FIELD_I32}, {"HouseId", EVT_FIELD_U8}, {"Icon", EVT_FIELD_U8},
          {"Name", EVT_FIELD_STRING}}},
        {EVENT_OpenChest, "OpenChest", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_U8}}},
        {EVENT_ShowFace, "FaceExpression", EVT_COMMAND_ACTION, {{"Player", EVT_FIELD_PLAYER}, {"Frame", EVT_FIELD_U8}}},
        {EVENT_ReceiveDamage, "DamagePlayer", EVT_COMMAND_ACTION, {{"Player", EVT_FIELD_PLAYER}, {"DamageType", EVT_FIELD_U8}, {"Damage", EVT_FIELD_I32}}},
        {EVENT_SetSnow, "SetSnow", EVT_COMMAND_ACTION, {{"EffectId", EVT_FIELD_U8}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_SetTexture, "SetTexture", EVT_COMMAND_ACTION, {{"Facet", EVT_FIELD_I32}, {"Name", EVT_FIELD_STRING}}},
        {EVENT_ShowMovie, "ShowMovie", EVT_COMMAND_ACTION, {{"DoubleSize", EVT_FIELD_U8}, {"ExitCurrentScreen", EVT_FIELD_BOOL}, {"Name", EVT_FIELD_STRING}}},
        {EVENT_SetSprite, "SetSprite", EVT_COMMAND_ACTION, {{"SpriteId", EVT_FIELD_I32}, {"Visible", EVT_FIELD_U8}, {"Name", EVT_FIELD_STRING}}},
        {EVENT_Compare, "Cmp", EVT_COMMAND_CONDITION, variableJump},
        {EVENT_ChangeDoorState, "SetDoorState", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_U8}, {"State", EVT_FIELD_U8}}},
        {EVENT_Add, "Add", EVT_COMMAND_ACTION, variable},
        {EVENT_Subtract, "Subtract", EVT_COMMAND_ACTION, variable},
        {EVENT_Set, "Set", EVT_COMMAND_ACTION, variable},
        {EVENT_SummonMonsters, "SummonMonsters", EVT_COMMAND_ACTION,
         {{"TypeIndexInMapStats", EVT_FIELD_U8}, {"Level", EVT_FIELD_U8}, {"Count", EVT_FIELD_U8}, x, y, z, {"NPCGroup", EVT_FIELD_I32}, {"unk", EVT_FIELD_I32}}},
        {EVENT_CastSpell, "CastSpell", EVT_COMMAND_ACTION,
         {{"Spell", EVT_FIELD_U8}, {"Mastery", EVT_FIELD_MASTERY}, {"Skill", EVT_FIELD_U8}, {"FromX", EVT_FIELD_I32}, {"FromY", EVT_FIELD_I32},
          {"FromZ", EVT_FIELD_I32}, {"ToX", EVT_FIELD_I32}, {"ToY", EVT_FIELD_I32}, {"ToZ", EVT_FIELD_I32}}},
        {EVENT_SpeakNPC, "SpeakNPC", EVT_COMMAND_ACTION, {{"NPC", EVT_FIELD_I32}}},
        {EVENT_SetFacesBit, "SetFacetBit", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_I32}, {"Bit", EVT_FIELD_I32, EVT_CONST_FACET_BITS}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_ToggleActorFlag, "SetMonsterBit", EVT_COMMAND_ACTION,
         {{"Monster", EVT_FIELD_I32}, {"Bit", EVT_FIELD_I32, EVT_CONST_MONSTER_BITS}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_RandomGoTo, "RandomGoTo", EVT_COMMAND_STRUCTURAL,
         {{"jump1", EVT_FIELD_JUMP}, {"jump2", EVT_FIELD_JUMP}, {"jump3", EVT_FIELD_JUMP}, {"jump4", EVT_FIELD_JUMP}, {"jump5", EVT_FIELD_JUMP},
          {"jump6", EVT_FIELD_JUMP}}},
        {EVENT_InputString, "Question", EVT_COMMAND_UNSUPPORTED,
         {{"Question", EVT_FIELD_I32}, {"Answer1", EVT_FIELD_I32}, {"Answer2", EVT_FIELD_I32}, jump}},
        {EVENT_StatusText, "StatusText", EVT_COMMAND_ACTION, {{"Str", EVT_FIELD_I32}}},
        {EVENT_ShowMessage, "SetMessage", EVT_COMMAND_ACTION, {{"Str", EVT_FIELD_I32}}},
        {EVENT_OnTimer, "OnTimer", EVT_COMMAND_STRUCTURAL, timer},
        {EVENT_ToggleIndoorLight, "SetLight", EVT_COMMAND_ACTION, {{"Id", EVT_FIELD_I32}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_PressAnyKey, "SimpleMessage", EVT_COMMAND_UNSUPPORTED, {unk}},
        {EVENT_SummonItem, "SummonObject", EVT_COMMAND_ACTION,
         {{"Type", EVT_FIELD_I32}, x, y, z, {"Speed", EVT_FIELD_I32}, {"Count", EVT_FIELD_U8}, {"RandomAngle", EVT_FIELD_BOOL}}},
        {EVENT_ForPartyMember, "ForPlayer", EVT_COMMAND_STRUCTURAL, {{"Player", EVT_FIELD_PLAYER}}},
        {EVENT_Jmp, "Jmp", EVT_COMMAND_STRUCTURAL, {jump}},
        {EVENT_OnMapReload, "OnMapReload", EVT_COMMAND_STRUCTURAL, {unk}},
        {EVENT_OnLongTimer, "OnLongTimer", EVT_COMMAND_STRUCTURAL, timer},
        {EVENT_SetNPCTopic, "SetNPCTopic", EVT_COMMAND_ACTION, {{"NPC", EVT_FIELD_I32}, {"Index", EVT_FIELD_U8}, {"Event", EVT_FIELD_I32}}},
        {EVENT_MoveNPC, "MoveNPC", EVT_COMMAND_ACTION, {{"NPC", EVT_FIELD_I32}, {"HouseId", EVT_FIELD_I32}}},
        {EVENT_GiveItem, "GiveItem", EVT_COMMAND_ACTION, {{"Strength", EVT_FIELD_U8}, {"Type", EVT_FIELD_U8}, {"Id", EVT_FIELD_I32}}},
        {EVENT_ChangeEvent, "ChangeEvent", EVT_COMMAND_ACTION, {{"NewEvent", EVT_FIELD_I32}}},
        {EVENT_CheckSkill, "CheckSkill", EVT_COMMAND_CONDITION, {{"Skill", EVT_FIELD_U8}, {"Mastery", EVT_FIELD_U8}, {"Level", EVT_FIELD_I32}, jump}},
        {EVENT_OnCanShowDialogItemCmp, "OnCanShowDialogItemCmp", EVT_COMMAND_STRUCTURAL, variableJump},
        {EVENT_EndCanShowDialogItem, "EndCanShowDialogItem", EVT_COMMAND_STRUCTURAL, {unk}},
        {EVENT_SetCanShowDialogItem, "SetCanShowDialogItem", EVT_COMMAND_STRUCTURAL, {{"Visible", EVT_FIELD_BOOL}}},
        {EVENT_SetNPCGroupNews, "SetNPCGroupNews", EVT_COMMAND_ACTION, {{"NPCGroup", EVT_FIELD_I32}, {"NPCNews", EVT_FIELD_I32}}},
        {EVENT_SetActorGroup, "SetMonsterGroup", EVT_COMMAND_UNSUPPORTED, {{"Monster", EVT_FIELD_I32}, {"NPCGroup", EVT_FIELD_I32}}},
        {EVENT_NPCSetItem, "SetNPCItem", EVT_COMMAND_ACTION, {{"NPC", EVT_FIELD_I32}, {"Item", EVT_FIELD_I32}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_SetNPCGreeting, "SetNPCGreeting", EVT_COMMAND_ACTION, {{"NPC", EVT_FIELD_I32}, {"Greeting", EVT_FIELD_I32}}},
        {EVENT_IsActorKilled, "CheckMonstersKilled", EVT_COMMAND_CONDITION, monstersKilled},
        {EVENT_CanShowTopic_IsActorKilled, "CanShowTopic_IsActorKilled", EVT_COMMAND_UNSUPPORTED, monstersKilled},
        {EVENT_OnMapLeave, "OnMapLeave", EVT_COMMAND_STRUCTURAL, {unk}},
        {EVENT_ChangeGroup, "ChangeGroupToGroup", EVT_COMMAND_UNSUPPORTED, {{"Old", EVT_FIELD_I32}, {"New", EVT_FIELD_I32}}},
        {EVENT_ChangeGroupAlly, "ChangeGroupAlly", EVT_COMMAND_UNSUPPORTED, {{"NPCGroup", EVT_FIELD_I32}, {"Ally", EVT_FIELD_I32}}},
        {EVENT_CheckSeason, "CheckSeason", EVT_COMMAND_CONDITION, {{"Season", EVT_FIELD_U8}, jump}},
        {EVENT_ToggleActorGroupFlag, "SetMonGroupBit", EVT_COMMAND_ACTION,
         {{"NPCGroup", EVT_FIELD_I32}, {"Bit", EVT_FIELD_I32, EVT_CONST_MONSTER_BITS}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_ToggleChestFlag, "SetChestBit", EVT_COMMAND_ACTION,
         {{"ChestId", EVT_FIELD_I32}, {"Bit", EVT_FIELD_I32, EVT_CONST_CHEST_BITS}, {"On", EVT_FIELD_BOOL}}},
        {EVENT_CharacterAnimation, "FaceAnimation", EVT_COMMAND_ACTION, {{"Player", EVT_FIELD_PLAYER}, {"Animation", EVT_FIELD_U8}}},
        {EVENT_SetActorItem, "SetMonsterItem", EVT_COMMAND_ACTION, {{"Monster", EVT_FIELD_I32}, {"Item", EVT_FIELD_I32}, {"Has", EVT_FIELD_BOOL}}},
    };
}

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

static int fieldSize(EvtFieldType type) {
    switch (type) {
        case EVT_FIELD_U16:
        case EVT_FIELD_VARIABLE:
            return 2;
        case EVT_FIELD_I32:
            return 4;
        default:
            return 1;
    }
}

/**
 * @param field                         Field of a command.
 * @return                              Smallest and largest value that a script can pass for the field.
 */
static std::pair<int64_t, int64_t> fieldRange(const EvtFieldInfo &field) {
    switch (field.type) {
        case EVT_FIELD_I32:
            return {INT32_MIN, field.constGroup == EVT_CONST_NONE ? INT32_MAX : UINT32_MAX}; // Bit masks go up to 2^31.
        case EVT_FIELD_MASTERY:
            return {std::to_underlying(MASTERY_NOVICE), std::to_underlying(MASTERY_GRANDMASTER)};
        case EVT_FIELD_PLAYER:
            return {std::to_underlying(CHOOSE_PLAYER1), std::to_underlying(CHOOSE_RANDOM)};
        default:
            return {0, (1ll << (8 * fieldSize(field.type))) - 1};
    }
}

/**
 * @param command                       Command to decode the fields of.
 * @param payload                       Bytes of a record after the opcode.
 * @return                              One value per field, or `std::nullopt` if the payload isn't laid out like the
 *                                      command.
 */
static std::optional<std::vector<EvtFieldValue>> decodeFields(const EvtCommandInfo &command, std::string_view payload) {
    std::vector<EvtFieldValue> result;

    size_t pos = 0;
    for (const EvtFieldInfo &field : command.fields) {
        if (field.type == EVT_FIELD_STRING) {
            size_t end = payload.find('\0', pos);
            if (end == std::string_view::npos)
                return std::nullopt;
            result.emplace_back(std::string(payload.substr(pos, end - pos)));
            pos = end + 1;
            continue;
        }

        int size = fieldSize(field.type);
        if (pos + size > payload.size())
            return std::nullopt;
        uint32_t raw = 0;
        for (int i = 0; i < size; i++)
            raw |= static_cast<uint32_t>(static_cast<uint8_t>(payload[pos + i])) << (8 * i);
        pos += size;

        if (field.type == EVT_FIELD_I32) {
            result.emplace_back(static_cast<int64_t>(static_cast<int32_t>(raw)));
        } else if (field.type == EVT_FIELD_MASTERY) {
            result.emplace_back(static_cast<int64_t>(raw) + 1);
        } else {
            result.emplace_back(static_cast<int64_t>(raw));
        }
    }

    if (pos != payload.size())
        return std::nullopt;
    return result;
}

std::vector<EvtRecord> decodeEvtRecords(const Blob &data) {
    std::vector<EvtRecord> result;

    std::string_view bytes = data.str();
    size_t pos = 0;
    while (pos < bytes.size()) {
        size_t size = static_cast<uint8_t>(bytes[pos]) + 1;
        if (size < 5 || pos + size > bytes.size())
            throw Exception("Encountered corrupted evt binary data");

        EvtRecord &record = result.emplace_back();
        record.eventId = static_cast<uint8_t>(bytes[pos + 1]) | (static_cast<uint8_t>(bytes[pos + 2]) << 8);
        record.step = static_cast<uint8_t>(bytes[pos + 3]);
        record.opcode = static_cast<EvtOpcode>(bytes[pos + 4]);

        std::string_view payload = bytes.substr(pos + 5, size - 5);
        if (const EvtCommandInfo *command = evtCommand(record.opcode))
            record.values = decodeFields(*command, payload);
        if (!record.values)
            record.payload = std::string(payload);

        pos += size;
    }

    return result;
}

std::string encodeEvtRecord(const EvtRecord &record) {
    if (record.eventId < 0 || record.eventId > 0xFFFF || record.step < 0 || record.step > 0xFF)
        throw Exception("Event {} step {} doesn't fit an evt record", record.eventId, record.step);

    std::string result;
    result.push_back(0); // Size, filled in below.
    result.push_back(static_cast<char>(record.eventId & 0xFF));
    result.push_back(static_cast<char>((record.eventId >> 8) & 0xFF));
    result.push_back(static_cast<char>(record.step));
    result.push_back(static_cast<char>(record.opcode));

    if (!record.values) {
        result += record.payload;
    } else {
        const EvtCommandInfo *command = evtCommand(record.opcode);
        if (!command)
            throw Exception("Unknown evt opcode {}", std::to_underlying(record.opcode));
        if (record.values->size() != command->fields.size())
            throw Exception("evt.{} takes {} fields, got {}", command->name, command->fields.size(), record.values->size());

        for (size_t i = 0; i < command->fields.size(); i++) {
            const EvtFieldInfo &field = command->fields[i];

            if (field.type == EVT_FIELD_STRING) {
                const std::string *value = std::get_if<std::string>(&(*record.values)[i]);
                if (!value)
                    throw Exception("evt.{}: field {} takes a string", command->name, field.name);
                if (value->contains('\0'))
                    throw Exception("evt.{}: field {} can't hold a string with a zero byte in it", command->name, field.name);
                result += *value;
                result.push_back('\0');
                continue;
            }

            const int64_t *value = std::get_if<int64_t>(&(*record.values)[i]);
            if (!value)
                throw Exception("evt.{}: field {} takes a number", command->name, field.name);
            auto [min, max] = fieldRange(field);
            if (*value < min || *value > max)
                throw Exception("evt.{}: value {} doesn't fit field {}, which takes {} to {}", command->name, *value, field.name, min, max);
            int64_t raw = field.type == EVT_FIELD_MASTERY ? *value - 1 : *value;
            for (int j = 0; j < fieldSize(field.type); j++)
                result.push_back(static_cast<char>((raw >> (8 * j)) & 0xFF));
        }
    }

    if (result.size() > 256)
        throw Exception("An evt record of {} bytes is over the limit of 256", result.size());
    result[0] = static_cast<char>(result.size() - 1);
    return result;
}

EvtInstruction evtInstruction(const EvtRecord &record) {
    const EvtCommandInfo *command = evtCommand(record.opcode);
    bool isSupported = record.values && command && command->kind != EVT_COMMAND_UNSUPPORTED;
    if (!isSupported) // `parse` asserts on the layouts of some unsupported commands.
        throw Exception("OpenEnroth doesn't support evt opcode {}", std::to_underlying(record.opcode));

    std::string bytes = encodeEvtRecord(record);
    MemoryInputStream stream(bytes.data() + 3, bytes.size() - 3); // Skips the size byte and the event id, as `EvtProgram::load` does.
    return EvtInstruction::parse(stream, bytes.size());
}
