#pragma once

#include <string>

#include "Engine/MapEnums.h"

/**
 * Weather state of an outdoor map, rolled on visit and persisted in the save.
 */
struct MapWeather {
    std::string skyTextureName;
    MapWeatherFlags flags = 0;
    int fogWeakDistance = 0;
    int fogStrongDistance = 0;
};
