#include "EvtDecompiler.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Tables/QuestTable.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

static const int END = -1; // Index that stands for the end of the event.
static const char LABEL_MARK = '\x01'; // Starts a line that turns into a label if something jumps to it.
static const std::string_view INDENT = "    ";
static const std::string_view RETURN = "do return end"; // Lua takes a bare return only as the last statement of a block.
static const std::array<std::string_view, 4> MASTERY_NAMES = {"const.Novice", "const.Expert", "const.Master", "const.GM"};

enum class EvtFlowKind {
    EVT_FLOW_NEXT, // Runs and falls through.
    EVT_FLOW_STOP, // Ends the event.
    EVT_FLOW_JUMP,
    EVT_FLOW_BRANCH, // Jumps if its condition holds.
    EVT_FLOW_RANDOM, // Jumps to one of its targets.
};
using enum EvtFlowKind;

struct EvtFlow {
    EvtFlowKind kind = EVT_FLOW_NEXT;
    std::string text; // Condition of `EVT_FLOW_BRANCH`, or the statement of another kind, which can be empty.
    std::vector<int> targets; // Instruction indices, or `END`.
};

/**
 * An NPC topic's event runs in two ways. The engine runs it to decide whether to show the topic, and then only the
 * `CanShowTopic` commands count. Everything else runs when the topic is picked.
 */
enum class EvtMode {
    EVT_MODE_EVENT,
    EVT_MODE_TOPIC,
};
using enum EvtMode;

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
    return result + "\"";
}

static std::string itemName(int64_t id) {
    if (!pItemTable || !pItemTable->items.indices().contains(static_cast<ItemId>(id)))
        return {};
    return pItemTable->items[static_cast<ItemId>(id)].name;
}

static std::string npcName(int64_t id) {
    if (!pNPCStats || id < 0 || id >= pNPCStats->pNPCData.size())
        return {};
    return pNPCStats->pNPCData[id].name;
}

static std::string houseName(int64_t id) {
    if (!houseTable.indices().contains(static_cast<HouseId>(id)))
        return {};
    return houseTable[static_cast<HouseId>(id)].name;
}

static std::string questName(int64_t bit) {
    if (!pQuestTable.indices().contains(static_cast<QuestBit>(bit)))
        return {};
    return pQuestTable[static_cast<QuestBit>(bit)];
}

static std::string formatPlayer(int64_t player) {
    if (player >= std::to_underlying(CHOOSE_ACTIVE) && player <= std::to_underlying(CHOOSE_RANDOM))
        return luaString(evtPlayerName(static_cast<EvtTargetCharacter>(player)));
    return fmt::format("{}", player);
}

static std::string formatVariable(EvtVariable variable) {
    std::string name = evtVariableName(variable);
    return name.empty() ? fmt::format("{}", std::to_underlying(variable)) : luaString(name);
}

static std::string withComment(std::string statement, std::string_view comment) {
    if (!comment.empty())
        statement += fmt::format("  -- {}", luaString(comment));
    return statement;
}

/**
 * Decompiles one event. Instructions are addressed by their index in the event, not by their step.
 */
class EvtEventDecompiler {
 public:
    EvtEventDecompiler(int eventId, const std::vector<EvtInstruction> &instructions, const std::vector<std::string> &strings, bool isGlobal);

    std::string decompile();

 private:
    std::string string(int64_t index) const;

    std::string formatValue(const EvtFieldInfo &field, const EvtFieldValue &value) const;
    std::string formatCall(int index, std::string_view prefix) const;
    std::string comment(int index) const;
    std::pair<std::string_view, std::string> timerCall(int index) const;
    std::string hint() const;

    int indexOfStep(int64_t step) const;
    int nextIndex(int index) const;
    EvtFlow flow(int index, EvtMode mode) const;
    std::set<int> reachable(int entry, EvtMode mode) const;

    void line(int depth, std::string_view text);
    std::string jump(int target);
    bool canNest(int lo, int hi, int owner) const;
    void emitStatement(int index, const EvtFlow &flow, int depth);
    void emitFlowTail(int index, const EvtFlow &flow, int following, int depth);
    void emitBlock(int lo, int hi, int depth);
    void emitFlat(int entry);
    std::string function(int entry, EvtMode mode, std::string_view header, std::string_view footer);

 private:
    int _eventId = 0;
    std::vector<EvtInstruction> _instructions;
    std::vector<EvtInstruction> _code; // Without the hints.
    const std::vector<std::string> &_strings;
    bool _isGlobal = false;

    EvtMode _mode = EVT_MODE_EVENT;
    std::set<int> _reachable;
    std::set<int> _labels;
    std::vector<std::string> _lines;
    bool _usesRandom = false;
};

EvtEventDecompiler::EvtEventDecompiler(int eventId, const std::vector<EvtInstruction> &instructions, const std::vector<std::string> &strings,
                                       bool isGlobal) : _eventId(eventId), _instructions(instructions), _strings(strings), _isGlobal(isGlobal) {
    for (const EvtInstruction &ir : instructions)
        if (ir.opcode != EVENT_MouseOver && ir.opcode != EVENT_LocationName) // These share their step with the next instruction.
            _code.push_back(ir);
}

std::string EvtEventDecompiler::decompile() {
    std::string result = hint();
    if (_code.empty())
        return result;

    // The interpreter starts an event from step 0, wherever that is in the file.
    int entry = indexOfStep(0);

    bool hasTopicCommands = std::ranges::any_of(_code, [](const EvtInstruction &ir) {
        return ir.opcode == EVENT_OnCanShowDialogItemCmp || ir.opcode == EVENT_SetCanShowDialogItem;
    });
    if (_isGlobal && hasTopicCommands)
        result += function(entry, EVT_MODE_TOPIC, fmt::format("evt.CanShowTopic[{}] = function()", _eventId), "end");

    // An event that does nothing still has to exist, or a click on it reports that there's nothing here. The engine
    // takes an event that opens with a hint and an exit for a hint alone, and its faces can't be clicked at all.
    std::string main = function(entry, EVT_MODE_EVENT, fmt::format("evt.{}[{}] = function()", _isGlobal ? "global" : "map", _eventId), "end");
    bool isEmpty = std::ranges::count(main, '\n') <= 2;
    if (!isEmpty || !EvtProgram::isHintOnly(_instructions))
        result += main;

    for (int index = 0; index < _code.size(); index++) {
        if (nextIndex(index) == END)
            continue;

        if (_code[index].opcode == EVENT_OnMapReload) {
            result += function(nextIndex(index), EVT_MODE_EVENT, "function events.LoadMap()", "end");
        } else if (_code[index].opcode == EVENT_OnMapLeave) {
            result += function(nextIndex(index), EVT_MODE_EVENT, "function events.LeaveMap()", "end");
        } else if (_code[index].opcode == EVENT_OnTimer || _code[index].opcode == EVENT_OnLongTimer) {
            auto [name, args] = timerCall(index);
            result += function(nextIndex(index), EVT_MODE_EVENT, fmt::format("{}(function()", name), fmt::format("end, {})", args));
        }
    }

    return result;
}

std::string EvtEventDecompiler::string(int64_t index) const {
    return index >= 0 && index < _strings.size() ? _strings[index] : std::string();
}

std::string EvtEventDecompiler::formatValue(const EvtFieldInfo &field, const EvtFieldValue &value) const {
    if (field.type == EVT_FIELD_STRING)
        return luaString(std::get<std::string>(value));

    int64_t number = std::get<int64_t>(value);
    switch (field.type) {
        case EVT_FIELD_BOOL:
            if (number == 0 || number == 1)
                return number ? "true" : "false";
            break;
        case EVT_FIELD_PLAYER:
            return formatPlayer(number);
        case EVT_FIELD_VARIABLE:
            return formatVariable(static_cast<EvtVariable>(number));
        case EVT_FIELD_MASTERY:
            if (number >= 1 && number <= MASTERY_NAMES.size())
                return std::string(MASTERY_NAMES[number - 1]);
            break;
        default:
            break;
    }

    if (field.constGroup != EVT_CONST_NONE) {
        int64_t bits = number & 0xFFFFFFFF;
        for (const auto &[name, constant] : evtConstants(field.constGroup))
            if (constant == bits)
                return fmt::format("const.{}.{}", evtConstGroupName(field.constGroup), name);
        return fmt::format("0x{:X}", bits);
    }
    return fmt::format("{}", number);
}

std::string EvtEventDecompiler::formatCall(int index, std::string_view prefix) const {
    const EvtInstruction &ir = _code[index];
    const EvtCommandInfo *info = evtCommand(ir.opcode);
    assert(info);

    std::vector<std::pair<std::string_view, std::string>> args;
    for (const EvtFieldInfo &field : info->fields)
        if (field.get)
            args.emplace_back(field.name, formatValue(field, field.get(ir)));

    bool isVariableCommand = std::ranges::contains(std::array{EVENT_Compare, EVENT_Add, EVENT_Subtract, EVENT_Set}, ir.opcode);
    bool isPositional = args.size() <= 1 || isVariableCommand;
    std::string result = fmt::format("{}{}{}", prefix, info->name, isPositional ? "(" : "{");
    for (size_t i = 0; i < args.size(); i++)
        result += fmt::format("{}{}{}", i ? ", " : "", isPositional ? "" : fmt::format("{} = ", args[i].first), args[i].second);
    return result + (isPositional ? ")" : "}");
}

std::string EvtEventDecompiler::comment(int index) const {
    const EvtInstruction &ir = _code[index];
    switch (ir.opcode) {
        case EVENT_Compare:
        case EVENT_Add:
        case EVENT_Subtract:
        case EVENT_Set:
        case EVENT_OnCanShowDialogItemCmp:
            if (ir.data.variable_descr.type == VAR_QBits_QuestsDone)
                return questName(ir.data.variable_descr.value);
            if (ir.data.variable_descr.type == VAR_PlayerItemInHands)
                return itemName(ir.data.variable_descr.value);
            return {};
        case EVENT_SpeakInHouse:
            return houseName(std::to_underlying(ir.data.house_id));
        case EVENT_SpeakNPC:
        case EVENT_SetNPCGreeting:
            return npcName(ir.data.npc_descr.npc_id);
        case EVENT_SetNPCTopic:
            return npcName(ir.data.npc_topic_descr.npc_id);
        case EVENT_MoveNPC:
            return npcName(ir.data.npc_move_descr.npc_id);
        case EVENT_NPCSetItem:
            return npcName(ir.data.npc_item_descr.id);
        case EVENT_GiveItem:
            return itemName(std::to_underlying(ir.data.give_item_descr.item_id));
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            return _isGlobal ? std::string() : string(ir.data.text_id);
        default:
            return {};
    }
}

/**
 * @param index                         An `OnTimer` or `OnLongTimer` instruction.
 * @return                              `Timer` or `RefillTimer`, and the arguments that follow the function in the
 *                                      call. `Timer` registers an `OnTimer` and `RefillTimer` an `OnLongTimer`.
 */
std::pair<std::string_view, std::string> EvtEventDecompiler::timerCall(int index) const {
    const auto &timer = _code[index].data.timer_descr;
    std::string_view name = _code[index].opcode == EVENT_OnTimer ? "Timer" : "RefillTimer";

    if (int64_t halfMinutes = timer.alt_halfmin_interval) {
        std::string count = halfMinutes % 2 ? fmt::format("{}.5", halfMinutes / 2) : fmt::format("{}", halfMinutes / 2);
        return {name, halfMinutes == 2 ? "const.Minute" : count + "*const.Minute"};
    }

    std::string_view period = "const.Day";
    if (timer.is_yearly) {
        period = "const.Year";
    } else if (timer.is_monthly) {
        period = "const.Month";
    } else if (timer.is_weekly) {
        period = "const.Week";
    }

    std::string start;
    for (auto [count, unit] : {std::pair(timer.daily_start_hour, "const.Hour"), std::pair(timer.daily_start_minute, "const.Minute"),
                               std::pair(timer.daily_start_second, "const.Minute/60")})
        if (count)
            start += fmt::format("{}{}*{}", start.empty() ? "" : " + ", count, unit);

    if (name == "Timer") // Without a start time `Timer` would count the period from the moment the map loads.
        return {name, fmt::format("{}, {}, false", period, start.empty() ? "0" : start)};
    return {name, start.empty() ? std::string(period) : fmt::format("{}, {}", period, start)};
}

/**
 * @return                              The line that gives the event its hint, or an empty string if it has none.
 */
std::string EvtEventDecompiler::hint() const {
    if (_isGlobal)
        return {}; // Hints belong to map events.

    std::optional<EvtHintSource> source = EvtProgram::hintSource(_instructions);
    if (!source)
        return {};

    if (source->houseId != HOUSE_INVALID) {
        int64_t houseId = std::to_underlying(source->houseId);
        return withComment(fmt::format("evt.house[{}] = {}", _eventId, houseId), houseName(houseId)) + "\n";
    }
    if (source->textId < 0 || source->textId >= _strings.size())
        return fmt::format("evt.hint[{}] = \"\"\n", _eventId);
    return withComment(fmt::format("evt.hint[{}] = evt.str[{}]", _eventId, source->textId), string(source->textId)) + "\n";
}

int EvtEventDecompiler::indexOfStep(int64_t step) const {
    auto pos = std::ranges::find(_code, step, &EvtInstruction::step);
    return pos == _code.end() ? END : pos - _code.begin();
}

int EvtEventDecompiler::nextIndex(int index) const {
    return indexOfStep(_code[index].step + 1);
}

/**
 * @param index                         Instruction to look at.
 * @param mode                          Which commands count.
 * @return                              Where control goes after the instruction, and the Lua text it turns into.
 */
EvtFlow EvtEventDecompiler::flow(int index, EvtMode mode) const {
    const EvtInstruction &ir = _code[index];

    if (mode == EVT_MODE_TOPIC) {
        switch (ir.opcode) {
            case EVENT_Exit:
            case EVENT_EndCanShowDialogItem:
                return {EVT_FLOW_STOP};
            case EVENT_OnCanShowDialogItemCmp: { // A topic's handler checks the whole party, as the interpreter does here.
                std::string condition = fmt::format("evt.Cmp({}, {})", formatVariable(ir.data.variable_descr.type), ir.data.variable_descr.value);
                return {EVT_FLOW_BRANCH, condition, {indexOfStep(ir.target_step)}};
            }
            case EVENT_SetCanShowDialogItem:
                return {EVT_FLOW_NEXT, fmt::format("visible = {}", ir.data.can_show_npc_dialogue ? "true" : "false")};
            default:
                return {EVT_FLOW_NEXT};
        }
    }

    switch (ir.opcode) {
        case EVENT_Exit:
        case EVENT_OnTimer:
        case EVENT_OnLongTimer:
        case EVENT_OnMapReload:
        case EVENT_OnMapLeave:
            return {EVT_FLOW_STOP};
        case EVENT_Jmp:
            return {EVT_FLOW_JUMP, {}, {indexOfStep(ir.target_step)}};
        case EVENT_RandomGoTo: {
            EvtFlow result = {EVT_FLOW_RANDOM};
            for (int i = 0; i < ir.data.random_goto_descr.random_goto_len; i++)
                result.targets.push_back(indexOfStep(ir.data.random_goto_descr.random_goto[i]));
            return result;
        }
        case EVENT_InputString: // The interpreter ends the event on it.
            return {EVT_FLOW_STOP, fmt::format("-- {} isn't supported by OpenEnroth.", ::toString(ir.opcode))};
        case EVENT_OnCanShowDialogItemCmp:
        case EVENT_EndCanShowDialogItem:
        case EVENT_SetCanShowDialogItem:
        case EVENT_CanShowTopic_IsActorKilled:
            return {EVT_FLOW_NEXT};
        case EVENT_ForPartyMember:
            return {EVT_FLOW_NEXT, fmt::format("evt.ForPlayer({})", formatPlayer(std::to_underlying(ir.who)))};
        default:
            break;
    }

    const EvtCommandInfo *info = evtCommand(ir.opcode);
    if (!info)
        return {EVT_FLOW_NEXT, fmt::format("-- {} isn't supported by OpenEnroth.", ::toString(ir.opcode))};
    if (info->kind == EVT_COMMAND_KIND_CONDITION)
        return {EVT_FLOW_BRANCH, formatCall(index, "evt."), {indexOfStep(ir.target_step)}};
    return {EVT_FLOW_NEXT, formatCall(index, "evt.")};
}

/**
 * @param entry                         Instruction to start from, or `END`.
 * @param mode                          Which commands count.
 * @return                              Instructions that control can get to from `entry`.
 */
std::set<int> EvtEventDecompiler::reachable(int entry, EvtMode mode) const {
    std::set<int> result;
    std::vector<int> queue = {entry};
    while (!queue.empty()) {
        int index = queue.back();
        queue.pop_back();
        if (index == END || !result.insert(index).second)
            continue;

        EvtFlow next = flow(index, mode);
        std::ranges::copy(next.targets, std::back_inserter(queue));
        if (next.kind == EVT_FLOW_NEXT || next.kind == EVT_FLOW_BRANCH)
            queue.push_back(nextIndex(index));
    }
    return result;
}

void EvtEventDecompiler::line(int depth, std::string_view text) {
    std::string result;
    for (int i = 0; i < depth; i++)
        result += INDENT;
    _lines.push_back(result + std::string(text));
}

std::string EvtEventDecompiler::jump(int target) {
    if (target == END)
        return std::string(RETURN);
    _labels.insert(target);
    return fmt::format("goto _{}", _code[target].step);
}

/**
 * @param lo                            First instruction of the would-be block.
 * @param hi                            One past its last instruction.
 * @param owner                         The condition that enters the block by jumping to `lo`, or `END` if the block
 *                                      is entered by falling into it.
 * @return                              Whether the instructions can sit in a nested Lua block, which a `goto` from
 *                                      outside can't enter.
 */
bool EvtEventDecompiler::canNest(int lo, int hi, int owner) const {
    for (int index : _reachable) {
        if (index >= lo && index < hi)
            continue;
        for (int target : flow(index, _mode).targets)
            if (target >= lo && target < hi && !(index == owner && target == lo))
                return false;
    }
    return true;
}

/**
 * Emits an instruction that is laid out the same way in a nested and in a flat listing, which is every kind but a
 * jump and a branch.
 *
 * @param index                         Instruction to emit.
 * @param flow                          Its flow.
 * @param depth                         Indentation depth.
 */
void EvtEventDecompiler::emitStatement(int index, const EvtFlow &flow, int depth) {
    assert(flow.kind == EVT_FLOW_NEXT || flow.kind == EVT_FLOW_STOP || flow.kind == EVT_FLOW_RANDOM);

    if (!flow.text.empty())
        line(depth, withComment(flow.text, _mode == EVT_MODE_EVENT ? comment(index) : std::string()));
    if (flow.kind == EVT_FLOW_STOP)
        line(depth, RETURN);
    if (flow.kind != EVT_FLOW_RANDOM)
        return;

    _usesRandom = true;
    line(depth, fmt::format("i = Game.Rand() % {}", flow.targets.size()));
    for (size_t i = 0; i < flow.targets.size(); i++) {
        bool isLast = i + 1 == flow.targets.size();
        line(depth, i == 0 ? "if i == 0 then" : isLast ? "else" : fmt::format("elseif i == {} then", i));
        line(depth + 1, jump(flow.targets[i]));
    }
    line(depth, "end");
}

/**
 * Emits what a flat listing needs after an instruction: nothing if control falls into `following`, a jump or a return
 * otherwise.
 *
 * @param index                         Instruction that was just emitted.
 * @param flow                          Its flow.
 * @param following                     Instruction that the listing puts next, or `END`.
 * @param depth                         Indentation depth.
 */
void EvtEventDecompiler::emitFlowTail(int index, const EvtFlow &flow, int following, int depth) {
    if (flow.kind != EVT_FLOW_NEXT && flow.kind != EVT_FLOW_BRANCH)
        return;
    int next = nextIndex(index);
    if (next == END) {
        line(depth, RETURN);
    } else if (next != following) {
        line(depth, jump(next));
    }
}

/**
 * Emits the instructions in `[lo, hi)` as structured code. A condition whose body nothing jumps into from outside
 * becomes an `if`, and the rest becomes `goto`.
 *
 * @param lo                            First instruction to emit.
 * @param hi                            One past the last instruction.
 * @param depth                         Indentation depth.
 */
void EvtEventDecompiler::emitBlock(int lo, int hi, int depth) {
    int index = lo;
    while (index < hi) {
        if (!_reachable.contains(index)) {
            index++;
            continue;
        }

        _lines.push_back(fmt::format("{}{}", LABEL_MARK, index));
        EvtFlow current = flow(index, _mode);

        if (current.kind == EVT_FLOW_JUMP) {
            if (current.targets[0] != index + 1)
                line(depth, jump(current.targets[0]));
        } else if (current.kind != EVT_FLOW_BRANCH) {
            emitStatement(index, current, depth);
        } else {
            std::string note = _mode == EVT_MODE_EVENT ? comment(index) : std::string();
            int target = current.targets[0];
            if (target != END && target > index && target <= hi && canNest(index + 1, target, END)) {
                int last = target - 1;
                while (last > index && !_reachable.contains(last))
                    last--;
                EvtFlow tail = last > index ? flow(last, _mode) : EvtFlow();
                int join = tail.kind == EVT_FLOW_JUMP ? tail.targets[0] : END;
                if (join != END && join > target && join <= hi && canNest(target, join, index)) {
                    // The body ends in a jump forward over what follows it, which makes what follows an else block.
                    line(depth, withComment(fmt::format("if {} then", current.text), note));
                    emitBlock(target, join, depth + 1);
                    line(depth, "else");
                    emitBlock(index + 1, last, depth + 1);
                    _lines.push_back(fmt::format("{}{}", LABEL_MARK, last));
                    line(depth, "end");
                    index = join;
                } else {
                    line(depth, withComment(fmt::format("if not {} then", current.text), note));
                    emitBlock(index + 1, target, depth + 1);
                    line(depth, "end");
                    index = target;
                }
                continue;
            }

            line(depth, withComment(fmt::format("if {} then", current.text), note));
            line(depth + 1, jump(target));
            line(depth, "end");
        }
        index++;
    }
}

/**
 * Emits the reachable instructions as a flat listing with a `goto` for every jump. It takes any order of steps.
 *
 * @param entry                         Instruction that the function starts from.
 */
void EvtEventDecompiler::emitFlat(int entry) {
    if (entry != *_reachable.begin())
        line(1, jump(entry));

    for (auto pos = _reachable.begin(); pos != _reachable.end(); pos++) {
        int index = *pos;
        int following = std::next(pos) == _reachable.end() ? END : *std::next(pos);

        _lines.push_back(fmt::format("{}{}", LABEL_MARK, index));
        EvtFlow current = flow(index, _mode);

        if (current.kind == EVT_FLOW_JUMP) {
            if (current.targets[0] != following || following == END)
                line(1, jump(current.targets[0]));
        } else if (current.kind != EVT_FLOW_BRANCH) {
            emitStatement(index, current, 1);
        } else {
            line(1, withComment(fmt::format("if {} then", current.text), _mode == EVT_MODE_EVENT ? comment(index) : std::string()));
            line(2, jump(current.targets[0]));
            line(1, "end");
        }
        emitFlowTail(index, current, following, 1);
    }
}

/**
 * @param entry                         Instruction that the function starts from, or `END` for an empty function.
 * @param mode                          Which commands count.
 * @param header                        Line that opens the function.
 * @param footer                        Line that closes it.
 * @return                              The function.
 */
std::string EvtEventDecompiler::function(int entry, EvtMode mode, std::string_view header, std::string_view footer) {
    _mode = mode;
    _reachable = reachable(entry, mode);
    _labels.clear();
    _lines.clear();
    _usesRandom = false;

    if (!_reachable.empty()) {
        // Nested blocks need the steps to follow each other, and nothing before the entry to be jumped back to.
        bool isLinear = *_reachable.begin() >= entry;
        for (int index : _reachable)
            isLinear = isLinear && (nextIndex(index) == index + 1 || (nextIndex(index) == END && index + 1 == _code.size()));
        if (isLinear) {
            emitBlock(entry, *_reachable.rbegin() + 1, 1);
        } else {
            emitFlat(entry);
        }
    }

    std::vector<std::string> body;
    for (const std::string &text : _lines) {
        if (text[0] != LABEL_MARK) {
            body.push_back(text);
        } else if (int index = std::stoi(text.substr(1)); _labels.contains(index)) {
            body.push_back(fmt::format("::_{}::", _code[index].step));
        }
    }

    std::string bareReturn = mode == EVT_MODE_TOPIC ? "return visible" : "return";
    for (size_t i = 0; i < body.size(); i++) {
        if (!body[i].ends_with(RETURN))
            continue;
        std::string_view next = i + 1 < body.size() ? std::string_view(body[i + 1]) : std::string_view();
        next.remove_prefix(std::min(next.find_first_not_of(' '), next.size()));
        bool isLastInBlock = i + 1 == body.size() || next.starts_with("end") || next.starts_with("else");
        body[i].replace(body[i].size() - RETURN.size(), RETURN.size(), isLastInBlock ? bareReturn : fmt::format("do {} end", bareReturn));
    }
    if (!body.empty() && body.back() == fmt::format("{}{}", INDENT, bareReturn))
        body.pop_back(); // The function ends here anyway.

    std::string result = fmt::format("{}\n", header);
    if (mode == EVT_MODE_TOPIC)
        result += fmt::format("{}local visible = true\n", INDENT);
    if (_usesRandom)
        result += fmt::format("{}local i\n", INDENT);
    for (const std::string &text : body)
        result += text + "\n";
    if (mode == EVT_MODE_TOPIC)
        result += fmt::format("{}return visible\n", INDENT);
    return result + std::string(footer) + "\n";
}

std::string EvtLuaScript::text() const {
    std::string result = header;
    for (const auto &[_, code] : events)
        result += code + "\n";
    return result;
}

EvtLuaScript decompileEvt(const EvtProgram &program, const std::vector<std::string> &strings, bool isGlobal) {
    EvtLuaScript result;
    if (!std::ranges::all_of(strings, &std::string::empty)) {
        result.header += "local TXT = Localize{\n";
        for (size_t i = 0; i < strings.size(); i++)
            if (!strings[i].empty())
                result.header += fmt::format("{}[{}] = {},\n", INDENT, i, luaString(strings[i]));
        result.header += "}\ntable.copy(TXT, evt.str, true)\n\n";
    }
    result.header += fmt::format("Game.{}EvtLines.Count = 0\n\n", isGlobal ? "Global" : "Map");

    for (int eventId : program.eventIds())
        if (std::string code = EvtEventDecompiler(eventId, program.function(eventId), strings, isGlobal).decompile(); !code.empty())
            result.events.emplace_back(eventId, std::move(code));
    return result;
}

EvtLuaScript decompileGameEvt(std::string_view name) {
    std::string fileName = ascii::toLower(name);
    bool isGlobal = fileName == "global";

    std::vector<std::string> strings;
    if (!isGlobal) {
        Blob blob = engine->resources()->eventsData(fileName + ".str");
        std::string_view data = blob.str(); // Strings with a zero after each.
        while (!data.empty()) {
            size_t size = std::min(data.find('\0'), data.size());
            strings.push_back(trimRemoveQuotes(data.substr(0, size)));
            data.remove_prefix(std::min(size + 1, data.size()));
        }
    }

    return decompileEvt(EvtProgram::load(engine->resources()->eventsData(fileName + ".evt")), strings, isGlobal);
}
