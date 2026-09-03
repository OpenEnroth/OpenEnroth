#pragma once

#include <string>

#include "Utility/System/Path.h"

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
        Path output;
    };

    Subcommand subcommand = SUBCOMMAND_DUMP;
    Path path;
    bool helpPrinted = false; // True means that help message was already printed.
    CatOptions cat;
    ExtractOptions extract;
    bool raw = false; // Raw flag, shared by cat & extract.
    Path palettesLodPath; // Path to bitmaps.lod for sprite palettes.

    static LodToolOptions parse(int argc, char **argv);
};
