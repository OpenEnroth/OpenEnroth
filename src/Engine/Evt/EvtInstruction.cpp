#include "EvtInstruction.h"

#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Engine/Evt/EvtEnums.h"
#include "Engine/Evt/EvtInstructionFields.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Engine.h"

#include "Library/Binary/BinaryExceptions.h"
#include "Library/Binary/BinarySerialization.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Streams/InputStream.h"
#include "Utility/Exception.h"
#include "Utility/Lambda.h"

/**
 * @param value                         What an evt record stores for a field, after the field's conversion.
 * @param name                          Name of the field.
 * @return                              The value as the member of type `T` that holds the field.
 * @throws Exception                    If the value doesn't fit the member.
 */
template<class T, class Value>
static T fromWire(Value value, const char *name) {
    if constexpr (std::is_same_v<T, Value>) {
        return value;
    } else if constexpr (requires { std::tuple_size<T>::value; }) { // An array, stored as an array of the same size.
        T result;
        for (size_t i = 0; i < result.size(); i++)
            result[i] = fromWire<typename T::value_type>(value[i], name);
        return result;
    } else if constexpr (std::is_enum_v<T>) {
        if (!std::in_range<std::underlying_type_t<T>>(value))
            throw Exception("Value '{}' is out of range for evt field '{}'", value, name);
        return static_cast<T>(value);
    } else if constexpr (std::is_same_v<T, bool>) {
        return value != 0;
    } else {
        return static_cast<T>(value); // An unsigned value past the range of a signed member is a negative one.
    }
}

/**
 * @param value                         Member of an instruction.
 * @return                              The member as the log shows it.
 */
template<class T>
static std::string toLogString(const T &value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return fmt::format("\"{}\"", value);
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_same_v<T, EvtVariable>) {
        std::string name;
        return trySerialize(value, &name) ? fmt::format("\"{}\"", name) : fmt::format("{}", std::to_underlying(value));
    } else if constexpr (std::is_same_v<T, FaceAttribute> || std::is_same_v<T, ActorAttribute> || std::is_same_v<T, ChestFlag>) {
        return fmt::format("0x{:X}", std::to_underlying(value));
    } else if constexpr (std::is_enum_v<T>) {
        return fmt::format("{}", std::to_underlying(value));
    } else if constexpr (std::is_arithmetic_v<T>) {
        return fmt::format("{}", value);
    } else {
        return fmt::format("{{{}}}", fmt::join(value, ", ")); // An array of numbers.
    }
}

/**
 * @param ir                            Instruction to look at.
 * @param source                        Evt file the instruction comes from, which picks the table its texts come from.
 * @return                              The text or the house that the instruction refers to by number, if it does and
 *                                      there is one.
 */
static std::string referencedText(const EvtInstruction &ir, EvtSource source) {
    switch (ir.opcode) {
        case EVENT_SpeakInHouse:
            return houseTable.indices().contains(ir.data.house_id) ? houseTable[ir.data.house_id].name : std::string();
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            if (source == EVT_SOURCE_GLOBAL) // Then it's a topic text, from 1.
                return ir.data.text_id >= 1 && ir.data.text_id <= pNPCTopics.size() ? pNPCTopics[ir.data.text_id - 1].pText : std::string();
            [[fallthrough]];
        case EVENT_MouseOver:
        case EVENT_LocationName:
        case EVENT_InputString:
            return ir.data.text_id >= 0 && ir.data.text_id < engine->_levelStrings.size() ? engine->_levelStrings[ir.data.text_id] : std::string();
        default:
            return {};
    }
}

std::string EvtInstruction::toString(EvtSource source) const {
    std::string name = ::toString(opcode);
    if (name.starts_with("EVENT_"))
        name = name.substr(6);

    std::string fields;
    bool isKnown = visitEvtFields(*this, [&](auto, const char *fieldName, const auto *member, auto &&) {
        if constexpr (!std::is_void_v<std::remove_pointer_t<decltype(member)>>)
            fields += fmt::format("{}{} = {}", fields.empty() ? "" : ", ", fieldName, toLogString(*member));
    });
    if (!isKnown)
        return fmt::format("{}: UNPROCESSED/{}", step, name);

    std::string result = fmt::format("{}: {}{{{}}}", step, name, fields);
    if (std::string text = referencedText(*this, source); !text.empty())
        result += fmt::format("  -- \"{}\"", text);
    return result;
}

EvtInstruction EvtInstruction::parse(InputStream &stream) {
    EvtInstruction ir = {};  // A zeroed member may be unset or may be a real 0, the two are not distinguishable here.

    uint8_t step = 0;
    uint8_t opcode = 0;
    deserialize(stream, &step);
    deserialize(stream, &opcode);
    ir.step = step;
    ir.opcode = static_cast<EvtOpcode>(opcode);

    // TODO(captainurist): verify enum ranges here.
    bool isKnown = visitEvtFields(ir, [&]<class Wire>(std::type_identity<Wire>, const char *name, auto *member, auto &&convert) {
        using Member = std::remove_pointer_t<decltype(member)>;
        if constexpr (std::is_same_v<Wire, std::string>) {
            deserialize(stream, member, tags::nullTerminated);
        } else {
            Wire stored = Wire();
            deserialize(stream, &stored);
            if constexpr (!std::is_void_v<Member>)
                *member = fromWire<Member>(convert(stored), name);
        }
    });
    if (!isKnown)
        throw Exception("Unknown evt type: {}", std::to_underlying(ir.opcode));

    if (ir.opcode == EVENT_MouseOver || ir.opcode == EVENT_LocationName) {
        // TODO(captainurist): a hint is not an instruction, store it in EvtProgram outside of the instruction list and drop the -1.
        ir.step = -1; // Never executed. The data gives a hint the step of the command that follows it, and -1 keeps it out of lookups by step.
    } else if (ir.opcode == EVENT_RandomGoTo) {
        auto &jumps = ir.data.random_goto_descr;
        jumps.random_goto_len = std::ranges::count_if(jumps.random_goto, _1 > 0);
        if (jumps.random_goto_len == 0)
            throw Exception("RandomGoTo event has 0 targets");
    }

    if (stream.position() != stream.size())
        throwBinarySerializationLeftoverDataError(stream.position(), stream.size() - stream.position(), typeid(EvtInstruction).name());

    return ir;
}
