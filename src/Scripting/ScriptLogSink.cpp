#include "ScriptLogSink.h"

#include <memory>
#include <string>
#include <sol/sol.hpp>

#include "Library/Serialization/Serialization.h"

ScriptLogSink::ScriptLogSink(const sol::state_view &solState) : _solState(solState) {
}

void ScriptLogSink::write(const LogCategory& category, LogLevel level, std::string_view message) {
    sol::protected_function logSink = _solState["logSink"];
    if (logSink) {
        logSink(toString(level).c_str(), message.data());
    }
}
