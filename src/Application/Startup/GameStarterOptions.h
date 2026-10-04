#pragma once

#include <optional>

#include "Library/Logger/LogEnums.h"

#include "Utility/System/Path.h"

struct GameStarterOptions {
    Path dataPath; // Path to game data.
    Path userPath; // Path to user data.
    std::optional<LogLevel> logLevel; // Override log level.
    bool ramFsUserData = false; // Use in-memory file system for user data, don't read/write config & saves
                                // from/to disk. This also means that default config will be used.
    bool headless = false; // Run in headless mode.
    bool tracingRng = false; // Use tracing random engine?
    bool quickStart = false; // Skip whatever slow initialization that we have, including additional asset generation.
};
