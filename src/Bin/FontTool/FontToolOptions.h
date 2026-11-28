#pragma once

#include <string>
#include <vector>

#include "Utility/String/Encoding.h"

#include "Render.h"
#include "Shadow.h"

struct FontToolOptions {
    enum class Subcommand {
        SUBCOMMAND_CONVERT,
        SUBCOMMAND_MERGE,
        SUBCOMMAND_RENDER,
        SUBCOMMAND_DIFF,
        SUBCOMMAND_SEARCH,
        SUBCOMMAND_GENERATE,
    };
    using enum Subcommand;

    Subcommand subcommand = SUBCOMMAND_CONVERT;
    bool helpPrinted = false; // True means that help message was already printed.

    TextEncoding encoding = ENCODING_WINDOWS_1252; // Encoding of LOD font inputs.
    std::string input;
    std::string secondInput; // Second font for diff.
    std::vector<std::string> inputs; // Fonts to merge, or candidate fonts and directories for search.
    std::string output;
    std::string text; // UTF-8, empty means the default for the subcommand.

    int faceIndex = 0;
    RenderSettings render;
    bool gray = false;
    ShadowMode shadow = SHADOW_NONE;

    int scale = 4;

    std::vector<std::string> references;
    std::string listPath; // File listing candidate fonts, one per line.
    int top = 20;
    int ppemWindow = 2;
    bool exhaustive = false;
    bool oblique = false;
    int threads = 0;

    std::string enIconsLodPath;
    std::string ruIconsLodPath;

    static FontToolOptions parse(int argc, char **argv);
};
