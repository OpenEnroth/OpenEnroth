#include "EvtBindings.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtDecompiler.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Tables/HouseTable.h"

#include "Library/Logger/Logger.h"

#include "Utility/Exception.h"
#include "Utility/String/Ascii.h"

#include "ScriptingSystem.h"

/**
 * What one running handler executes its commands in.
 */
struct EvtScriptContext {
    EvtInterpreter interpreter;
    int eventId = 0;
};

static const int JUMP_STEP = 2; // A command runs as step 0, so it continues from 1 unless it jumps.

static EvtTargetCharacter toPlayer(const sol::object &value, std::string_view what) {
    if (value.is<std::string>()) {
        if (std::optional<EvtTargetCharacter> result = evtPlayerByName(value.as<std::string>()))
            return *result;
        throw Exception("{}: unknown player '{}'", what, value.as<std::string>());
    }

    int result = value.as<int>();
    if (result < std::to_underlying(CHOOSE_PLAYER1) || result > std::to_underlying(CHOOSE_RANDOM))
        throw Exception("{}: player {} is out of range", what, result);
    return static_cast<EvtTargetCharacter>(result);
}

static EvtFieldValue toFieldValue(const EvtCommandInfo &command, const EvtFieldInfo &field, const sol::object &value, EvtTargetCharacter player) {
    std::string what = fmt::format("evt.{}: field {}", command.name, field.name);

    if (field.type == EVT_FIELD_JUMP)
        return int64_t(JUMP_STEP);

    if (!value.valid() || value.is<sol::lua_nil_t>()) {
        if (field.type == EVT_FIELD_STRING)
            return std::string();
        return int64_t(field.type == EVT_FIELD_PLAYER ? std::to_underlying(player) : 0);
    }

    switch (field.type) {
        case EVT_FIELD_STRING:
            if (!value.is<std::string>())
                throw Exception("{} takes a string", what);
            return value.as<std::string>();
        case EVT_FIELD_PLAYER:
            return int64_t(std::to_underlying(toPlayer(value, what)));
        case EVT_FIELD_VARIABLE:
            if (value.is<std::string>()) {
                if (std::optional<EvtVariable> variable = evtVariableByName(value.as<std::string>()))
                    return int64_t(std::to_underlying(*variable));
                throw Exception("{}: unknown variable '{}'", what, value.as<std::string>());
            }
            break;
        default:
            break;
    }

    if (value.is<bool>())
        return int64_t(value.as<bool>());
    if (!value.is<double>())
        throw Exception("{} takes a number", what);
    return int64_t(std::llround(value.as<double>()));
}

/**
 * @param command                       Command to run.
 * @param args                          Its arguments, by field name, by position, or a mix of the two.
 * @param player                        Current player of the script.
 * @return                              The command as an evt record.
 */
static EvtRecord toRecord(const EvtCommandInfo &command, const sol::table &args, EvtTargetCharacter player) {
    for (const auto &[key, _] : args) {
        if (key.is<std::string>()) {
            if (!std::ranges::contains(command.fields, key.as<std::string>(), &EvtFieldInfo::name))
                throw Exception("evt.{} has no field '{}'", command.name, key.as<std::string>());
        } else if (!key.is<int>() || key.as<int>() < 1 || key.as<int>() > command.fields.size()) {
            throw Exception("evt.{} takes {} arguments at most", command.name, command.fields.size());
        }
    }

    EvtRecord result;
    result.opcode = command.opcode;
    for (size_t i = 0; i < command.fields.size(); i++) {
        const EvtFieldInfo &field = command.fields[i];
        sol::object value = args[field.name];
        if (!value.valid() || value.is<sol::lua_nil_t>())
            value = args[i + 1];
        result.values.push_back(toFieldValue(command, field, value, player));
    }
    return result;
}

/**
 * @return                              Whether the command's condition held, or nil if it's not a condition. Then
 *                                      "ok", "exit" if the handler has to stop, or "wait" if it has to stop until
 *                                      the engine resumes the event.
 */
static std::tuple<sol::object, std::string> execute(EvtScriptContext &context, sol::this_state state, const std::string &name, const sol::table &args,
                                                    const sol::object &player) {
    const EvtCommandInfo *command = evtCommand(name);
    if (!command)
        throw Exception("evt.{} doesn't exist", name);
    if (command->kind != EVT_COMMAND_ACTION && command->kind != EVT_COMMAND_CONDITION)
        throw Exception("evt.{} can't be called from a script", name);

    EvtTargetCharacter who = toPlayer(player, "evt.Player");
    EvtInstruction ir = evtInstruction(toRecord(*command, args, who));

    context.interpreter.setTargetCharacter(who);
    int nextStep = context.interpreter.executeInstruction(ir);

    sol::object result = sol::make_object(state, sol::lua_nil);
    if (command->kind == EVT_COMMAND_CONDITION)
        result = sol::make_object(state, nextStep == JUMP_STEP);

    if (nextStep != -1 && !context.interpreter.isCancelled())
        return {result, "ok"};

    // The transition dialogue continues the event from `savedEventStep` if the party stays on the map.
    bool isWaiting = ir.opcode == EVENT_MoveToMap && (ir.data.move_map_descr.house_id != HOUSE_INVALID || ir.data.move_map_descr.exit_pic_id);
    return {result, isWaiting ? "wait" : "exit"};
}

static EvtProgram &program(bool isGlobal) {
    return isGlobal ? engine->_globalEventMap : engine->_localEventMap;
}

static std::vector<std::string> scriptsIn(std::string_view folder, std::function<bool(std::string_view)> filter) {
    std::vector<std::string> result;
    if (!dfs->exists(folder))
        return result;

    for (const DirectoryEntry &entry : dfs->ls(folder))
        if (entry.type == FILE_REGULAR && filter(ascii::toLower(entry.name)))
            result.push_back(fmt::format("{}/{}", folder, entry.name));
    std::ranges::sort(result);
    return result;
}

static std::tuple<sol::object, sol::object> loadChunk(sol::state_view lua, std::string_view code, const std::string &chunkName, const sol::table &environment) {
    sol::load_result chunk = lua.load(code, chunkName);
    if (!chunk.valid())
        return {sol::make_object(lua, sol::lua_nil), sol::make_object(lua, chunk.get<sol::error>().what())};
    sol::function result = chunk.get<sol::function>();
    lua["setfenv"](result, environment);
    return {result, sol::make_object(lua, sol::lua_nil)};
}

/**
 * @param period                        Timer period in ticks.
 * @param start                         Time of day in ticks for a daily timer, if the timer follows the calendar.
 * @return                              The `OnTimer` or `OnLongTimer` instruction that fires the same way.
 */
static EvtInstruction timerInstruction(int64_t period, std::optional<double> start) {
    EvtRecord record;
    if (!start) {
        int64_t halfMinutes = period / Duration::fromSeconds(30).ticks();
        if (halfMinutes < 1 || halfMinutes > 0xFFFF || halfMinutes * Duration::fromSeconds(30).ticks() != period)
            throw Exception("Timer: a period of {} ticks isn't a whole number of half minutes", period);
        record.opcode = EVENT_OnTimer;
        record.values = {int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), halfMinutes, int64_t(0)};
    } else {
        Duration timeOfDay = Duration::fromSeconds(std::llround(*start * Duration::fromSeconds(30).seconds() / Duration::fromSeconds(30).ticks()));
        bool isYearly = period == Duration::fromYears(1).ticks();
        bool isMonthly = period == Duration::fromDays(28).ticks();
        bool isWeekly = period == Duration::fromDays(7).ticks();
        if (!isYearly && !isMonthly && !isWeekly && period != Duration::fromDays(1).ticks())
            throw Exception("Timer: a calendar timer fires every const.Day, const.Week, const.Month or const.Year");
        if (timeOfDay < 0_ticks || timeOfDay >= Duration::fromDays(1))
            throw Exception("Timer: the start has to be a time of day");
        record.opcode = EVENT_OnLongTimer;
        record.values = {int64_t(isYearly), int64_t(isMonthly), int64_t(isWeekly), timeOfDay.hours() % 24, timeOfDay.minutes() % 60,
                         timeOfDay.seconds() % 60, int64_t(0), int64_t(0)};
    }
    return evtInstruction(record);
}

sol::table EvtBindings::createBindingTable(sol::state_view &solState) const {
    solState.new_usertype<EvtScriptContext>("EvtScriptContext", sol::no_constructor,
        "execute", &execute,
        "isMapExitTriggered", [](const EvtScriptContext &context) { return context.interpreter.isMapExitTriggered(); });

    return solState.create_table_with(
        "newContext", sol::as_function([](int eventId, int targetPid, bool canShowMessages) {
            auto result = std::make_unique<EvtScriptContext>();
            result->eventId = eventId;
            result->interpreter.prepare(eventId, Pid::fromPacked(targetPid), canShowMessages);
            return result;
        }),
        "defaultPlayer", sol::as_function([] {
            return std::to_underlying(pParty->hasActiveCharacter() ? CHOOSE_ACTIVE : CHOOSE_RANDOM);
        }),
        "commands", sol::as_function([](sol::this_state state) {
            sol::state_view lua(state);
            sol::table result = lua.create_table();
            for (const EvtCommandInfo &command : evtCommands())
                if (command.kind == EVT_COMMAND_ACTION || command.kind == EVT_COMMAND_CONDITION)
                    result.add(command.name);
            return result;
        }),
        "constants", sol::as_function([](sol::this_state state) {
            sol::state_view lua(state);
            sol::table result = lua.create_table();
            for (EvtConstGroup group : {EVT_CONST_FACET_BITS, EVT_CONST_MONSTER_BITS, EVT_CONST_CHEST_BITS}) {
                sol::table constants = lua.create_table();
                for (const auto &[name, value] : evtConstants(group))
                    constants[name] = static_cast<double>(value);
                result[evtConstGroupName(group)] = constants;
            }
            return result;
        }),
        "removeEvent", sol::as_function([](bool isGlobal, int eventId) { program(isGlobal).remove(eventId); }),
        "clearEvents", sol::as_function([](bool isGlobal) { program(isGlobal).clear(); }),
        "eventCount", sol::as_function([](bool isGlobal) { return program(isGlobal).eventCount(); }),
        "str", sol::as_function([](int index) -> std::optional<std::string> {
            if (index < 0 || index >= engine->_levelStrings.size())
                return std::nullopt;
            return engine->_levelStrings[index];
        }),
        "setStr", sol::as_function([](int index, std::string value) {
            if (index < 0 || index >= 500)
                throw Exception("evt.str index {} is out of bounds [0..499]", index);
            if (index >= engine->_levelStrings.size())
                engine->_levelStrings.resize(index + 1);
            engine->_levelStrings[index] = std::move(value);
        }),
        "houseName", sol::as_function([](int houseId) -> std::optional<std::string> {
            if (!houseTable.indices().contains(static_cast<HouseId>(houseId)))
                return std::nullopt;
            return houseTable[static_cast<HouseId>(houseId)].name;
        }),
        "random", sol::as_function([](int hi) { return grng->random(hi); }),
        "time", sol::as_function([] { return static_cast<double>(pParty->GetPlayingTime().ticks()); }),
        "addTimer", sol::as_function([](double period, std::optional<double> start, sol::protected_function callback) {
            addTimer(timerInstruction(std::llround(period), start), [callback] {
                sol::protected_function_result result = callback();
                if (!result.valid()) {
                    MM_ERROR_IN(ScriptingSystem::ScriptingLogCategory, "Timer failed: {}", result.get<sol::error>().what());
                } else if (result.get<bool>()) {
                    onMapLeave(); // The handler sent the party to another map.
                }
            });
        }),
        "mapScripts", sol::as_function([](std::string mapName) {
            std::string fileName = ascii::toLower(mapName) + ".lua";
            return sol::as_table(scriptsIn("scripts/maps", [&](std::string_view name) {
                return name == fileName || name.ends_with("." + fileName);
            }));
        }),
        "globalScripts", sol::as_function([] {
            return sol::as_table(scriptsIn("scripts/global", [](std::string_view name) { return name.ends_with(".lua"); }));
        }),
        "loadScript", sol::as_function([](sol::this_state state, std::string path, sol::table environment) {
            return loadChunk(state, dfs->read(path).str(), "@" + path, environment);
        }),
        "loadString", sol::as_function([](sol::this_state state, std::string code, std::string chunkName, sol::table environment) {
            return loadChunk(state, code, "=" + chunkName, environment);
        }),
        "decompile", sol::as_function([](std::string name, std::optional<std::vector<int>> skippedEvents) {
            return decompileGameEvt(name, skippedEvents.value_or(std::vector<int>()));
        }),
        "isDecompilingEvents", sol::as_function([] { return engine->config->debug.DecompiledEvents.value(); }),
        "questBit", sol::as_function([](int bit) { return pParty->_questBits.test(static_cast<QuestBit>(bit)); }),
        "setQuestBit", sol::as_function([](int bit, bool value) { pParty->_questBits.set(static_cast<QuestBit>(bit), value); }),
        "mouseItem", sol::as_function([] { return std::to_underlying(pParty->pPickedItem.itemId); })
    );
}
