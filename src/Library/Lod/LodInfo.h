#pragma once

#include <string>

#include "LodEnums.h"

struct LodInfo {
    LodVersion version = LOD_VERSION_MM6;
    std::string description;
    std::string rootName;
};
