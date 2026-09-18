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
        return sol::object();
    }
    return result.get<sol::object>();
}

void LuaEvtScripts::loadGlobalScripts() {
    call("loadGlobalScripts");
}

void LuaEvtScripts::loadMapScripts(std::string_view mapName) {
    call("loadMapScripts", std::string(mapName));
}

bool LuaEvtScripts::hasEvent(bool isGlobal, int eventId) const {
    return call("hasEvent", isGlobal, eventId).as<bool>();
}

bool LuaEvtScripts::runEvent(bool isGlobal, int eventId, Pid targetObj, bool canShowMessages) {
    return call("runEvent", isGlobal, eventId, targetObj.packed(), canShowMessages).as<bool>();
}

bool LuaEvtScripts::resumeEvent(int eventId, bool *mapExitTriggered) {
    sol::object result = call("resumeEvent", eventId);
    if (!result.is<bool>())
        return false; // Nothing was waiting.
    *mapExitTriggered = result.as<bool>();
    return true;
}

std::optional<std::string> LuaEvtScripts::eventHint(int eventId) const {
    return call("eventHint", eventId).as<std::optional<std::string>>();
}

std::optional<bool> LuaEvtScripts::canShowTopic(int eventId) {
    return call("canShowTopic", eventId).as<std::optional<bool>>();
}

void LuaEvtScripts::onMapLoad() {
    call("onMapLoad");
}

void LuaEvtScripts::onMapLeave() {
    call("onMapLeave");
}
