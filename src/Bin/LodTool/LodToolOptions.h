#pragma once

#include <string>

struct LodToolOptions {
    enum class Subcommand {
        SUBCOMMAND_LS,
        SUBCOMMAND_DUMP,
        SUBCOMMAND_CAT,
    };
    using enum Subcommand;

    struct CatOptions {
        std::string entry;
        bool raw = false;
    };

    Subcommand subcommand = SUBCOMMAND_DUMP;
    std::string lodPath;
    bool helpPrinted = false; // True means that help message was already printed.
    CatOptions cat;

    static LodToolOptions parse(int argc, char **argv);
};
