#pragma once

#include <vector>

#include "Engine/Strings.h"

#pragma pack(push, 1)
struct GUICharMetric {
    uint32_t uLeftSpacing;
    uint32_t uWidth;
    uint32_t uRightSpacing;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct FontData {
    uint8_t cFirstChar;  // 0
    uint8_t cLastChar;   // 1
    uint8_t field_2;
    uint8_t field_3;
    uint8_t field_4;
    uint16_t uFontHeight;  // 5-6
    uint8_t field_7;
    uint32_t palletes_count;
    uint8_t *pFontPalettes[5];
    GUICharMetric pMetrics[256];
    uint32_t font_pixels_offset[256];
    std::vector<uint8_t> pFontData;  // array of font pixels
//    uint8_t pFontData[0];  // array of font pixels
};
#pragma pack(pop)

class GUIWindow;
class Image;
struct FontData;

class GUIFont {
 public:
    GUIFont () : pData(new FontData()) {}
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
