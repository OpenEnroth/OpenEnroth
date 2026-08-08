#pragma once

#include <cfloat>

#include "Application/Startup/GameStarterOptions.h"

#include "Utility/System/NativePath.h"

struct GameTestOptions : GameStarterOptions {
    NativePath testPath;
    float speed = FLT_MAX; // Test playback speed.
    bool helpPrinted = false;
    bool listRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
