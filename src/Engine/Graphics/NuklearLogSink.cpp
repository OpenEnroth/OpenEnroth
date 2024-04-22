#include "NuklearLogSink.h"

#include <memory>
#include <string>
#include <sol/sol.hpp>

#include "Library/Serialization/Serialization.h"
#include "Nuklear.h"

void NuklearLogSink::write(const LogCategory& category, LogLevel level, std::string_view message) {
    sol::state_view luaState = Nuklear::getLuaState();
    if (luaState) {
        sol::protected_function logSink = luaState["logSink"];
        if (logSink) {
            logSink(toString(level).c_str(), message.data());
        }
    }
}

std::unique_ptr<LogSink> NuklearLogSink::createNuklearLogSink() {
    return std::make_unique<NuklearLogSink>();
}
