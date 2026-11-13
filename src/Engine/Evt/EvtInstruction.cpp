#include "EvtInstruction.h"

#include <algorithm>
#include <array>
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

namespace {

/**
 * Reads the fields of an instruction from an evt record.
 */
class EvtFieldReader {
 public:
    explicit EvtFieldReader(InputStream &stream) : _stream(stream) {}

    template<class Wire>
    void operator()(EvtWire<Wire>, EvtPadding) {
        fromStream<Wire>(_stream);
    }

    template<class Wire, class T>
    void operator()(EvtWire<Wire>, std::string_view name, T &field) {
        field = convert<T>(fromStream<Wire>(_stream), name);
    }

    template<class Wire, class T, size_t N>
    void operator()(EvtWire<Wire>, std::string_view name, std::array<T, N> &field) {
        for (T &element : field)
            element = convert<T>(fromStream<Wire>(_stream), name);
    }

    template<class Wire, class T, class Convert>
    void operator()(EvtWire<Wire>, std::string_view, T &field, Convert &&convert) {
        field = convert(fromStream<Wire>(_stream));
    }

    void operator()(EvtWireString, std::string_view, std::string &field) {
        field = fromStream<std::string>(_stream, tags::nullTerminated);
    }

 private:
    template<class T, class Wire>
    static T convert(Wire value, std::string_view name) {
        if constexpr (std::is_enum_v<T>) {
            if (!std::in_range<std::underlying_type_t<T>>(value))
                throw Exception("Evt field {} doesn't take {}", name, value);
            return static_cast<T>(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return value != 0;
        } else {
            return static_cast<T>(value); // An unsigned value past the range of a signed member is a negative one.
        }
    }

 private:
    InputStream &_stream;
};

/**
 * Writes the fields of an instruction as `Name = value` pairs, for logging.
 */
class EvtFieldWriter {
 public:
    template<class Wire>
    void operator()(EvtWire<Wire>, EvtPadding) {}

    template<class Wire, class T, class... Convert>
    void operator()(EvtWire<Wire>, std::string_view name, const T &field, Convert &&...) {
        add(name, format(field));
    }

    void operator()(EvtWireString, std::string_view name, const std::string &field) {
        add(name, fmt::format("\"{}\"", field));
    }

    [[nodiscard]] const std::string &text() const {
        return _text;
    }

 private:
    void add(std::string_view name, std::string_view value) {
        _text += fmt::format("{}{} = {}", _text.empty() ? "" : ", ", name, value);
    }

    static std::string format(bool value) {
        return value ? "true" : "false";
    }

    static std::string format(EvtVariable value) {
        std::string name;
        return trySerialize(value, &name) ? fmt::format("\"{}\"", name) : fmt::format("{}", std::to_underlying(value));
    }

    template<class T> requires std::is_same_v<T, FaceAttribute> || std::is_same_v<T, ActorAttribute> || std::is_same_v<T, ChestFlag>
    static std::string format(T value) {
        return fmt::format("0x{:X}", std::to_underlying(value));
    }

    template<class T, size_t N>
    static std::string format(const std::array<T, N> &value) {
        return fmt::format("{{{}}}", fmt::join(value, ", "));
    }

    template<class T>
    static std::string format(T value) {
        if constexpr (std::is_enum_v<T>) {
            return fmt::format("{}", std::to_underlying(value));
        } else {
            return fmt::format("{}", value);
        }
    }

 private:
    std::string _text;
};

} // namespace

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

    EvtFieldWriter writer;
    if (!visitEvtFields(*this, writer))
        return fmt::format("{}: UNPROCESSED/{}", step, name);

    std::string result = fmt::format("{}: {}{{{}}}", step, name, writer.text());
    if (std::string text = referencedText(*this, source); !text.empty())
        result += fmt::format("  -- \"{}\"", text);
    return result;
}

EvtInstruction EvtInstruction::parse(InputStream &stream) {
    EvtInstruction ir = {};  // A zeroed member may be unset or may be a real 0, the two are not distinguishable here.

    ir.step = fromStream<uint8_t>(stream);
    ir.opcode = EvtOpcode(fromStream<uint8_t>(stream));

    // TODO(captainurist): verify enum ranges here.
    if (!visitEvtFields(ir, EvtFieldReader(stream)))
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
