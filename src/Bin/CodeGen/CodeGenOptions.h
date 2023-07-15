#pragma once

#include "Application/GameStarterOptions.h"

struct CodeGenOptions : GameStarterOptions {
    enum class Subcommand {
        SUBCOMMAND_NONE,
        SUBCOMMAND_ITEM_ID,
        SUBCOMMAND_MAP_ID
    };
    using enum Subcommand;

    Subcommand subcommand = SUBCOMMAND_NONE;
    bool helpPrinted = false; // True means that help message was already printed.

    static CodeGenOptions Parse(int argc, char **argv);
};
