#pragma once

#include <string>

struct FontGenOptions {
    std::string enLodPath; // Path to icons.lod from an English localization.
    std::string ruLodPath; // Path to icons.lod from a Russian localization.
    std::string outputPath; // Directory to write the generated fonts into.
    bool helpPrinted = false; // True means that help message was already printed.

    static FontGenOptions parse(int argc, char **argv);
};
