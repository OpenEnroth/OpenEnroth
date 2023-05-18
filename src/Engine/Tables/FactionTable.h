#pragma once

#include <string>

struct FactionTable {
    void Initialize();

    char relations[89][89];
};

extern std::string pHostileTXT_Raw;
