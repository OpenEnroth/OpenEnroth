#pragma once

#include <string>

struct GameStarterOptions {
    std::string configPath; // Path to config, empty means use default.
    std::string dataPath; // Path to game data, empty means use default.
    bool resetConfig = false; // Reset config to default on startup.
    bool verbose = false;
};
