#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Engine/Events/EventEnums.h"

struct MapEventVariables {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};

extern MapEventVariables mapEventVariables;
