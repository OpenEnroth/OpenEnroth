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
        MIGRATION_DROP_AUTOREPEAT, // Drops all autorepeat events - they are ignored by the engine anyway.
        MIGRATION_DROP_ORPHANED_KEY_RELEASES, // Drops key release events w/o a corresponding key press.
        MIGRATION_COLLAPSE_KEY_EVENTS, // Collapses key press & release events inside a single frame for continuously
                                            // toggleable input actions.
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
