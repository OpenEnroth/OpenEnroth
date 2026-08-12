#pragma once

#include <string>

#include "Application/Startup/GameStarterOptions.h"

struct RetraceTestOptions : GameStarterOptions {
    std::string testPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static RetraceTestOptions parse(int argc, char **argv);
};
