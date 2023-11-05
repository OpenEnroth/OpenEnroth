#pragma once

#include <array>

#include "Library/Geometry/Point.h"

class Weather {
 public:
    Weather() : bNight(false), bRenderSnow(false) {}

    void DrawSnow();
    void Initialize();
    void Draw();
    bool OnPlayerTurn(int dangle);

    std::array<Pointi, 1000> Screen_Coord;
    bool bNight;
    bool bRenderSnow;
};

extern Weather *pWeather;
