#include "LoggerBindings.h"

#include "Library/Logger/Logger.h"

#include "ScriptingSystem.h"

sol::table LoggerBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "info", sol::as_function([](std::string_view message) {
            MM_INFO_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        }),
        "trace", sol::as_function([](std::string_view message) {
            MM_TRACE_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        }),
        "debug", sol::as_function([](std::string_view message) {
            MM_DEBUG_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        }),
        "warning", sol::as_function([](std::string_view message) {
            MM_WARNING_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        }),
        "error", sol::as_function([](std::string_view message) {
            MM_ERROR_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        }),
        "critical", sol::as_function([](std::string_view message) {
            MM_CRITICAL_IN(ScriptingSystem::ScriptingLogCategory, "{}", message);
        })
    );
}
