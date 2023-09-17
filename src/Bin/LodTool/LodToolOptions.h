#pragma once

#include <string>

struct LodToolOptions {
    enum class Subcommand {
        SUBCOMMAND_DUMP,
    };
    using enum Subcommand;

    Subcommand subcommand = SUBCOMMAND_DUMP;
    std::string lodPath;
    bool helpPrinted = false; // True means that help message was already printed.

    static LodToolOptions parse(int argc, char **argv);
};
