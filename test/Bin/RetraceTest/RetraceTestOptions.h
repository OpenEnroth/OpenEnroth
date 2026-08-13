#pragma once

#include "Application/Startup/GameStarterOptions.h"

#include "Utility/System/NativePath.h"

struct RetraceTestOptions : GameStarterOptions {
    NativePath testPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static RetraceTestOptions parse(int argc, char **argv);
};
