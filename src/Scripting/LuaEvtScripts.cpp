#include "LuaEvtScripts.h"

#include <string>
#include <utility>

LuaEvtScripts::LuaEvtScripts(sol::state_view lua) {
    _core = lua.script("return require 'mmext.core'").get<sol::table>();
}

template<class... Args>
sol::object LuaEvtScripts::call(std::string_view function, Args &&... args) const {
    sol::protected_function protectedFunction = _core.get<sol::protected_function>(function);
    sol::function errorHandler = sol::state_view(_core.lua_state())["_globalErrorHandler"];
    if (errorHandler.valid())
        protectedFunction.set_error_handler(errorHandler);

    sol::protected_function_result result = protectedFunction(std::forward<Args>(args)...);
    if (!result.valid())
        return sol::make_object(_core.lua_state(), sol::lua_nil);
    return result.get<sol::object>();
}

template<class... Args>
EvtTransition LuaEvtScripts::callForTransition(std::string_view function, Args &&... args) const {
    sol::object result = call(function, std::forward<Args>(args)...);
    return result.is<bool>() && result.as<bool>() ? EVT_TRANSITION_STARTED : EVT_TRANSITION_NONE;
}

void LuaEvtScripts::loadGlobalScripts() {
    call("loadGlobalScripts");
}

void LuaEvtScripts::loadMapScripts(std::string_view mapName) {
    call("loadMapScripts", mapName);
}

bool LuaEvtScripts::hasEvent(EvtSource source, int eventId) const {
    sol::object result = call("hasEvent", source == EVT_SOURCE_GLOBAL, eventId);
    return result.is<bool>() && result.as<bool>();
}

EvtTransition LuaEvtScripts::runEvent(const EvtContext &context) {
    return callForTransition("runEvent", context.source == EVT_SOURCE_GLOBAL, context.eventId, context.objectPid.packed(), context.duringMapLoad);
}

EvtTransition LuaEvtScripts::resumeEvent(int frame) {
    return callForTransition("resumeEvent", frame);
}

EvtTransition LuaEvtScripts::fireTimer(int handle) {
    return callForTransition("fireTimer", handle);
}

std::optional<std::string> LuaEvtScripts::eventHint(int eventId) const {
    sol::object result = call("eventHint", eventId);
    return result.is<std::string>() ? std::optional(result.as<std::string>()) : std::nullopt;
}

std::optional<bool> LuaEvtScripts::canShowTopic(int eventId) {
    sol::object result = call("canShowTopic", eventId);
    return result.is<bool>() ? std::optional(result.as<bool>()) : std::nullopt;
}

EvtTransition LuaEvtScripts::onMapLoad() {
    return callForTransition("onMapLoad");
}

void LuaEvtScripts::onMapLeave() {
    call("onMapLeave");
}
