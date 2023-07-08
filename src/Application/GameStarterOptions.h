#pragma once

#include <string>
#include <optional>

#include "Platform/PlatformEnums.h"

struct GameStarterOptions {
    std::string configPath; // Path to config, empty means use default.
    std::string dataPath; // Path to game data, empty means use default.
    bool resetConfig = false; // Reset config to default on startup.
    std::optional<PlatformLogLevel> logLevel; // Override log level.
};
