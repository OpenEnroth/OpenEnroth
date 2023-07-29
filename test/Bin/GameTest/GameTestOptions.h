#pragma once

#include <string>

#include "Application/GameStarterOptions.h"
#include "Engine/Graphics/RenderEnums.h"

struct GameTestOptions : GameStarterOptions {
    std::string testPath;
    bool helpRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
