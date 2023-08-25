#pragma once

#include <string>
#include "Application/GameStarterOptions.h"
#include "Engine/Graphics/RenderEnums.h"

struct GameTestOptions : GameStarterOptions {
    std::string testPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
