#pragma once

#include <string>

#include "Application/GameStarterOptions.h"

struct GameTestOptions : GameStarterOptions {
    std::string testPath;
    bool helpRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
