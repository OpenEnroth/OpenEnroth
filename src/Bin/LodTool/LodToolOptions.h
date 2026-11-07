#pragma once

#include "Utility/System/Path.h"

struct LodToolOptions {
    enum class Subcommand {
        SUBCOMMAND_LS,
        SUBCOMMAND_CAT,
        SUBCOMMAND_EXTRACT,
        SUBCOMMAND_CONVERT,
        SUBCOMMAND_MAGIC,
    };
    using enum Subcommand;

    Subcommand subcommand = SUBCOMMAND_LS;
    Path path; // Input path, can point into an archive, as in "games.lod/d.odm".
    Path output; // Output path for extract & convert.
    bool helpPrinted = false; // True means that help message was already printed.
    bool raw = false; // Raw flag, shared by cat & extract.
    Path palettesLodPath; // Path to bitmaps.lod for sprite palettes.

    static LodToolOptions parse(int argc, char **argv);
};
