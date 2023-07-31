#pragma once

#include <string>
#include <optional>

#include "Engine/Graphics/RenderEnums.h"

#include "Platform/PlatformEnums.h"

struct GameStarterOptions {
    bool useConfig = true; // Load external config & save it on exit?
    std::string configPath; // Path to config, empty means use default.
    std::string dataPath; // Path to game data, empty means use default.
    std::optional<PlatformLogLevel> logLevel; // Override log level.
    bool headless = false; // Run in headless mode.
};
