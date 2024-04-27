#include "ScriptLogSink.h"

#include <memory>
#include <string>
#include <sol/sol.hpp>

#include "Library/Serialization/Serialization.h"

ScriptLogSink::ScriptLogSink(std::weak_ptr<sol::state_view> solState) : _solState(solState) {
}

void ScriptLogSink::write(const LogCategory& category, LogLevel level, std::string_view message) {
    if (auto state = _solState.lock()) {
        if (sol::protected_function logSink = (*state)["_globalLogSink"]) {
            logSink(toString(level), message);
        }
    }
}
