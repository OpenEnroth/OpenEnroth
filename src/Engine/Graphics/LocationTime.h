#pragma once

#include <string>

#include "Engine/Time.h"
#include "Engine/MapEnums.h"

// TODO(captainurist): rename to smth like MapTime.
struct LocationTime {
    GameTime last_visit;
    std::string sky_texture_name;
    MapWeatherFlags day_attrib = 0;
    int day_fogrange_1 = 0;
    int day_fogrange_2 = 0;
};
