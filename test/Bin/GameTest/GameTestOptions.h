#pragma once

#include <string>

#include "Application/GameStarterOptions.h"
#include "Engine/Graphics/RenderEnums.h"

struct GameTestOptions : GameStarterOptions {
    std::string testPath;
    bool helpRequested = false;

    // TODO(captainurist): belongs to GameStarterOptions.
    std::optional<RendererType> renderer; // Override renderer type.

    static GameTestOptions parse(int argc, char **argv);
};
