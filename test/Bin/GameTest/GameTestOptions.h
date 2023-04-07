#pragma once

#include <string>

#include "Application/GameStarterOptions.h"

class Platform;

struct GameTestOptions : public GameStarterOptions {
    std::string testPath;
    bool helpPrinted = false;

    static GameTestOptions Parse(int argc, char **argv);
};
