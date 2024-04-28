#pragma once

#include <Library/Logger/LogSink.h>

#include <memory>
#include <optional>
#include <sol/sol.hpp>

class ScriptLogSink : public LogSink {
 public:
    explicit ScriptLogSink(std::weak_ptr<sol::state_view> solState);
    void write(const LogCategory& category, LogLevel level, std::string_view message) override;
 private:
    std::weak_ptr<sol::state_view> _solState;
};
