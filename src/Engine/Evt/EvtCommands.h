#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "Engine/Evt/EvtEnums.h"
#include "Engine/Evt/EvtInstruction.h"

class Blob;

enum class EvtFieldType {
    EVT_FIELD_U8,
    EVT_FIELD_U16,
    EVT_FIELD_I32,
    EVT_FIELD_BOOL, // One byte, scripts pass true or false.
    EVT_FIELD_STRING, // Null-terminated.
    EVT_FIELD_JUMP, // One byte, the step to continue from when the command's condition holds.
    EVT_FIELD_PLAYER, // One byte, `EvtTargetCharacter`. Scripts pass 0-3, "Current", "All" or "Random".
    EVT_FIELD_VARIABLE, // Two bytes, `EvtVariable`. Scripts pass a name like "QBits".
    EVT_FIELD_MASTERY, // One byte holding mastery minus one. Scripts pass 1-4, as in `const.Novice` to `const.GM`.
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

/**
 * How a command takes part in a script.
 */
enum class EvtCommandKind {
    EVT_COMMAND_ACTION, // Does something and falls through to the next step.
    EVT_COMMAND_CONDITION, // Jumps when its condition holds. From a script it returns the condition as a boolean.
    EVT_COMMAND_STRUCTURAL, // Control flow, triggers and hints. Scripts express these with Lua itself.
    EVT_COMMAND_UNSUPPORTED, // Known to MMExtension, not parsed or not executed by OpenEnroth.
};
using enum EvtCommandKind;

struct EvtFieldInfo {
    std::string_view name; // MMExtension's field name, e.g. "SpriteId".
    EvtFieldType type = EVT_FIELD_U8;
    EvtConstGroup constGroup = EVT_CONST_NONE;
};

struct EvtCommandInfo {
    EvtOpcode opcode = EVENT_Invalid;
    std::string_view name; // MMExtension's command name, e.g. "SetFacetBit".
    EvtCommandKind kind = EVT_COMMAND_ACTION;
    std::vector<EvtFieldInfo> fields; // In record order, which is also the order of positional script arguments.
};

using EvtFieldValue = std::variant<int64_t, std::string>;

/**
 * One record of an evt file with its fields decoded by the command table.
 */
struct EvtRecord {
    int eventId = 0;
    int step = 0;
    EvtOpcode opcode = EVENT_Invalid;
    std::vector<EvtFieldValue> values; // One per field of the command. Empty if the record didn't match the table.
    std::string payload; // Bytes after the opcode, kept for records that didn't match.
};

/**
 * @return                              All commands of the MM7 evt format, in opcode order.
 */
std::span<const EvtCommandInfo> evtCommands();

/**
 * @param opcode                        Opcode to look up.
 * @return                              Command for the opcode, or `nullptr` if the table has none.
 */
const EvtCommandInfo *evtCommand(EvtOpcode opcode);

/**
 * @param name                          MMExtension command name, e.g. "Cmp". Aliases like "Sub" work too.
 * @return                              Command with that name, or `nullptr` if the table has none.
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

/**
 * @param data                          Contents of an evt file.
 * @return                              Its records in file order.
 * @throws Exception                    If the record framing is broken.
 */
std::vector<EvtRecord> decodeEvtRecords(const Blob &data);

/**
 * @param record                        Record to encode. Needs one value per field of its command.
 * @return                              The record as it sits in an evt file, size byte included.
 * @throws Exception                    If the opcode has no command, or the values don't fit the fields.
 */
std::string encodeEvtRecord(const EvtRecord &record);

/**
 * @param record                        Record to convert.
 * @return                              The instruction `EvtInstruction::parse` makes of the encoded record.
 * @throws Exception                    If the record can't be encoded or parsed.
 */
EvtInstruction evtInstruction(const EvtRecord &record);
