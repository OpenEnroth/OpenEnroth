#pragma once

#include <string>

#include "AutonoteEnums.h"

struct AutonoteData {
    std::string pText;
    AutonoteType eType = AUTONOTE_POTION_RECIPE;
};
