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
    };

    struct ExtractOptions {
        std::string output;
    };

    Subcommand subcommand = SUBCOMMAND_DUMP;
    std::string lodPath;
    bool helpPrinted = false; // True means that help message was already printed.
    CatOptions cat;
    ExtractOptions extract;
    bool raw = false; // Raw flag, shared by cat & extract.

    static LodToolOptions parse(int argc, char **argv);
};
