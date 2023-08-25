#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include "Library/Color/Color.h"
#include "Library/Image/Palette.h"
#include "Utility/Geometry/Point.h"

struct Palette;

struct GUICharMetric {
    int32_t uLeftSpacing;
    int32_t uWidth;
    int32_t uRightSpacing;
};

struct FontData {
    uint8_t cFirstChar = 0;  // 0
    uint8_t cLastChar = 0;   // 1
    uint16_t uFontHeight = 0;  // 5-6
    uint32_t palletes_count = 0;
    std::array<Palette, 5> pFontPalettes = {{}};
    std::array<GUICharMetric, 256> pMetrics = {{}};
    std::array<uint32_t, 256> font_pixels_offset = {{}};
    std::vector<uint8_t> pFontData;  // array of font pixels
//    uint8_t pFontData[0];  // array of font pixels
};

class GUIWindow;
class GraphicsImage;
struct FontData;

class GUIFont {
 public:
    GUIFont () : pData(new FontData()) {}
    ~GUIFont() {
        ReleaseFontTex();
        delete pData;
    }

    static std::unique_ptr<GUIFont> LoadFont(const char *pFontFile, const char *pFontPalette);

    void CreateFontTex();
    void ReleaseFontTex();

    bool IsCharValid(unsigned char c) const;
    int GetHeight() const;

    int AlignText_Center(int width, const std::string &str);

    int GetLineWidth(const std::string &str);

    int CalcTextHeight(const std::string &str, int width, int x_offset, bool return_on_carriage = false);

    std::string GetPageTop(const std::string &pInString, GUIWindow *pWindow,
                      unsigned int uX, int a5);

    /**
     * Draws a single line of text.
     *
     * @param text                          Input line of text.
     * @param color                         Color that the text should be started to be drawn at - this allows feeding
     *                                      in the color returned from the previous call to maintain correct color when
     *                                      its split onto a new line.
     * @param defaultColor                  The color that the text should return to on hitting a default color tag.
     * @param position                      Position to draw the text line to.
     * @param max_len_pix                   The maximum allowed width for this line of text.
     * 
     * @return                              Color that was used to draw text at the end of the line.
     */
    Color DrawTextLine(const std::string &text, Color color, Color defaultColor, Pointi position, int max_len_pix);
    void DrawText(GUIWindow *window, Pointi position, Color color, const std::string &text, int maxHeight, Color shadowColor);
    int DrawTextInRect(GUIWindow *window, Pointi position,
                       Color color, const std::string &text, int rect_width,
                       int reverse_text);

    std::string FitTextInAWindow(const std::string &inString, unsigned int width, int uX,
                            bool return_on_carriage = false);

    // TODO: these should take std::string_view
    void DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY,
                          unsigned int w, unsigned int h, Color firstColor,
                          Color secondColor, const std::string &pString,
                          GraphicsImage *image);
    int GetStringHeight2(GUIFont *secondFont, const std::string &text_str,
                         GUIWindow *pWindow, int startX, int a6);

    int maxcharwidth = 0;
    GraphicsImage *fonttex = nullptr;
    GraphicsImage *fontshadow = nullptr;

 protected:
    FontData *pData;

    std::string FitTwoFontStringINWindow(const std::string &inString, GUIFont *pFontSecond,
                                    GUIWindow *pWindow, int startPixlOff,
                                    bool return_on_carriage = false);
    void DrawTextLineToBuff(Color color, Color *uX_buff_pos,
                            const std::string &text, int line_width);
};

void ReloadFonts();
