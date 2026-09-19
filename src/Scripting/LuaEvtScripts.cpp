#include "LuaEvtScripts.h"

#include <string>
#include <utility>

#include "Engine/Evt/Processor.h"

#include "Library/Logger/Logger.h"

#include "ScriptingSystem.h"

LuaEvtScripts::LuaEvtScripts(sol::state_view lua) {
    _core = lua.script("return require 'mmext.core'").get<sol::table>();
    setEvtScripts(this);
}

LuaEvtScripts::~LuaEvtScripts() {
    setEvtScripts(nullptr);
}

template<class... Args>
sol::object LuaEvtScripts::call(std::string_view function, Args &&... args) const {
    sol::protected_function_result result = _core.get<sol::protected_function>(function)(std::forward<Args>(args)...);
    if (!result.valid()) {
        MM_ERROR_IN(ScriptingSystem::ScriptingLogCategory, "mmext.core.{} failed: {}", function, result.get<sol::error>().what());
        return sol::make_object(_core.lua_state(), sol::lua_nil);
    }
    return result.get<sol::object>();
}

void LuaEvtScripts::loadGlobalScripts() {
    call("loadGlobalScripts");
}

void LuaEvtScripts::loadMapScripts(std::string_view mapName) {
    call("loadMapScripts", mapName);
}

bool LuaEvtScripts::hasEvent(bool isGlobal, int eventId) const {
    sol::object result = call("hasEvent", isGlobal, eventId);
    return result.is<bool>() && result.as<bool>();
}

bool LuaEvtScripts::runEvent(bool isGlobal, int eventId, Pid targetObj, bool canShowMessages) {
    sol::object result = call("runEvent", isGlobal, eventId, targetObj.packed(), canShowMessages);
    return result.is<bool>() && result.as<bool>();
}

bool LuaEvtScripts::resumeEvent(int eventId, bool *mapExitTriggered) {
    sol::object result = call("resumeEvent", eventId);
    if (!result.is<bool>())
        return false; // Nothing was waiting.
    *mapExitTriggered = result.as<bool>();
    return true;
}

void LuaEvtScripts::cancelEvent() {
    call("cancelEvent");
}

std::optional<std::string> LuaEvtScripts::eventHint(int eventId) const {
    sol::object result = call("eventHint", eventId);
    return result.is<std::string>() ? std::optional(result.as<std::string>()) : std::nullopt;
}

std::optional<bool> LuaEvtScripts::canShowTopic(int eventId) {
    sol::object result = call("canShowTopic", eventId);
    return result.is<bool>() ? std::optional(result.as<bool>()) : std::nullopt;
}

bool LuaEvtScripts::onMapLoad() {
    sol::object result = call("onMapLoad");
    return result.is<bool>() && result.as<bool>();
}

void LuaEvtScripts::onMapLeave() {
    call("onMapLeave");
}
