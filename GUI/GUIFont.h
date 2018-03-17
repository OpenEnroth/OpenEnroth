#pragma once
#include "Engine/Strings.h"

class GUIWindow;
struct FontData;

class GUIFont {
 public:
  static GUIFont *LoadFont(const char *pFontFile, const char *pFontPalette, ...);

  bool IsCharValid(unsigned char c) const;
  unsigned int GetHeight() const;

  unsigned int AlignText_Center(unsigned int width, const String &str);

  unsigned int GetLineWidth(const String &str);

  unsigned int CalcTextHeight(const String &str, struct GUIWindow *window, int x_offset, bool return_on_carriage = false);

  String GetPageTop(const String &pInString, GUIWindow *pWindow, unsigned int uX, int a5);
  void DrawTextLineToBuff(int uColor, unsigned short* uX_buff_pos, const char *text, int line_width);
  void DrawTextLine(const String &text, uint16_t uDefaultColor, int uX, int uY, int max_len_pix);
  void DrawText(GUIWindow *pWindow, int uX, int uY, unsigned short uFontColor, const char *Str, bool present_time_transparency, int max_text_height, int uFontShadowColor);
  int DrawTextInRect(GUIWindow *pWindow, unsigned int uX, unsigned int uY, unsigned int uColor, String &str, int rect_width, int reverse_text);

  String FitTextInAWindow(const String &inString, GUIWindow *pWindow, int uX, bool return_on_carriage = false);

  static void _44D2FD_prolly_draw_credits_entry(GUIFont *firstFont, GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h, uint16_t firstColor, uint16_t secondColor, const char *pString, uint16_t *pPixels, unsigned int uPixelsWidth);
  static char *FitTwoFontStringINWindow(const char *pString, GUIFont *pFontMain, GUIFont *pFontSecond, GUIWindow* pWindow, int startPixlOff, int a6);
  static int GetStringHeight2(GUIFont *firstFont, GUIFont *secondFont, const char *text_str, GUIWindow* pWindow, int startX, int a6);

 protected:
  FontData *pData;
};

extern struct GUIFont *pAutonoteFont;
extern struct GUIFont *pSpellFont;
extern struct GUIFont *pFontArrus;
extern struct GUIFont *pFontLucida;
extern struct GUIFont *pBook2Font;
extern struct GUIFont *pBookFont;
extern struct GUIFont *pFontCreate;
extern struct GUIFont *pFontComic;
extern struct GUIFont *pFontSmallnum;
