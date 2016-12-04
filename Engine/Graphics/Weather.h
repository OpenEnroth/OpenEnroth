#pragma once


#pragma pack(push, 1)



/*  255 */
struct Weather
{
  inline Weather():
    bNight(false), bRenderSnow(false)
  {}

  int DrawSnow();
  int Initialize();
  int Draw();
  bool OnPlayerTurn(__int16 dangle);



  __int16 Screen_Coord[2000];
  int bNight;
  int bRenderSnow;
};
#pragma pack(pop)



extern struct Weather *pWeather;