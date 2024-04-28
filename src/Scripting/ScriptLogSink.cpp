#include "ScriptLogSink.h"

#include <sol/sol.hpp>

#include "Library/Serialization/Serialization.h"

ScriptLogSink::ScriptLogSink(sol::state_view solState) : _solState(solState) {}

void ScriptLogSink::write(const LogCategory& category, LogLevel level, std::string_view message) {
    if (sol::protected_function logSink = _solState["_globalLogSink"]) {
        logSink(toString(level), message);
    }
}
