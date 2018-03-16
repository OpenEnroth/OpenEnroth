#pragma once
#include "Engine/Strings.h"

/*  171 */
#pragma pack(push, 1)
struct GUICharMetric
{
  unsigned int uLeftSpacing;
  unsigned int uWidth;
  unsigned int uRightSpacing;
};
#pragma pack(pop)

/*  170 */
#pragma pack(push, 1)
struct GUIFont
{
    //----- (0044C4DE) --------------------------------------------------------
    bool IsCharValid(unsigned char c) { return (c >= cFirstChar) && (c <= cLastChar) || (c == '\f') || (c == '\r') || (c == '\t') || (c == '\n'); }

    int AlignText_Center(unsigned int center_x, const char *pString);
    int AlignText_Center(unsigned int center_x, const String &str);

    int GetLineWidth(const char *pString);
    int GetLineWidth(const String &str);

    int CalcTextHeight(const char *str, struct GUIWindow *window, int x_offset, bool return_on_carriage = false);
    int CalcTextHeight(const String &str, struct GUIWindow *window, int x_offset, bool return_on_carriage = false);

    char* GetPageTop(const char *pInString, GUIWindow *pWindow, unsigned int uX, int a5);
    void DrawTextLineToBuff(int uColor, int a3, unsigned short* uX_buff_pos, const char *text, int line_width);
    void DrawTextLine(unsigned int uDefaultColor, signed int uX, signed int uY, const char *text, int max_len_pix);

    static void uGameUIFontMain_initialize();
    static void uGameUIFontShadow_initialize();

    GUICharMetric GetCharMetric(unsigned char c) { return pMetrics[c]; }
    unsigned int GetFontHeight() const { return (unsigned char)this->uFontHeight; }

    unsigned char cFirstChar;  //0
    unsigned char cLastChar;  //1
    char field_2;
    char field_3;
    char field_4;
    __int16 uFontHeight;  //5-6
    char field_7;
    int palletes_count;
    unsigned __int16 *pFontPalettes[5];
    GUICharMetric pMetrics[256];
    int font_pixels_offset[256];
    unsigned char pFontData[0]; //array of font pixels
};
#pragma pack(pop)

GUIFont *LoadFont(const char *pFontFile, const char *pFontPalette, ...);
char *FitTextInAWindow(const char *pInString, GUIFont *pFont, struct GUIWindow *pWindow, int uX, bool return_on_carriage = false);
String FitTextInAWindow(String &str, GUIFont *pFont, struct GUIWindow *pWindow, int uX, bool return_on_carriage = false);


extern struct GUIFont *pAutonoteFont;
extern struct GUIFont *pSpellFont;
extern struct GUIFont *pFontArrus;
extern struct GUIFont *pFontLucida;
extern struct GUIFont *pBook2Font;
extern struct GUIFont *pBookFont;
extern struct GUIFont *pFontCreate;
extern struct GUIFont *pFontComic;
extern struct GUIFont *pFontSmallnum;