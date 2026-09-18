#include "EvtDecompiler.h"

#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Evt/EvtCommands.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Tables/QuestTable.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

static const int END = -1; // Index that stands for the end of the event.
static const char LABEL_MARK = '\x01'; // Starts a line that turns into a label if something jumps to it.
static const std::string_view INDENT = "    ";
static const std::string_view RETURN = "do return end"; // Lua takes a bare return only as the last statement of a block.
static const std::array<std::string_view, 4> MASTERY_NAMES = {"const.Novice", "const.Expert", "const.Master", "const.GM"};

enum class EvtFlowKind {
    FLOW_NEXT, // Runs and falls through.
    FLOW_STOP, // Ends the event.
    FLOW_JUMP,
    FLOW_BRANCH, // Jumps if its condition holds.
    FLOW_RANDOM, // Jumps to one of its targets.
};
using enum EvtFlowKind;

struct EvtFlow {
    EvtFlowKind kind = FLOW_NEXT;
    std::string text; // Statement of `FLOW_NEXT`, which can be empty, or condition of `FLOW_BRANCH`.
    std::vector<int> targets; // Instruction indices, or `END`.
};

/**
 * An NPC topic's event runs in two ways. The engine runs it to decide whether to show the topic, and then only the
 * `CanShowTopic` commands count. Everything else runs when the topic is picked.
 */
enum class EvtMode {
    MODE_EVENT,
    MODE_TOPIC,
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

static std::string withComment(std::string statement, std::string_view comment) {
    if (!comment.empty())
        statement += fmt::format("  -- {}", luaString(comment));
    return statement;
}

class EventDecompiler {
 public:
    EventDecompiler(int eventId, const std::vector<EvtRecord> &records, const std::vector<std::string> &strings, bool isGlobal);

    std::string decompile();

 private:
    const EvtCommandInfo *command(int index) const;
    int64_t value(int index, std::string_view field) const;
    std::string string(int64_t index) const;

    std::string formatValue(const EvtFieldInfo &field, const EvtFieldValue &value) const;
    std::string formatCall(int index, std::string_view prefix) const;
    std::string comment(int index) const;
    std::string timerCall(int index) const;

    int indexOfStep(int64_t step) const;
    int nextIndex(int index) const;
    EvtFlow flow(int index, EvtMode mode) const;
    std::set<int> reachable(int entry, EvtMode mode) const;

    bool canNest(int lo, int hi, int owner) const;
    std::string jump(int target);
    void emitFlowTail(int index, const EvtFlow &flow, int following, int depth);
    void emitBlock(int lo, int hi, int depth);
    void emitFlat(int entry);
    std::string function(int entry, EvtMode mode, std::string_view header, std::string_view footer);

    void line(int depth, std::string_view text);

 private:
    int _eventId = 0;
    std::vector<EvtRecord> _code; // Without the hint records.
    std::optional<int64_t> _hint;
    const std::vector<std::string> &_strings;
    bool _isGlobal = false;

    // State of the function being emitted.
    EvtMode _mode = MODE_EVENT;
    std::set<int> _reachable;
    std::set<int> _labels;
    std::vector<std::string> _lines;
    bool _usesRandom = false;
};

EventDecompiler::EventDecompiler(int eventId, const std::vector<EvtRecord> &records, const std::vector<std::string> &strings, bool isGlobal)
    : _eventId(eventId), _strings(strings), _isGlobal(isGlobal) {
    for (const EvtRecord &record : records) {
        if (record.opcode == EVENT_MouseOver && !record.values.empty()) {
            _hint = std::get<int64_t>(record.values[0]);
        } else if (record.opcode != EVENT_MouseOver && record.opcode != EVENT_LocationName) {
            _code.push_back(record);
        }
    }
}

const EvtCommandInfo *EventDecompiler::command(int index) const {
    return _code[index].values.empty() && !_code[index].payload.empty() ? nullptr : evtCommand(_code[index].opcode);
}

int64_t EventDecompiler::value(int index, std::string_view field) const {
    const EvtCommandInfo *info = command(index);
    auto pos = std::ranges::find(info->fields, field, &EvtFieldInfo::name);
    return std::get<int64_t>(_code[index].values[pos - info->fields.begin()]);
}

std::string EventDecompiler::string(int64_t index) const {
    return index >= 0 && index < _strings.size() ? _strings[index] : std::string();
}

std::string EventDecompiler::formatValue(const EvtFieldInfo &field, const EvtFieldValue &value) const {
    if (field.type == EVT_FIELD_STRING)
        return luaString(std::get<std::string>(value));

    int64_t number = std::get<int64_t>(value);
    switch (field.type) {
        case EVT_FIELD_BOOL:
            if (number == 0 || number == 1)
                return number ? "true" : "false";
            break;
        case EVT_FIELD_PLAYER:
            if (number >= std::to_underlying(CHOOSE_ACTIVE) && number <= std::to_underlying(CHOOSE_RANDOM))
                return luaString(evtPlayerName(static_cast<EvtTargetCharacter>(number)));
            break;
        case EVT_FIELD_VARIABLE:
            if (std::string name = evtVariableName(static_cast<EvtVariable>(number)); !name.empty())
                return luaString(name);
            break;
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

std::string EventDecompiler::formatCall(int index, std::string_view prefix) const {
    const EvtCommandInfo *info = command(index);
    const EvtRecord &record = _code[index];

    std::vector<std::pair<std::string_view, std::string>> args;
    for (size_t i = 0; i < info->fields.size(); i++)
        if (info->fields[i].type != EVT_FIELD_JUMP)
            args.emplace_back(info->fields[i].name, formatValue(info->fields[i], record.values[i]));

    bool isPositional = args.size() <= 1 || std::ranges::contains(std::array{EVENT_Compare, EVENT_Add, EVENT_Subtract, EVENT_Set}, record.opcode);
    std::string result = fmt::format("{}{}{}", prefix, info->name, isPositional ? "(" : "{");
    for (size_t i = 0; i < args.size(); i++)
        result += fmt::format("{}{}{}", i ? ", " : "", isPositional ? "" : fmt::format("{} = ", args[i].first), args[i].second);
    return result + (isPositional ? ")" : "}");
}

std::string EventDecompiler::comment(int index) const {
    switch (_code[index].opcode) {
        case EVENT_Compare:
        case EVENT_Add:
        case EVENT_Subtract:
        case EVENT_Set:
        case EVENT_OnCanShowDialogItemCmp:
            if (value(index, "VarNum") == std::to_underlying(VAR_QBits_QuestsDone))
                return questName(value(index, "Value"));
            if (value(index, "VarNum") == std::to_underlying(VAR_PlayerItemInHands))
                return itemName(value(index, "Value"));
            return {};
        case EVENT_SpeakInHouse:
            return houseName(value(index, "Id"));
        case EVENT_SpeakNPC:
        case EVENT_SetNPCTopic:
        case EVENT_MoveNPC:
        case EVENT_SetNPCGreeting:
        case EVENT_NPCSetItem:
            return npcName(value(index, "NPC"));
        case EVENT_GiveItem:
            return itemName(value(index, "Id"));
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            return _isGlobal ? std::string() : string(value(index, "Str"));
        default:
            return {};
    }
}

std::string EventDecompiler::timerCall(int index) const {
    int64_t halfMinutes = value(index, "IntervalInHalfMinutes");
    if (halfMinutes) {
        std::string count = halfMinutes % 2 ? fmt::format("{}.5", halfMinutes / 2) : fmt::format("{}", halfMinutes / 2);
        return fmt::format("Timer(function()|end, {})", halfMinutes == 2 ? "const.Minute" : count + "*const.Minute");
    }

    std::string_view period = value(index, "EachYear") ? "const.Year" : value(index, "EachMonth") ? "const.Month" : value(index, "EachWeek") ? "const.Week" : "const.Day";
    std::string start;
    for (auto [field, unit] : {std::pair("Hour", "const.Hour"), std::pair("Minute", "const.Minute"), std::pair("Second", "const.Second")})
        if (int64_t count = value(index, field))
            start += fmt::format("{}{}*{}", start.empty() ? "" : " + ", count, unit);
    return fmt::format("RefillTimer(function()|end, {}{})", period, start.empty() ? "" : ", " + start);
}

int EventDecompiler::indexOfStep(int64_t step) const {
    auto pos = std::ranges::find(_code, step, &EvtRecord::step);
    return pos == _code.end() ? END : pos - _code.begin();
}

int EventDecompiler::nextIndex(int index) const {
    return indexOfStep(_code[index].step + 1);
}

EvtFlow EventDecompiler::flow(int index, EvtMode mode) const {
    const EvtRecord &record = _code[index];
    const EvtCommandInfo *info = command(index);
    if (!info)
        return {FLOW_NEXT, fmt::format("-- Command {} with {} bytes of data isn't known to OpenEnroth.", std::to_underlying(record.opcode), record.payload.size())};

    if (mode == MODE_TOPIC) {
        switch (record.opcode) {
            case EVENT_Exit:
            case EVENT_EndCanShowDialogItem:
                return {FLOW_STOP};
            case EVENT_OnCanShowDialogItemCmp:
                return {FLOW_BRANCH, fmt::format("evt.All.Cmp({}, {})", formatValue(info->fields[0], record.values[0]), value(index, "Value")),
                        {indexOfStep(value(index, "jump"))}};
            case EVENT_SetCanShowDialogItem:
                return {FLOW_NEXT, fmt::format("visible = {}", value(index, "Visible") ? "true" : "false")};
            default:
                return {FLOW_NEXT};
        }
    }

    switch (record.opcode) {
        case EVENT_Exit:
        case EVENT_OnTimer:
        case EVENT_OnLongTimer:
        case EVENT_OnMapReload:
        case EVENT_OnMapLeave:
            return {FLOW_STOP};
        case EVENT_Jmp:
            return {FLOW_JUMP, {}, {indexOfStep(value(index, "jump"))}};
        case EVENT_RandomGoTo: {
            // The interpreter counts the non-zero steps and then picks among that many of the leading ones.
            EvtFlow result = {FLOW_RANDOM};
            int64_t count = std::ranges::count_if(record.values, [](const EvtFieldValue &step) { return std::get<int64_t>(step) > 0; });
            for (int64_t i = 0; i < count; i++)
                result.targets.push_back(indexOfStep(std::get<int64_t>(record.values[i])));
            return result;
        }
        case EVENT_OnCanShowDialogItemCmp:
        case EVENT_EndCanShowDialogItem:
        case EVENT_SetCanShowDialogItem:
        case EVENT_CanShowTopic_IsActorKilled:
            return {FLOW_NEXT};
        case EVENT_ForPartyMember:
            return {FLOW_NEXT, formatCall(index, "evt.")};
        default:
            break;
    }

    if (info->kind == EVT_COMMAND_CONDITION)
        return {FLOW_BRANCH, formatCall(index, "evt."), {indexOfStep(value(index, "jump"))}};
    if (info->kind == EVT_COMMAND_ACTION)
        return {FLOW_NEXT, formatCall(index, "evt.")};
    return {FLOW_NEXT, fmt::format("-- {} isn't supported by OpenEnroth.", formatCall(index, "evt."))};
}

std::set<int> EventDecompiler::reachable(int entry, EvtMode mode) const {
    std::set<int> result;
    std::vector<int> queue = {entry};
    while (!queue.empty()) {
        int index = queue.back();
        queue.pop_back();
        if (index == END || !result.insert(index).second)
            continue;

        EvtFlow next = flow(index, mode);
        std::ranges::copy(next.targets, std::back_inserter(queue));
        if (next.kind == FLOW_NEXT || next.kind == FLOW_BRANCH)
            queue.push_back(nextIndex(index));
    }
    return result;
}

void EventDecompiler::line(int depth, std::string_view text) {
    std::string result;
    for (int i = 0; i < depth; i++)
        result += INDENT;
    _lines.push_back(result + std::string(text));
}

std::string EventDecompiler::jump(int target) {
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
bool EventDecompiler::canNest(int lo, int hi, int owner) const {
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
 * Emits what a flat listing needs after instruction `index`: nothing if control falls into `following`, a jump or a
 * return otherwise.
 */
void EventDecompiler::emitFlowTail(int index, const EvtFlow &flow, int following, int depth) {
    if (flow.kind != FLOW_NEXT && flow.kind != FLOW_BRANCH)
        return;
    int next = nextIndex(index);
    if (next == END) {
        line(depth, RETURN);
    } else if (next != following) {
        line(depth, jump(next));
    }
}

void EventDecompiler::emitBlock(int lo, int hi, int depth) {
    int index = lo;
    while (index < hi) {
        if (!_reachable.contains(index)) {
            index++;
            continue;
        }

        _lines.push_back(fmt::format("{}{}", LABEL_MARK, index));
        EvtFlow current = flow(index, _mode);
        std::string note = _mode == MODE_EVENT && command(index) ? comment(index) : std::string();

        if (current.kind == FLOW_NEXT) {
            if (!current.text.empty())
                line(depth, withComment(current.text, note));
        } else if (current.kind == FLOW_STOP) {
            line(depth, RETURN);
        } else if (current.kind == FLOW_JUMP) {
            if (current.targets[0] != index + 1)
                line(depth, jump(current.targets[0]));
        } else if (current.kind == FLOW_RANDOM) {
            _usesRandom = true;
            line(depth, fmt::format("i = Game.Rand() % {}", current.targets.size()));
            for (size_t i = 0; i < current.targets.size(); i++) {
                bool isLast = i + 1 == current.targets.size();
                line(depth, i == 0 ? "if i == 0 then" : isLast ? "else" : fmt::format("elseif i == {} then", i));
                line(depth + 1, jump(current.targets[i]));
            }
            line(depth, "end");
        } else {
            int target = current.targets[0];
            if (target != END && target > index && target <= hi && canNest(index + 1, target, END)) {
                // The last instruction of the body that jumps forward over what follows makes that an else block.
                int last = target - 1;
                while (last > index && !_reachable.contains(last))
                    last--;
                EvtFlow tail = last > index ? flow(last, _mode) : EvtFlow();
                int join = tail.kind == FLOW_JUMP ? tail.targets[0] : END;
                if (join != END && join > target && join <= hi && canNest(target, join, index)) {
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

void EventDecompiler::emitFlat(int entry) {
    if (entry != *_reachable.begin())
        line(1, jump(entry));

    for (auto pos = _reachable.begin(); pos != _reachable.end(); pos++) {
        int index = *pos;
        int following = std::next(pos) == _reachable.end() ? END : *std::next(pos);

        _lines.push_back(fmt::format("{}{}", LABEL_MARK, index));
        EvtFlow current = flow(index, _mode);
        std::string note = _mode == MODE_EVENT && command(index) ? comment(index) : std::string();

        if (current.kind == FLOW_NEXT) {
            if (!current.text.empty())
                line(1, withComment(current.text, note));
        } else if (current.kind == FLOW_STOP) {
            line(1, RETURN);
        } else if (current.kind == FLOW_JUMP) {
            if (current.targets[0] != following || following == END)
                line(1, jump(current.targets[0]));
        } else if (current.kind == FLOW_RANDOM) {
            _usesRandom = true;
            line(1, fmt::format("i = Game.Rand() % {}", current.targets.size()));
            for (size_t i = 0; i < current.targets.size(); i++) {
                bool isLast = i + 1 == current.targets.size();
                line(1, i == 0 ? "if i == 0 then" : isLast ? "else" : fmt::format("elseif i == {} then", i));
                line(2, jump(current.targets[i]));
            }
            line(1, "end");
        } else {
            line(1, withComment(fmt::format("if {} then", current.text), note));
            line(2, jump(current.targets[0]));
            line(1, "end");
        }
        emitFlowTail(index, current, following, 1);
    }
}

/**
 * @param entry                         Instruction that the function starts from.
 * @param mode                          Which commands count.
 * @param header                        Line that opens the function.
 * @param footer                        Line that closes it.
 * @return                              The function, or an empty string if it has no body.
 */
std::string EventDecompiler::function(int entry, EvtMode mode, std::string_view header, std::string_view footer) {
    _mode = mode;
    _reachable = reachable(entry, mode);
    _labels.clear();
    _lines.clear();
    _usesRandom = false;
    if (_reachable.empty())
        return {};

    // Nested blocks need the steps to follow each other, and nothing before the entry to be jumped back to.
    bool isLinear = *_reachable.begin() >= entry;
    for (int index : _reachable)
        isLinear = isLinear && (nextIndex(index) == index + 1 || (nextIndex(index) == END && index + 1 == _code.size()));
    if (isLinear) {
        emitBlock(entry, *_reachable.rbegin() + 1, 1);
    } else {
        emitFlat(entry);
    }

    std::vector<std::string> body;
    for (const std::string &text : _lines) {
        if (text[0] != LABEL_MARK) {
            body.push_back(text);
        } else if (int index = std::stoi(text.substr(1)); _labels.contains(index)) {
            body.push_back(fmt::format("::_{}::", _code[index].step));
        }
    }

    std::string bareReturn = mode == MODE_TOPIC ? "return visible" : "return";
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
    if (mode == MODE_TOPIC)
        result += fmt::format("{}local visible = true\n", INDENT);
    if (_usesRandom)
        result += fmt::format("{}local i\n", INDENT);
    for (const std::string &text : body)
        result += text + "\n";
    if (mode == MODE_TOPIC)
        result += fmt::format("{}return visible\n", INDENT);
    return result + std::string(footer) + "\n";
}

std::string EventDecompiler::decompile() {
    std::string_view events = _isGlobal ? "global" : "map";
    std::string result;

    auto house = std::ranges::find(_code, EVENT_SpeakInHouse, &EvtRecord::opcode);
    if (_hint && house != _code.end() && !house->values.empty()) {
        int64_t houseId = std::get<int64_t>(house->values[0]);
        result += withComment(fmt::format("evt.house[{}] = {}", _eventId, houseId), houseName(houseId)) + "\n";
    } else if (_hint) {
        result += withComment(fmt::format("evt.hint[{}] = evt.str[{}]", _eventId, *_hint), string(*_hint)) + "\n";
    }
    if (_code.empty())
        return result;

    bool hasTopicCommands = std::ranges::any_of(_code, [](const EvtRecord &record) {
        return record.opcode == EVENT_OnCanShowDialogItemCmp || record.opcode == EVENT_SetCanShowDialogItem;
    });
    if (hasTopicCommands)
        result += function(0, MODE_TOPIC, fmt::format("evt.CanShowTopic[{}] = function()", _eventId), "end");

    // An event that does nothing still has to exist, or a click on it reports that there's nothing here. One that
    // only shows a hint is the exception, it's the hint that makes it exist.
    std::string main = function(0, MODE_EVENT, fmt::format("evt.{}[{}] = function()", events, _eventId), "end");
    bool isEmpty = std::ranges::count(main, '\n') <= 2;
    if (!isEmpty || !_hint)
        result += main;

    for (int index = 0; index < _code.size(); index++) {
        if (!command(index) || nextIndex(index) == END)
            continue;

        std::string header, footer = "end";
        if (_code[index].opcode == EVENT_OnMapReload) {
            header = "function events.LoadMap()";
        } else if (_code[index].opcode == EVENT_OnMapLeave) {
            header = "function events.LeaveMap()";
        } else if (_code[index].opcode == EVENT_OnTimer || _code[index].opcode == EVENT_OnLongTimer) {
            std::string call = timerCall(index);
            header = call.substr(0, call.find('|'));
            footer = call.substr(call.find('|') + 1);
        } else {
            continue;
        }
        result += function(nextIndex(index), MODE_EVENT, header, footer);
    }

    return result;
}

std::string decompileEvt(std::span<const EvtRecord> records, const std::vector<std::string> &strings, bool isGlobal) {
    std::map<int, std::vector<EvtRecord>> recordsByEvent;
    for (const EvtRecord &record : records)
        recordsByEvent[record.eventId].push_back(record);

    std::string result;
    if (!std::ranges::all_of(strings, &std::string::empty)) {
        result += "local TXT = Localize{\n";
        for (size_t i = 0; i < strings.size(); i++)
            if (!strings[i].empty())
                result += fmt::format("{}[{}] = {},\n", INDENT, i, luaString(strings[i]));
        result += "}\ntable.copy(TXT, evt.str, true)\n\n";
    }
    result += fmt::format("Game.{}EvtLines.Count = 0  -- Deactivate all standard events\n\n", isGlobal ? "Global" : "Map");

    for (const auto &[eventId, eventRecords] : recordsByEvent)
        if (std::string event = EventDecompiler(eventId, eventRecords, strings, isGlobal).decompile(); !event.empty())
            result += event + "\n";
    return result;
}

std::string decompileGameEvt(std::string_view name, std::span<const int> skippedEvents) {
    std::string fileName = ascii::toLower(name);
    bool isGlobal = fileName == "global";

    std::vector<std::string> strings;
    if (!isGlobal) {
        Blob blob = engine->resources()->eventsData(fileName + ".str");
        std::string_view data = blob.str(); // Strings with a zero after each.
        while (!data.empty()) {
            strings.push_back(trimRemoveQuotes(data.substr(0, data.find('\0'))));
            data.remove_prefix(std::min(data.size(), data.find('\0') + 1));
        }
    }

    std::vector<EvtRecord> records = decodeEvtRecords(engine->resources()->eventsData(fileName + ".evt"));
    std::erase_if(records, [&](const EvtRecord &record) { return std::ranges::contains(skippedEvents, record.eventId); });
    return decompileEvt(records, strings, isGlobal);
}
