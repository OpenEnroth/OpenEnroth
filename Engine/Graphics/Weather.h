#pragma once

#include "Engine/Point.h"

#include <array>

class Weather {
 public:
  Weather() : bNight(false), bRenderSnow(false) {}

  void DrawSnow();
  void Initialize();
  void Draw();
  bool OnPlayerTurn(int dangle);

  std::array<Point, 1000> Screen_Coord;
  bool bNight;
  bool bRenderSnow;
};

extern Weather *pWeather;
