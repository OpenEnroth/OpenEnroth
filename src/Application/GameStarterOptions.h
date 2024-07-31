#pragma once

#include <string>
#include <optional>

#include "Library/Logger/LogEnums.h"

struct GameStarterOptions {
    bool useConfig = true; // Load external config & save it on exit?
    std::string configPath; // Path to config, empty means use default.
    std::string dataPath; // Path to game data, empty means use default.
    std::optional<LogLevel> logLevel; // Override log level.
    bool headless = false; // Run in headless mode.
    bool tracingRng = false; // Use tracing random engine?
};
