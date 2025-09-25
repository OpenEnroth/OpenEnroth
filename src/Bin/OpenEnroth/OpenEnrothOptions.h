#pragma once

#include <string>
#include <vector>

#include "Application/Startup/GameStarterOptions.h"

class GameConfig;
class Platform;

struct OpenEnrothOptions : public GameStarterOptions {
    enum class Subcommand {
        SUBCOMMAND_GAME,
        SUBCOMMAND_PLAY,
        SUBCOMMAND_RETRACE
    };
    using enum Subcommand;

    enum class Migration {
        MIGRATION_NONE,
        MIGRATION_DROP_REDUNDANT_KEY_EVENTS,
        MIGRATION_COLLAPSE_KEY_EVENTS,
    };
    using enum Migration;

    struct RetraceOptions {
        std::vector<std::string> traces;
        Migration migration = MIGRATION_NONE;
        bool checkCanonical = false;
    };

    struct PlayOptions {
        std::vector<std::string> traces;
        float speed = 1.0f;
    };

    Subcommand subcommand = SUBCOMMAND_GAME;
    bool helpPrinted = false; // True means that help message was already printed.
    RetraceOptions retrace;
    PlayOptions play;

    /**
     * Parses OpenEnroth command line options.
     *
     * @param argc                      argc as passed to main.
     * @param argv                      argv as passed to main.
     * @throw std::exception            On errors.
     */
    static OpenEnrothOptions parse(int argc, char **argv);
};

MM_DECLARE_SERIALIZATION_FUNCTIONS(OpenEnrothOptions::Migration)
