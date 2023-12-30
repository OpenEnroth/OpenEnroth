#pragma once

#include <cfloat>
#include <string>

#include "Application/GameStarterOptions.h"
#include "Engine/Graphics/Renderer/RendererEnums.h"

struct GameTestOptions : GameStarterOptions {
    std::string testPath;
    float speed = FLT_MAX; // Test playback speed.
    bool helpPrinted = false;
    bool listRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
