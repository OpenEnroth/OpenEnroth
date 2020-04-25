#pragma once
#include "Engine/Strings.h"

class GUIWindow;
class Image;
struct FontData;
struct FontData_MM7;

class GUIFont {
 public:
    static GUIFont *LoadFont(const char *pFontFile, const char *pFontPalette);

    bool IsCharValid(unsigned char c) const;
    unsigned int GetHeight() const;

    unsigned int AlignText_Center(unsigned int width, const String &str);

    unsigned int GetLineWidth(const String &str);

    unsigned int CalcTextHeight(const String &str, unsigned int width,
                                int x_offset, bool return_on_carriage = false);

    String GetPageTop(const String &pInString, GUIWindow *pWindow,
                      unsigned int uX, int a5);
    void DrawTextLine(const String &text, uint16_t uDefaultColor, int uX,
                      int uY, int max_len_pix);
    void DrawText(GUIWindow *pWindow, int uX, int uY, uint16_t uFontColor,
                  const String &str, bool present_time_transparency,
                  int max_text_height, int uFontShadowColor);
    int DrawTextInRect(GUIWindow *pWindow, unsigned int uX, unsigned int uY,
                       uint16_t uColor, const String &str, int rect_width,
                       int reverse_text);

    String FitTextInAWindow(const String &inString, unsigned int width, int uX,
                            bool return_on_carriage = false);

    void DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY,
                          unsigned int w, unsigned int h, uint16_t firstColor,
                          uint16_t secondColor, const String &pString,
                          Image *image);
    int GetStringHeight2(GUIFont *secondFont, const String &text_str,
                         GUIWindow *pWindow, int startX, int a6);

 protected:
    FontData *pData;

    String FitTwoFontStringINWindow(const String &pString, GUIFont *pFontSecond,
                                    GUIWindow *pWindow, int startPixlOff,
                                    int a6);
    void DrawTextLineToBuff(uint16_t uColor, uint32_t *uX_buff_pos,
                            const String &text, int line_width);
};

extern GUIFont *pAutonoteFont;
extern GUIFont *pSpellFont;
extern GUIFont *pFontArrus;
extern GUIFont *pFontLucida;
extern GUIFont *pBook2Font;
extern GUIFont *pBookFont;
extern GUIFont *pFontCreate;
extern GUIFont *pFontComic;
extern GUIFont *pFontSmallnum;
