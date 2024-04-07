#pragma once

#include <array>
#include <vector>
#include <string>
#include <memory>

#include "Library/Color/Color.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Point.h"

struct GUICharMetric {
    int32_t uLeftSpacing;
    int32_t uWidth;
    int32_t uRightSpacing;
};

struct FontHeader {
    uint8_t cFirstChar = 0;
    uint8_t cLastChar = 0;
    uint16_t uFontHeight = 0;
    std::array<GUICharMetric, 256> pMetrics = {{}};
    std::array<uint32_t, 256> font_pixels_offset = {{}};
};

struct FontData {
    FontHeader header;
    std::vector<uint8_t> pixels;
};

class GUIWindow;
class GraphicsImage;

class GUIFont {
 public:
    GUIFont();
    ~GUIFont();

    static std::unique_ptr<GUIFont> LoadFont(std::string_view pFontFile, std::string_view pFontPalette);

    void CreateFontTex();
    void ReleaseFontTex();

    bool IsCharValid(unsigned char c) const;
    int GetHeight() const;

    int AlignText_Center(int width, std::string_view str);

    int GetLineWidth(std::string_view str);

    int CalcTextHeight(std::string_view str, int width, int x_offset, bool return_on_carriage = false);

    std::string GetPageTop(std::string_view pInString, GUIWindow *pWindow,
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
    Color DrawTextLine(std::string_view text, Color color, Color defaultColor, Pointi position, int max_len_pix);
    void DrawText(GUIWindow *window, Pointi position, Color color, std::string_view text, int maxHeight, Color shadowColor);
    int DrawTextInRect(GUIWindow *window, Pointi position,
                       Color color, std::string_view text, int rect_width,
                       int reverse_text);

    std::string FitTextInAWindow(std::string_view inString, int width, int uX, bool return_on_carriage = false);

    // TODO: these should take std::string_view
    void DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY,
                          unsigned int w, unsigned int h, Color firstColor,
                          Color secondColor, std::string_view pString,
                          GraphicsImage *image);
    int GetStringHeight2(GUIFont *secondFont, std::string_view text_str,
                         GUIWindow *pWindow, int startX, int a6);

    int maxcharwidth = 0;
    GraphicsImage *fonttex = nullptr;
    GraphicsImage *fontshadow = nullptr;

 private:
    std::string FitTwoFontStringINWindow(std::string_view inString, GUIFont *pFontSecond,
                                    GUIWindow *pWindow, int startPixlOff,
                                    bool return_on_carriage = false);
    void DrawTextLineToBuff(Color color, Color *uX_buff_pos,
                            std::string_view text, int line_width);

 private:
    FontData pData;
    Palette palette;
};

void ReloadFonts();
