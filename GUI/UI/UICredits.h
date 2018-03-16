#pragma once

#include <cstdint>
#include "GUI/GUIWindow.h"

class GUIFont;
class GUIWindow;

class GUICredits : public GUIWindow {
 public:
  static bool ExecuteCredits();

  static void DrawCreditsEntry(GUIFont *firstFont, GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h,
    uint16_t firstColor, uint16_t secondColor, const char *pString,
    uint16_t *pPixels, unsigned int uPixelsWidth);
  static char *FitTwoFontStringINWindow(const char *pString, GUIFont *pFontMain, GUIFont *pFontSecond, GUIWindow* pWindow, int startPixlOff, int a6);
  static int GetStringHeight2(GUIFont *firstFont, GUIFont *secondFont, const char *text_str, GUIWindow* pWindow, int startX, int a6);
};
