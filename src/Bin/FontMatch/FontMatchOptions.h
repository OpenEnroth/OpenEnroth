#pragma once

#include <string>

struct FontMatchOptions {
    std::string lodPath; // Path to icons.lod to read the reference font from.
    std::string fontName; // Name of the font entry inside the lod, e.g. "autonote.fnt".
    std::string listPath; // Path to a file listing candidate font paths, one per line.
    std::string characters; // Characters to match on.
    int top = 8; // Number of best matches to print.
    bool helpPrinted = false; // True means that help message was already printed.

    static FontMatchOptions parse(int argc, char **argv);
};
