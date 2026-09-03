#pragma once

#include <cfloat>

#include "Application/Startup/GameStarterOptions.h"

#include "Utility/System/Path.h"

struct GameTestOptions : GameStarterOptions {
    Path testPath;
    float speed = FLT_MAX; // Test playback speed.
    bool helpPrinted = false;
    bool listRequested = false;

    static GameTestOptions parse(int argc, char **argv);
};
