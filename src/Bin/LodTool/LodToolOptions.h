#pragma once

#include <string>

struct LodToolOptions {
    enum class Subcommand {
        SUBCOMMAND_LS,
        SUBCOMMAND_DUMP,
        SUBCOMMAND_CAT,
        SUBCOMMAND_EXTRACT,
    };
    using enum Subcommand;

    struct CatOptions {
        std::string entry;
        bool raw = false;
    };

    struct ExtractOptions {
        std::string output;
        bool raw = false;
    };

    Subcommand subcommand = SUBCOMMAND_DUMP;
    std::string lodPath;
    bool helpPrinted = false; // True means that help message was already printed.
    CatOptions cat;
    ExtractOptions extract;

    static LodToolOptions parse(int argc, char **argv);
};
