#pragma once

#include <Library/Logger/LogSink.h>

#include <memory>
#include <sol/sol.hpp>

class ScriptLogSink : public LogSink {
 public:
    explicit ScriptLogSink(const sol::state_view &solState);
    void write(const LogCategory& category, LogLevel level, std::string_view message) override;

 private:
    sol::state_view _solState;
};
