#pragma once

#include <string>

#include "Engine/Time/Time.h"
#include "Engine/MapEnums.h"

// TODO(captainurist): rename to smth like MapTime.
struct LocationTime {
    Time lastVisitTime;
    std::string skyTextureName;
    MapWeatherFlags weatherFlags = 0;
    int fogWeakDistance = 0;
    int fogStrongDistance = 0;
};
