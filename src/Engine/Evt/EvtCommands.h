#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "Engine/Evt/EvtEnums.h"
#include "Engine/Evt/EvtInstruction.h"

/**
 * What scripts pass for a field of a command.
 */
enum class EvtFieldType {
    EVT_FIELD_INT,
    EVT_FIELD_BOOL, // Scripts pass true or false.
    EVT_FIELD_STRING,
    EVT_FIELD_PLAYER, // `EvtTargetCharacter`. Scripts pass 0-3, "Current", "All" or "Random".
    EVT_FIELD_VARIABLE, // `EvtVariable`. Scripts pass a name like "QBits".
    EVT_FIELD_MASTERY, // Scripts pass 1-4, as in `const.Novice` to `const.GM`.
    EVT_FIELD_NPC, // Index into `NPCStats::pNPCData`.
    EVT_FIELD_NPC_GROUP, // Index into `NPCStats::pGroups`.
};
using enum EvtFieldType;

/**
 * Groups of named constants that scripts write as `const.<Group>.<Name>`.
 */
enum class EvtConstGroup {
    EVT_CONST_NONE,
    EVT_CONST_FACET_BITS,
    EVT_CONST_MONSTER_BITS,
    EVT_CONST_CHEST_BITS,
};
using enum EvtConstGroup;

using EvtFieldValue = std::variant<int64_t, std::string>;

/**
 * One field of a command, and where it lives in an `EvtInstruction`.
 */
struct EvtFieldInfo {
    std::string_view name; // MMExtension's field name, e.g. "SpriteId".
    EvtFieldType type = EVT_FIELD_INT;
    EvtConstGroup constGroup = EVT_CONST_NONE;
    std::function<EvtFieldValue(const EvtInstruction &)> get; // Empty for a field that OpenEnroth doesn't keep.
    std::function<void(EvtInstruction *, const EvtFieldValue &)> set; // Throws if the value doesn't fit the field.
};

/**
 * A command that scripts can call, e.g. `evt.SetSprite`.
 */
struct EvtCommandInfo {
    EvtOpcode opcode = EVENT_Invalid;
    std::string_view name; // MMExtension's command name, e.g. "SetFacetBit".
    bool isCondition = false; // Jumps when its condition holds. From a script it returns the condition.
    std::vector<EvtFieldInfo> fields; // In the order of positional script arguments.
};

/**
 * @return                              The commands that scripts can call, in opcode order.
 */
std::span<const EvtCommandInfo> evtCommands();

/**
 * @param opcode                        Opcode to look up.
 * @return                              Command for the opcode, or `nullptr` if scripts can't call it.
 */
const EvtCommandInfo *evtCommand(EvtOpcode opcode);

/**
 * @param name                          MMExtension command name, e.g. "Cmp". Aliases like "Sub" work too.
 * @return                              Command with that name, or `nullptr` if there's none.
 */
const EvtCommandInfo *evtCommand(std::string_view name);

/**
 * @param variable                      Variable to name.
 * @return                              MMExtension's name for it, e.g. "QBits" or "MapVar3". Empty if it has none.
 */
std::string evtVariableName(EvtVariable variable);

/**
 * @param name                          MMExtension variable name. Aliases like "Items" work too.
 * @return                              The variable, or `std::nullopt` if the name is unknown.
 */
std::optional<EvtVariable> evtVariableByName(std::string_view name);

/**
 * @param group                         Constant group.
 * @return                              Name of the group as scripts spell it, e.g. "FacetBits".
 */
std::string_view evtConstGroupName(EvtConstGroup group);

/**
 * @param group                         Constant group.
 * @return                              Names and values of the group's constants.
 */
std::span<const std::pair<std::string_view, int64_t>> evtConstants(EvtConstGroup group);

/**
 * @param target                        Character target.
 * @return                              What scripts pass for it: "0" to "3", "Current", "All" or "Random".
 */
std::string_view evtPlayerName(EvtTargetCharacter target);

/**
 * @param name                          "Current", "All" or "Random", in any of MMExtension's spellings.
 * @return                              The character target, or `std::nullopt` if the name is unknown.
 */
std::optional<EvtTargetCharacter> evtPlayerByName(std::string_view name);
