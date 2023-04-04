#pragma once

#include <string>

class GameConfig;
class Platform;

struct GameOptions {
    std::string configPath;
    std::string dataPath;
    bool verbose = false;
    bool helpPrinted = false; // True means that help message was already printed.

    /**
     * Parses OpenEnroth command line options.
     *
     * @param argc                      argc as passed to main.
     * @param argv                      argv as passed to main.
     * @throw std::exception            On errors.
     */
    static GameOptions Parse(int argc, char **argv);

    /**
     * Resolves default values after `Platform` was constructed.
     *
     * @param platform                  Platform object.
     */
    void ResolveDefaults(Platform *platform);
};
