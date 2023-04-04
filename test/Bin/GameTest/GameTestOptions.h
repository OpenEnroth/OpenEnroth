#pragma once

#include <string>

class Platform;

struct GameTestOptions {
    std::string testPath;
    std::string dataPath;
    bool helpPrinted = false;

    static GameTestOptions Parse(int argc, char **argv);

    void ResolveDefaults(Platform *platform);
};
