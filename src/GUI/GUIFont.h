#pragma once

#include <array>
#include <vector>
#include <string>
#include <memory>

#include "Library/Color/Color.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Point.h"
#include "Library/LodFormats/LodFont.h"

class GUIWindow;
class GraphicsImage;

/**
 * Some notes on markup characters supported by `GUIFont` functions.
 *
 * - `\n` is a regular line feed.
 * - `\tXXX`, where `XXX` is decimal offset in pixels. Moves the caret to a position offset by `XXX` to the right from
 *   the start of the line. This is how aligned tables are implemented.
 * - `\rXXX`, where `XXX` is a decimal offset in pixels. Moves the caret so that the rest of the line is
 *   right-justified, with the given offset from the right window border. This is used e.g. in the create party screen.
 * - `\fXXXXX`, where `XXXXX` is a decimal color code for 16-bit color to use for the text that follows.
 *
 * @see Color::fromC16
 */
class GUIFont {
 public:
    GUIFont();
    ~GUIFont();

    static std::unique_ptr<GUIFont> LoadFont(std::string_view pFontFile);

    void CreateFontTex();
    void ReleaseFontTex();

    int GetHeight() const;

    /**
     * @param width                     Width of the rect to position the text in.
     * @param str                       Text to position.
     * @return                          X-offset for a call to `DrawTextLine` to make the text aligned, or 0 if the
     *                                  text overflows.
     */
    int AlignText_Center(int width, std::string_view str);

    /**
     * @param str                       Text to check.
     * @return                          Width of the first line of `str`, in pixels. Note that `\t` counts as a line
     *                                  break as it essentially starts a new table column.
     */
    int GetLineWidth(std::string_view str);

    /**
     * @param str                       Text to check.
     * @param width                     Width of the window that the text should fit into.
     * @param x                         Where the text starts relative to the window's left border.
     * @return                          Wrapped text height, in pixels.
     */
    int CalcTextHeight(std::string_view str, int width, int x);

    std::string GetPageTop(std::string_view str, GUIWindow *window, int x, int page);

    /**
     * Draws a single line of text.
     *
     * @param text                          Input line of text.
     * @param color                         Color that the text should be started to be drawn at - this allows feeding
     *                                      in the color returned from the previous call to maintain correct color when
     *                                      it's split onto a new line.
     * @param defaultColor                  The color that the text should return to on hitting a default color tag.
     * @param position                      Position to draw the text line to.
     * @return                              Color that was used to draw text at the end of the line.
     */
    Color DrawTextLine(std::string_view text, Color color, Color defaultColor, Pointi position);
    void DrawText(GUIWindow *window, Pointi position, Color color, std::string_view text, int maxHeight, Color shadowColor);
    int DrawTextInRect(GUIWindow *window, Pointi position,
                       Color color, std::string_view text, int rect_width,
                       int reverse_text);

    std::string WrapText(std::string_view inString, int width, int uX, bool return_on_carriage = false);

    // TODO: these should take std::string_view
    void DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY,
                          unsigned int w, unsigned int h, Color firstColor,
                          Color secondColor, Color shadowColor, std::string_view pString,
                          GraphicsImage *image);
    int GetStringHeight2(GUIFont *secondFont, std::string_view text_str,
                         GUIWindow *pWindow, int startX, int a6);

 private:
    bool IsCharValid(unsigned char c) const;
    std::string FitTwoFontStringINWindow(std::string_view inString, GUIFont *pFontSecond,
                                    GUIWindow *pWindow, int startPixlOff,
                                    bool return_on_carriage = false);
    void DrawTextLineToBuff(Color color, Color shadowColor, Color *uX_buff_pos,
                            std::string_view text, int line_width);

 private:
    LodFont _font;
    GraphicsImage *_mainTexture = nullptr;
    GraphicsImage *_shadowTexture = nullptr;
};

