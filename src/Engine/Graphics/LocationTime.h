#pragma once

#include <string>

#include "Engine/Time.h"

struct LocationTime {
    GameTime last_visit;
    std::string sky_texture_name;
    int day_attrib = 0; // TODO(captainurist): actually WeatherFlags, see DAY_ATTRIB_FOG.
    int day_fogrange_1 = 0;
    int day_fogrange_2 = 0;
};
