#pragma once

#include <sol/state_view.hpp>

#include "Library/Logger/LogSink.h"

class ScriptLogSink : public LogSink {
 public:
    explicit ScriptLogSink(sol::state_view solState);

    virtual void write(const LogCategory& category, LogLevel level, std::string_view message) override;

 private:
    sol::state_view _solState;
};
