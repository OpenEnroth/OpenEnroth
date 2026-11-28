#include "EvtInstruction.h"

#include <algorithm>
#include <bit>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "Engine/Evt/EvtEnums.h"
#include "Engine/Evt/EvtInstructionFields.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Engine.h"

#include "Library/Binary/BinaryExceptions.h"
#include "Library/Binary/BinarySerialization.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Streams/InputStream.h"
#include "Utility/Exception.h"
#include "Utility/Lambda.h"

namespace {

// TODO(captainurist): tag this as MMExtension's names once serialization functions are told apart by tags.
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(FaceAttribute, CASE_SENSITIVE, {
    {FACE_IsPortal,         "IsPortal"},
    {FACE_IsSecret,         "IsSecret"},
    {FACE_FlowDown,         "ScrollDown"},
    {FACE_TexAlignDown,     "AlignTop"},
    {FACE_IsFluid,          "IsWater"},
    {FACE_FlowUp,           "ScrollUp"},
    {FACE_FlowLeft,         "ScrollLeft"},
    {FACE_XY_PLANE,         "ProjectToXY"},
    {FACE_XZ_PLANE,         "ProjectToXZ"},
    {FACE_YZ_PLANE,         "ProjectToYZ"},
    {FACE_FlowRight,        "ScrollRight"},
    {FACE_TexAlignLeft,     "AlignLeft"},
    {FACE_IsInvisible,      "Invisible"},
    {FACE_ANIMATED,         "AnimatedTFT"},
    {FACE_TexAlignRight,    "AlignRight"},
    {FACE_TexAlignBottom,   "AlignBottom"},
    {FACE_TexMoveByDoor,    "MoveByDoor"},
    {FACE_EVENT_IS_HINT,    "IsEventJustHint"},
    {FACE_INDOOR_CARPET,    "AlternativeSound"},
    {FACE_INDOOR_SKY,       "IsSky"},
    {FACE_FlipNormalU,      "FlipU"},
    {FACE_FlipNormalV,      "FlipV"},
    {FACE_CLICKABLE,        "TriggerByClick"},
    {FACE_PRESSURE_PLATE,   "TriggerByStep"},
    {FACE_TriggerByMonster, "TriggerByMonster"},
    {FACE_TriggerByObject,  "TriggerByObject"},
    {FACE_ETHEREAL,         "Untouchable"},
    {FACE_IsLava,           "IsLava"},
    {FACE_IsPicked,         "HasData"}
})

// TODO(captainurist): tag this as MMExtension's names once serialization functions are told apart by tags.
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(ActorAttribute, CASE_SENSITIVE, {
    {ACTOR_FULL_AI_STATE, "Active"},
    {ACTOR_NEARBY,        "ShowOnMap"},
    {ACTOR_UNKNOW11,      "Invisible"},
    {ACTOR_FLEEING,       "NoFlee"},
    {ACTOR_AGGRESSOR,     "Hostile"},
    {ACTOR_UNKNOW7,       "OnAlertMap"},
    {ACTOR_HAS_ITEM,      "TreasureGenerated"},
    {ACTOR_HOSTILE,       "ShowAsHostile"}
})

// TODO(captainurist): tag this as MMExtension's names once serialization functions are told apart by tags.
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(ChestFlag, CASE_SENSITIVE, {
    {CHEST_TRAPPED,      "Trapped"},
    {CHEST_ITEMS_PLACED, "ItemsPlaced"},
    {CHEST_OPENED,       "Identified"}
})

// TODO(captainurist): tag this as MMExtension's names once serialization functions are told apart by tags.
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(Mastery, CASE_SENSITIVE, {
    {MASTERY_NOVICE,      "Novice"},
    {MASTERY_EXPERT,      "Expert"},
    {MASTERY_MASTER,      "Master"},
    {MASTERY_GRANDMASTER, "GM"}
})

} // namespace

/**
 * @tparam T                            Type of a member of an instruction.
 * @return                              Name of the table of constants that MMExtension keeps the flags of type `T`
 *                                      in, as in `const.FacetBits.Untouchable`. Null if scripts have no names for `T`.
 */
template<class T>
static constexpr const char *constGroup() {
    if constexpr (std::is_same_v<T, FaceAttribute>) {
        return "FacetBits";
    } else if constexpr (std::is_same_v<T, ActorAttribute>) {
        return "MonsterBits";
    } else if constexpr (std::is_same_v<T, ChestFlag>) {
        return "ChestBits";
    } else {
        return nullptr;
    }
}

/**
 * @param text                          Text to quote.
 * @return                              The text as a Lua string.
 */
static std::string luaString(std::string_view text) {
    std::string result = "\"";
    for (char c : text) {
        if (c == '"' || c == '\\') {
            result += '\\';
            result += c;
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (static_cast<unsigned char>(c) < 0x20) {
            result += fmt::format("\\{:03}", static_cast<int>(c));
        } else {
            result += c;
        }
    }
    result += '"';
    return result;
}

/**
 * @param value                         Flags that scripts have names for.
 * @return                              The flags as a script writes them, e.g. `const.FacetBits.Untouchable`. Several
 *                                      flags are a sum, and a flag without a name is a hex number.
 */
template<class T>
static std::string luaFlags(T value) {
    auto bits = std::to_underlying(value);
    std::vector<std::string> flags;
    for (auto bit = std::bit_floor(bits); bit != 0; bit >>= 1) { // From the highest flag down, like MMExtension's decompiler.
        if (!(bits & bit))
            continue;

        std::string name;
        flags.push_back(trySerialize(static_cast<T>(bit), &name) ? fmt::format("const.{}.{}", constGroup<T>(), name) : fmt::format("0x{:X}", bit));
    }
    return flags.empty() ? "0" : fmt::format("{}", fmt::join(flags, " + "));
}

/**
 * @param value                         Member of an instruction.
 * @return                              The member as a script writes it.
 */
template<class T>
static std::string toLua(const T &value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return luaString(value);
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_same_v<T, EvtVariable>) {
        // A script passes a variable by name, as a string: `evt.Cmp("QBits", 240)`. So the name is in quotes.
        std::string name;
        return trySerialize(value, &name) ? luaString(name) : fmt::format("{}", std::to_underlying(value));
    } else if constexpr (std::is_same_v<T, EvtTargetCharacter>) {
        // A script passes a single player as a number and the others by name, as a string: `evt.ForPlayer(0)`,
        // `evt.ForPlayer("All")`. So only a name is in quotes.
        std::string name;
        if (!trySerialize(value, &name))
            return fmt::format("{}", std::to_underlying(value));
        return value <= CHOOSE_PLAYER4 ? name : luaString(name);
    } else if constexpr (std::is_same_v<T, Mastery>) {
        // A script takes a mastery and a flag from MMExtension's constants: `const.Master`, `const.FacetBits.Untouchable`.
        // These name Lua values, so they have no quotes.
        Mastery mastery = value == MASTERY_NONE ? MASTERY_NOVICE : value; // A skill check takes novice for any mastery.
        std::string name;
        return trySerialize(mastery, &name) ? fmt::format("const.{}", name) : fmt::format("{}", std::to_underlying(value));
    } else if constexpr (constGroup<T>() != nullptr) {
        return luaFlags(value);
    } else if constexpr (std::is_enum_v<T>) {
        return fmt::format("{}", std::to_underlying(value));
    } else {
        static_assert(std::is_arithmetic_v<T>);
        return fmt::format("{}", value);
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
    std::string name;
    if (!trySerialize(opcode, &name))
        name = fmt::format("Cmd{:02X}", std::to_underlying(opcode)); // What MMExtension calls a command that it has no name for.

    std::string fields;
    bool isKnown = visitEvtFields(*this, [&](auto, const char *fieldName, const auto *member, auto &&) {
        if constexpr (!isEvtPadding(member))
            fields += fmt::format("{}{} = {}", fields.empty() ? "" : ", ", fieldName, toLua(*member));
    });
    if (!isKnown)
        return fmt::format("{}: UNPROCESSED/{}", step, name);

    std::string result = fmt::format("{}: evt.{}{{{}}}", step, name, fields);
    if (std::string text = referencedText(*this, source); !text.empty())
        result += fmt::format("  -- {}", luaString(text));
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
    bool isKnown = visitEvtFields(ir, [&]<class Wire, class Member>(std::type_identity<Wire>, const char *name, Member *member, auto &&convert) {
        if constexpr (std::is_same_v<Wire, std::string>) {
            deserialize(stream, member, tags::nullTerminated);
        } else {
            Wire stored = {};
            deserialize(stream, &stored);
            if constexpr (!isEvtPadding(member)) {
                auto value = convert(stored);
                if constexpr (std::is_enum_v<Member> && std::is_integral_v<decltype(value)>)
                    if (!std::in_range<std::underlying_type_t<Member>>(value)) // The cast below would cut the value down to the enum's size.
                        throw Exception("Value '{}' is out of range for evt field '{}'", value, name);
                *member = static_cast<Member>(value); // An unsigned value past the range of a signed member is a negative one.
            }
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
