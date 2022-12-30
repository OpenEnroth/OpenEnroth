#pragma once

#include <string>

struct GameTestOptions {
    std::string testDataDir;
    std::string gameDataDir;

    bool Parse(int argc, char **argv);
};
