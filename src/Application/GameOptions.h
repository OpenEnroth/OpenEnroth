#pragma once

#include <string>

#include "GameStarterOptions.h"

class GameConfig;
class Platform;

struct GameOptions : public GameStarterOptions {
    bool helpPrinted = false; // True means that help message was already printed.

    /**
     * Parses OpenEnroth command line options.
     *
     * @param argc                      argc as passed to main.
     * @param argv                      argv as passed to main.
     * @throw std::exception            On errors.
     */
    static GameOptions Parse(int argc, char **argv);
};
