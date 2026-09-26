#include "EvtBindings.h"

#include <algorithm>
#include <array>
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
#include "Engine/Evt/EvtScripts.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/NPCTable.h"

#include "Library/Logger/Logger.h"

#include "Utility/Exception.h"
#include "Utility/String/Ascii.h"

#include "ScriptingSystem.h"

/**
 * What one running handler executes its commands in.
 */
struct EvtScriptContext {
    EvtInterpreter interpreter;
};

static const int MAX_LEVEL_STRINGS = 1000; // Keeps a mistyped `evt.str` index from growing the string table without bound.

static int64_t toInteger(double value, std::string_view what) {
    if (!std::isfinite(value) || std::abs(value) >= 0x1p53 || value != std::trunc(value)) // Past 2^53 a double doesn't hold every integer.
        throw Exception("{} takes a whole number smaller than 2^53 in magnitude, got {}", what, value);
    return static_cast<int64_t>(value);
}

static EvtTargetCharacter toPlayer(const sol::object &value, std::string_view what) {
    if (value.is<std::string>()) {
        if (std::optional<EvtTargetCharacter> result = evtPlayerByName(value.as<std::string>()))
            return *result;
        throw Exception("{}: unknown player '{}'", what, value.as<std::string>());
    }

    if (!value.is<double>())
        throw Exception("{} takes a player number or name", what);
    int64_t result = toInteger(value.as<double>(), what);
    if (result < std::to_underlying(CHOOSE_PLAYER1) || result > std::to_underlying(CHOOSE_RANDOM))
        throw Exception("{}: player {} is out of range", what, result);
    return static_cast<EvtTargetCharacter>(result);
}

static EvtFieldValue toFieldValue(const EvtCommandInfo &command, const EvtFieldInfo &field, const sol::object &value, EvtTargetCharacter player) {
    std::string what = fmt::format("evt.{}: field {}", command.name, field.name);

    if (!value.valid() || value.is<sol::lua_nil_t>()) {
        switch (field.type) {
            case EVT_FIELD_STRING: return std::string();
            case EVT_FIELD_PLAYER: return int64_t(std::to_underlying(player));
            case EVT_FIELD_VARIABLE:
            case EVT_FIELD_HOUSE: throw Exception("{} is required", what);
            default: return std::clamp(int64_t(0), field.min, field.max);
        }
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
    return toInteger(value.as<double>(), what);
}

/**
 * @param command                       Command to run.
 * @param args                          Its arguments, by field name, by position, or a mix of the two.
 * @param player                        Current player of the script.
 * @return                              The command as an instruction.
 */
static EvtInstruction toInstruction(const EvtCommandInfo &command, const sol::table &args, EvtTargetCharacter player) {
    for (const auto &[key, _] : args) {
        if (key.is<std::string>()) {
            if (!std::ranges::contains(command.fields, key.as<std::string>(), &EvtFieldInfo::name))
                throw Exception("evt.{} has no field '{}'", command.name, key.as<std::string>());
        } else if (!key.is<int>() || key.as<int>() < 1 || key.as<int>() > command.fields.size()) {
            throw Exception("evt.{} takes {} arguments at most", command.name, command.fields.size());
        }
    }

    EvtInstruction result = {};
    result.opcode = command.opcode;
    for (size_t i = 0; i < command.fields.size(); i++) {
        const EvtFieldInfo &field = command.fields[i];
        if (!field.set)
            continue;
        sol::object value = args[field.name];
        if (!value.valid() || value.is<sol::lua_nil_t>())
            value = args[i + 1];
        EvtFieldValue fieldValue = toFieldValue(command, field, value, player);
        try {
            field.set(&result, fieldValue);
        } catch (const Exception &e) {
            throw Exception("evt.{}: {}", command.name, e.what());
        }
    }
    return result;
}

/**
 * Rejects a command that the interpreter can't run where the script runs it. The table's setters have already checked
 * the values that don't depend on that.
 *
 * @param command                       Command to check.
 * @param ir                            The command as an instruction.
 * @param who                           Player that the command applies to.
 * @throws Exception                    If the command can't run here.
 */
static void checkContext(const EvtCommandInfo &command, const EvtInstruction &ir, EvtTargetCharacter who) {
    switch (ir.opcode) {
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            if (activeLevelDecoration && (ir.data.text_id < 1 || ir.data.text_id > pNPCTopics.size())) // Then it's a topic text, from 1.
                throw Exception("evt.{}: topic text {} is out of range [1, {}]", command.name, ir.data.text_id, pNPCTopics.size());
            break;
        case EVENT_ChangeEvent:
            if (!activeLevelDecoration || activeLevelDecoration == reinterpret_cast<LevelDecoration *>(1))
                throw Exception("evt.{} works only in the event of a decoration", command.name);
            break;
        case EVENT_CheckSkill:
            if (who == CHOOSE_PARTY)
                throw Exception("evt.{} can't check the whole party, it checks one player", command.name);
            break;
        default:
            break;
    }
}

/**
 * @param context                       Context of the running handler.
 * @param state                         Lua state that gets the result.
 * @param name                          Command to run, e.g. "SetSprite".
 * @param args                          Its arguments, by field name, by position, or a mix of the two.
 * @param player                        Current player of the script, a number or a name.
 * @return                              Whether the command's condition held, or nil if it's not a condition. Then
 *                                      "ok", "exit" if the handler has to stop, or "wait" if it has to stop until
 *                                      the engine resumes the event.
 */
static std::tuple<sol::object, std::string> execute(EvtScriptContext &context, sol::this_state state, std::string_view name,
                                                    const sol::table &args, const sol::object &player) {
    const EvtCommandInfo *command = evtCommand(name);
    if (!command)
        throw Exception("evt.{} doesn't exist", name);

    EvtTargetCharacter who = toPlayer(player, "evt.Player");
    EvtInstruction ir = toInstruction(*command, args, who);
    checkContext(*command, ir, who);

    context.interpreter.setTargetCharacter(who);
    EvtResult next = context.interpreter.executeInstruction(ir);

    sol::object result = sol::make_object(state, sol::lua_nil);
    if (command->kind == EVT_COMMAND_KIND_CONDITION)
        result = sol::make_object(state, next.outcome == EVT_OUTCOME_JUMP);

    switch (next.outcome) {
        case EVT_OUTCOME_STOP:
            return {result, "exit"};
        case EVT_OUTCOME_WAIT:
            setEventContinuation([] { // Replaces the interpreter's, which would go on with an evt event.
                if (EvtScripts *scripts = evtScripts(); scripts && scripts->resumeEvent())
                    onMapLeave();
            });
            return {result, "wait"};
        default:
            return {result, "ok"};
    }
}

static EvtProgram &program(bool isGlobal) {
    return isGlobal ? engine->_globalEventMap : engine->_localEventMap;
}

/**
 * @param folder                        Folder to list.
 * @param filter                        Takes a file name in lower case and tells whether the file is wanted.
 * @return                              Paths of the wanted files, sorted.
 */
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

/**
 * @param lua                           Lua state to load into.
 * @param code                          Lua source.
 * @param chunkName                     Name that errors in the code are reported under.
 * @param environment                   Table that the code sees as its globals.
 * @return                              The loaded function and nil, or nil and the reason the code didn't load.
 */
static std::tuple<sol::object, sol::object> loadChunk(sol::state_view lua, std::string_view code, const std::string &chunkName,
                                                      const sol::table &environment) {
    sol::load_result chunk = lua.load(code, chunkName);
    if (!chunk.valid())
        return {sol::make_object(lua, sol::lua_nil), sol::make_object(lua, chunk.get<sol::error>().what())};
    sol::function result = chunk.get<sol::function>();
    lua["setfenv"](result, environment);
    return {result, sol::make_object(lua, sol::lua_nil)};
}

/**
 * @param period                        Timer period in ticks.
 * @param start                         Time of day in ticks that a daily timer fires at. A timer without it counts its
 *                                      period from the moment it's added, unless it's a refill timer with a calendar
 *                                      period. A daily one of those fires at midnight.
 * @param kind                          Which timers the timer is checked with.
 * @return                              When the timer fires.
 * @throws Exception                    If a timer can't fire like that.
 */
static EvtTimerSchedule timerSchedule(double period, std::optional<double> start, EvtTimerKind kind) {
    Duration duration = Duration::fromTicks(toInteger(period, "Timer: the period"));
    bool isCalendarPeriod = std::ranges::contains(std::array{Duration::fromDays(1), Duration::fromDays(7), Duration::fromDays(28), Duration::fromYears(1)},
                                                  duration);

    EvtTimerSchedule result;
    if (!start && !(kind == EVT_TIMER_KIND_REFILL && isCalendarPeriod)) {
        if (duration <= 0_ticks)
            throw Exception("Timer: the period has to be positive, got {} ticks", duration.ticks());
        result.interval = duration;
    } else {
        if (!isCalendarPeriod)
            throw Exception("Timer: a timer with a start time fires every const.Day, const.Week, const.Month or const.Year");
        if (start && *start != 0 && duration != Duration::fromDays(1))
            throw Exception("Timer: only a daily timer takes a start time");
        Duration timeOfDay = Duration::fromTicks(toInteger(std::floor(start.value_or(0)), "Timer: the start")); // Seconds are fractions of a tick.
        if (timeOfDay < 0_ticks || timeOfDay >= Duration::fromDays(1))
            throw Exception("Timer: the start has to be a time of day");
        result.period = duration;
        result.timeOfDay = timeOfDay;
    }
    return result;
}

sol::table EvtBindings::createBindingTable(sol::state_view &solState) const {
    solState.new_usertype<EvtScriptContext>("EvtScriptContext", sol::no_constructor,
        "execute", &execute,
        "isMapExitTriggered", [](const EvtScriptContext &context) { return context.interpreter.isMapExitTriggered(); });

    return solState.create_table_with(
        "newContext", sol::as_function([](int eventId, int targetPid, bool canShowMessages) {
            auto result = std::make_unique<EvtScriptContext>();
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
        "removeEvent", sol::as_function([](bool isGlobal, int eventId) {
            program(isGlobal).remove(eventId);
            if (!isGlobal)
                removeEventTriggers(eventId);
        }),
        "clearEvents", sol::as_function([](bool isGlobal) {
            program(isGlobal).clear();
            if (!isGlobal)
                removeEventTriggers(std::nullopt);
        }),
        "eventCount", sol::as_function([](bool isGlobal) { return program(isGlobal).eventCount(); }),
        "str", sol::as_function([](int index) -> std::optional<std::string> {
            if (index < 0 || index >= engine->_levelStrings.size())
                return std::nullopt;
            return engine->_levelStrings[index];
        }),
        "setStr", sol::as_function([](int index, std::string value) {
            if (index < 0 || index >= MAX_LEVEL_STRINGS)
                throw Exception("evt.str has room for strings 0 to {}, {} is outside", MAX_LEVEL_STRINGS - 1, index);
            if (index >= engine->_levelStrings.size())
                engine->_levelStrings.resize(index + 1);
            engine->_levelStrings[index] = std::move(value);
        }),
        "houseName", sol::as_function([](int houseId) -> std::optional<std::string> {
            if (!houseTable.indices().contains(static_cast<HouseId>(houseId)))
                return std::nullopt;
            return houseTable[static_cast<HouseId>(houseId)].name;
        }),
        "random", sol::as_function([](int hi) {
            if (hi < 1)
                throw Exception("Game.Rand() % {}: the modulus has to be positive", hi);
            return grng->random(hi);
        }),
        "time", sol::as_function([] { return static_cast<double>(pParty->GetPlayingTime().ticks()); }),
        "checkTimer", sol::as_function([](double period, std::optional<double> start, bool isRefill) {
            timerSchedule(period, start, isRefill ? EVT_TIMER_KIND_REFILL : EVT_TIMER_KIND_REGULAR);
        }),
        "addTimer", sol::as_function([](const sol::table &options, sol::main_protected_function callback) {
            EvtTimerKind kind = options.get_or("isRefill", false) ? EVT_TIMER_KIND_REFILL : EVT_TIMER_KIND_REGULAR;
            EvtTimerSchedule schedule = timerSchedule(options.get<double>("period"), options.get<std::optional<double>>("start"), kind);
            if (options.get_or("countsFromNow", false))
                schedule.start = EVT_TIMER_START_NOW;
            EvtTimerLifetime lifetime = options.get_or("isGlobal", false) ? EVT_TIMER_LIFETIME_GAME : EVT_TIMER_LIFETIME_MAP;
            return addTimer(schedule, kind, lifetime, [callback] {
                sol::protected_function_result result = callback();
                if (!result.valid()) {
                    MM_ERROR_IN(ScriptingSystem::ScriptingLogCategory, "Timer failed: {}", result.get<sol::error>().what());
                } else if (result.get<bool>()) {
                    onMapLeave(); // The handler sent the party to another map.
                }
            });
        }),
        "removeTimer", sol::as_function([](int handle) { removeTimer(handle); }),
        "mapScripts", sol::as_function([](std::string_view mapName) {
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
        "loadString", sol::as_function([](sol::this_state state, std::string_view code, std::string chunkName, sol::table environment) {
            return loadChunk(state, code, "=" + chunkName, environment);
        }),
        "decompile", sol::as_function([](sol::this_state state, std::string_view name) {
            sol::state_view lua(state);
            EvtLuaScript script = decompileGameEvt(name);
            sol::table events = lua.create_table();
            for (const auto &[eventId, code] : script.events)
                events.add(lua.create_table_with("id", eventId, "code", code));
            return lua.create_table_with("header", script.header, "events", events);
        }),
        "isDecompilingEvents", sol::as_function([] { return engine->config->debug.DecompiledEvents.value(); }),
        "questBit", sol::as_function([](int bit) { return pParty->_questBits.test(static_cast<QuestBit>(bit)); }),
        "setQuestBit", sol::as_function([](int bit, bool value) { pParty->_questBits.set(static_cast<QuestBit>(bit), value); }),
        "mouseItem", sol::as_function([] { return std::to_underlying(pParty->pPickedItem.itemId); })
    );
}
