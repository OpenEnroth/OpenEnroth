#pragma once

#include "Application/Startup/GameStarterOptions.h"

#include "Utility/System/Path.h"

struct RetraceTestOptions : GameStarterOptions {
    Path testPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static RetraceTestOptions parse(int argc, char **argv);
};
