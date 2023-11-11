#pragma once

#include <array>

#include "Library/Geometry/Point.h"

class Weather {
 public:
    void DrawSnow();
    void Initialize();
    void Draw();
    bool OnPlayerTurn(int dangle);

    std::array<Pointi, 1000> Screen_Coord;
    bool bNight = false;
    bool bRenderSnow = false;
};

extern Weather *pWeather;
