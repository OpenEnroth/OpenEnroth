#include "GUIFont.h"

#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Resources/ResourceManager.h"

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Font/Oef.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Format.h"

static constexpr char32_t REPLACEMENT_CHARACTER = 0xFFFD; // First fallback for an unsupported character, if present.

// Reads a fixed-width decimal number embedded in the markup, e.g. the offset in a `\tXXX` tag, or the color code in
// a `\fXXXXX` tag. The tag character is already consumed, `*pos` is at the first digit.
static int parseMarkupNumber(std::string_view s, size_t *pos, int digitCount) {
    // Markup numbers are always ASCII digits, so reading them code point by code point works.
    char digits[6] = {};
    assert(digitCount > 0 && static_cast<size_t>(digitCount) < sizeof(digits));
    for (int i = 0; i < digitCount && *pos < s.size(); i++)
        digits[i] = static_cast<char>(txt::nextRune(s, pos));
    return std::strtol(digits, nullptr, 10);
}

static Color parseColorTag(std::string_view s, size_t *pos, const Color &defaultColor) {
    int color16 = parseMarkupNumber(s, pos, 5);
    return color16 == 0 ? defaultColor : Color::fromC16(color16); // Zero color code means back to default color.
}

GUIFont::GUIFont() = default;

GUIFont::~GUIFont() {
    ReleaseFontTex();
}

std::unique_ptr<GUIFont> GUIFont::LoadFont(std::string_view pFontFile) {
    std::unique_ptr<GUIFont> result = std::make_unique<GUIFont>();
    result->_font = oef::decode(dfs->read(fmt::format("fonts/{}", pFontFile)));

    result->CreateFontTex();
    return result;
}

// TODO(pskelton): Save built atlas so it doesnt get recalcualted on reload?
void GUIFont::CreateFontTex() {
    assert(_font.size() > 0);

    ReleaseFontTex();

    // Atlas cells are (maxWidth+1) x (height+1) so that there is no color bleeding when rendering with blending.
    int maxWidth = 0;
    for (int i = 0; i < _font.size(); i++)
        maxWidth = std::max(maxWidth, _font.metrics(i).width);

    int columns = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(_font.size()))));
    int rows = (_font.size() + columns - 1) / columns;
    _layout = AtlasLayout({columns, rows}, {maxWidth + 1, _font.height() + 1});

    RgbaImage pixels = RgbaImage::solid(Color(), _layout.geometry().size());

    // Pack per-channel color weights: color index n goes into channel n-1. R is for text, G is for shadow,
    // B & A are reserved for multi-color fonts (MM3 fonts use 4 colors and we will import them eventually).
    for (int l = 0; l < _font.size(); l++) {
        Recti cell = _layout[l];
        GrayscaleImageView image = _font.image(l);

        for (int y = 0; y < image.height(); y++) {
            for (int x = 0; x < image.width(); x++) {
                switch (image[y][x]) {
                case 1: pixels[cell.y + y][cell.x + x] = Color(255, 0, 0, 0); break;
                case 2: pixels[cell.y + y][cell.x + x] = Color(0, 255, 0, 0); break;
                case 3: pixels[cell.y + y][cell.x + x] = Color(0, 0, 255, 0); break;
                case 4: pixels[cell.y + y][cell.x + x] = Color(0, 0, 0, 255); break;
                default: break;
                }
            }
        }
    }

    _texture = GraphicsImage::Create(std::move(pixels));
}

void GUIFont::ReleaseFontTex() {
    _texture.reset();
}

int GUIFont::GetHeight() const {
    return _font.height();
}

int GUIFont::AlignText_Center(int width, std::string_view str) {
    int position = (width - GetLineWidth(str)) / 2;
    return (position < 0) ? 0 : position;
}

int GUIFont::GetLineWidth(std::string_view str) {
    int resultWidth = 0;
    GetTextLenLimitedByWidth(str, INT_MAX, resultWidth);
    return resultWidth;
}

int GUIFont::GetTextLenLimitedByWidth(std::string_view str, int maxWidth, int& resultWidth) {
    resultWidth = 0;
    for (size_t pos = 0; pos < str.size();) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(str, &pos);
        switch (c) {
        case U'\n': // New line.
        case U'\t': // Move to next cell, offset from the left border.
        case U'\r': // Right-justify, offset from the right border.
            return charPos;
        case U'\f': // Color tag.
            parseMarkupNumber(str, &pos, 5);
            break;
        default: {
            int glyph = glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = _font.metrics(glyph);
            if (charPos > 0)
                resultWidth += metrics.leftSpacing;
            resultWidth += metrics.width;
            if (pos < str.size())
                resultWidth += metrics.rightSpacing;

            if (resultWidth > maxWidth)
                return charPos;
        }
        }
    }
    return str.length();
}


int GUIFont::CalcTextHeight(std::string_view str, int width, int x) {
    if (str.empty())
        return 0;

    int height = _font.height() - 6;
    std::string wrappedStr = WrapText(str, width, x);
    for (int i = 0, len = wrappedStr.length(); i < len; ++i) {
        switch (wrappedStr[i]) {
        case '\n': // New line.
            height += _font.height() - 3;
            break;
        case '\f': // Color tag.
            i += 5;
            break;
        case '\t': // Move to next cell, offset from the left border.
        case '\r': // Right-justify, offset from the right border.
            i += 3;
            break;
        default:
            break;
        }
    }

    return height;
}

std::string GUIFont::GetPageText(std::string_view str, Sizei pageSize, int x, int page) {
    if (str.empty())
        return {};

    int height = 0;
    std::string wrappedText = WrapText(str, pageSize.w, x);
    for (int i = 0, len = wrappedText.length(); i < len; ++i) {
        switch (wrappedText[i]) {
        case '\n': // New line.
            height += _font.height() - 3;
            if (height >= page * (pageSize.h - (_font.height() - 3)))
                return wrappedText.substr(i);
            break;
        case '\f': // Color tag.
            i += 5;
            break;
        case '\t': // Move to next cell, offset from the left border.
        case '\r': // Right-justify, offset from the right border.
            i += 3;
            break;
        default:
            break;
        }
        if (height >= page * pageSize.h)
            break;
    }
    return wrappedText;
}

int GUIFont::glyphIndex(char32_t c) const {
    // Fall back through the replacement character, '?', and space, so an unsupported character renders as an existing
    // glyph in the font's own style rather than as a synthesized box (which wouldn't fit e.g. italic fonts). If none
    // of those exist either, render nothing.
    for (char32_t candidate : {c, REPLACEMENT_CHARACTER, U'?', U' '}) {
        int result = _font.index(candidate);
        if (result != -1)
            return result;
    }
    return -1;
}

Color GUIFont::DrawTextLine(std::string_view text, Color startColor, Color defaultColor, Pointi position) {
    assert(startColor.a > 0);

    if (text.empty())
        return startColor;

    render->BeginTextNew(_texture);

    Color color = startColor;
    int x = position.x;
    for (size_t pos = 0; pos < text.size();) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(text, &pos);
        switch (c) {
        case U'\n': // New line.
            return color;
        case U'\f': // Color tag.
            color = parseColorTag(text, &pos, defaultColor);
            break;
        case U'\t': // Move to next cell, offset from the left border.
        case U'\r': // Right-justify, offset from the right border.
            break;
        default: {
            int glyph = glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = _font.metrics(glyph);
            if (charPos > 0)
                x += metrics.leftSpacing;

            Recti cell = _layout[glyph];
            Recti srcRect(cell.x, cell.y, metrics.width, _font.height());
            Recti dstRect(x, position.y, metrics.width, _font.height());

            render->DrawTextNew(srcRect, dstRect, {color, colorTable.Black});

            x += metrics.width;
            if (pos < text.size())
                x += metrics.rightSpacing;
        }
        }
    }
    return color;
}

void DrawCharToBuff(Color *draw_buff, GrayscaleImageView image, Color draw_color, Color shadowColor, int line_width) {
    assert(draw_color.a > 0);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            uint8_t char_pxl = image[y][x];
            if (char_pxl) {
                if (char_pxl == 2) {
                    *draw_buff = shadowColor;
                } else {
                    *draw_buff = draw_color;
                }
            }
            ++draw_buff;
        }
        draw_buff += line_width - image.width();
    }
}

void GUIFont::DrawTextLineToBuff(Color startColor, Color shadowColor, Color *uX_buff_pos, std::string_view text, int line_width) {
    assert(startColor.a > 0);

    if (text.empty()) {
        return;
    }

    Color color = startColor;
    Color *uX_pos = uX_buff_pos;
    for (size_t pos = 0; pos < text.size();) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(text, &pos);
        switch (c) {
        case U'\n': // New line.
            return;
        case U'\f': // Color tag.
            color = parseColorTag(text, &pos, startColor);
            break;
        case U'\t': // Move to next cell, offset from the left border.
        case U'_': // Use alternative font.
            break;
        default: {
            int glyph = glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = _font.metrics(glyph);
            if (charPos > 0)
                uX_pos += metrics.leftSpacing;

            DrawCharToBuff(uX_pos, _font.image(glyph), color, shadowColor, line_width);
            uX_pos += metrics.width;

            if (pos < text.size())
                uX_pos += metrics.rightSpacing;
        }
        }
    }
}

std::string GUIFont::WrapText(std::string_view inString, int width, int uX, bool return_on_carriage) {
    assert(uX < width);

    if (inString.empty()) {
        return {};
    }

    int lineWidth = uX;
    int newlinePos = -1;
    int lastCopyPos = 0;
    std::string out;

    for (size_t pos = 0; pos < inString.size();) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(inString, &pos);
        switch (c) {
        case U'\t': // Move to next cell, offset from the left border.
            lineWidth = parseMarkupNumber(inString, &pos, 3) + uX;
            break;
        case U'\n': // New line.
            lineWidth = uX;
            newlinePos = -1;
            out += inString.substr(lastCopyPos, charPos - lastCopyPos);
            out += "\n";
            lastCopyPos = pos;
            break;
        case U'\f': // Color tag.
            parseMarkupNumber(inString, &pos, 5);
            break;
        case U'\r': // Right-justify, offset from the right border.
            if (!return_on_carriage) {
                return std::string(inString); // TODO(captainurist): this return is very sus.
            }
            break;
        case U' ': {
            int glyph = glyphIndex(U' ');
            if (glyph != -1)
                lineWidth += _font.metrics(glyph).width;
            newlinePos = charPos;
            break;
        }
        default: {
            int glyph = glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = _font.metrics(glyph);
            if ((lineWidth + metrics.width + metrics.leftSpacing + metrics.rightSpacing) < width) {
                if (static_cast<int>(charPos) > newlinePos)
                    lineWidth += metrics.leftSpacing;
                lineWidth += metrics.width;
                if (pos < inString.size())
                    lineWidth += metrics.rightSpacing;
            } else {
                lineWidth = uX;
                if (newlinePos >= 0) {
                    out += inString.substr(lastCopyPos, newlinePos - lastCopyPos);
                    out += "\n";
                    lastCopyPos = newlinePos + 1;
                    pos = newlinePos + 1;
                } else {
                    out += inString.substr(lastCopyPos, charPos - lastCopyPos);
                    out += "\n";
                    lastCopyPos = charPos;
                    pos = charPos; // Reprocess this character on the new line.
                }
                newlinePos = -1;
            }
        }
        }
    }

    if (lastCopyPos < inString.length()) {
        out += inString.substr(lastCopyPos, inString.length() - lastCopyPos);
    }

    return out;
}

void GUIFont::DrawText(const Recti &rect, Pointi position, Color defaultColor, std::string_view text, int maxY, Color shadowColor) {
    assert(defaultColor.a > 0);

    int left_margin = 0;
    if (text.empty()) {
        return;
    }
    if (text == "null") {
        return;
    }

    render->BeginTextNew(_texture);

    if (!position.x) {
        position.x = 12;
    }

    std::string string_base = std::string(text);
    if (maxY == 0) {
        string_base = WrapText(text, rect.w, position.x);
    }

    int out_x = position.x + rect.x;
    int out_y = position.y + rect.y;

    if (maxY != 0 && out_y + _font.height() > maxY) {
        return;
    }

    Color draw_color = defaultColor;

    size_t len = text.length();
    for (size_t pos = 0; pos < string_base.size() && pos < len;) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(string_base, &pos);
        switch (c) {
        case U'\t': // Move to next cell, offset from the left border.
            left_margin = parseMarkupNumber(string_base, &pos, 3);
            out_x = position.x + rect.x + left_margin;
            break;
        case U'\n': // New line.
            position.y = position.y + _font.height() - 3;
            out_y = position.y + rect.y;
            out_x = position.x + rect.x + left_margin;
            if (maxY != 0) {
                if (_font.height() + out_y - 3 > maxY) {
                    return;
                }
            }
            break;
        case U'\f': // Color tag.
            draw_color = parseColorTag(string_base, &pos, defaultColor);
            break;
        case U'\r': // Right-justify, offset from the right border.
            left_margin = parseMarkupNumber(string_base, &pos, 3);
            // Measuring from `pos - 1` includes the last digit of the margin in the measured width, shifting the
            // text left by one digit glyph. This reproduces an off-by-one in the original engine that the game's
            // UI layouts are tuned around, e.g. the gap after '/' in the character screen stat lines.
            out_x = rect.x + rect.w - 1 - GetLineWidth(std::string_view(string_base).substr(pos - 1)) - left_margin;
            out_y = position.y + rect.y;
            if (maxY != 0) {
                if (_font.height() + out_y - 3 > maxY) {
                    return;
                }
            }
            break;
        default: {
            int glyph = glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = _font.metrics(glyph);
            if (charPos > 0) {
                out_x += metrics.leftSpacing;
            }

            Recti cell = _layout[glyph];
            Recti srcRect(cell.x, cell.y, metrics.width, _font.height());
            Recti dstRect(out_x, out_y, metrics.width, _font.height());

            render->DrawTextNew(srcRect, dstRect, {draw_color, shadowColor});

            out_x += metrics.width;
            if (pos < len) {
                out_x += metrics.rightSpacing;
            }
            break;
        }
        }
    }
    // render->EndTextNew();
}

int GUIFont::DrawTextInRect(const Recti &rect, Pointi position, Color color, std::string_view text, int rect_width, int reverse_text) {
    assert(color.a > 0);

    char buf[4096];
    assert(text.length() < sizeof(buf));
    strncpy(buf, text.data(), text.size());
    buf[text.size()] = '\0';

    size_t pNumLen = strlen(buf);
    if (pNumLen == 0) return 0;

    unsigned int pLineWidth = GetLineWidth(buf);
    if (pLineWidth < rect_width) {
        DrawText(rect, position, color, buf, 0, colorTable.Black);
        return pLineWidth;
    } else {
        int resultWidth = 0;
        int textLen = GetTextLenLimitedByWidth(text, rect_width, resultWidth);
        if (0 <= textLen && textLen < sizeof(buf)) {
            buf[textLen] = '\0';
            DrawText(rect, position, color, buf, 0, colorTable.Black);
            return rect_width;
        } else {
            assert(false);
            return 0;
        }
    }
}

void GUIFont::DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h,
                               Color firstColor, Color secondColor, Color shadowColor, std::string_view pString,
                               RgbaImage *image) {
    std::string work_string = FitTwoFontStringInWindow(pString, pSecondFont, w, 0);
    std::istringstream stream(work_string);
    std::getline(stream, work_string);

    Color *pPixels = image->pixels().data();
    Color *curr_pixel_pos = &pPixels[image->width() * uFrameY];
    if (!work_string.empty()) {
        int half_frameX = uFrameX >> 1;
        while (!stream.eof()) {
            GUIFont *currentFont = this;
            int start_str_pos = 0;
            Color currentColor = firstColor;
            if (work_string[0] == '_') {
                currentFont = pSecondFont;
                currentColor = secondColor;
                start_str_pos = 1;
            }
            int line_w = (int)(w - currentFont->GetLineWidth(&work_string[start_str_pos])) / 2;
            if (line_w < 0) {
                line_w = 0;
            }
            currentFont->DrawTextLineToBuff(currentColor, shadowColor, &curr_pixel_pos[line_w + half_frameX],
                work_string, image->width());
            curr_pixel_pos += image->width() * (currentFont->GetHeight() - 3);
            std::getline(stream, work_string);
            if (work_string.empty()) {
                break;
            }
        }
    }
}

std::string GUIFont::FitTwoFontStringInWindow(std::string_view inString, GUIFont *pFontSecond, int width, int x) {
    if (inString.empty()) {
        return "";
    }

    GUIFont *currentFont = this;
    GUIFont *newlineFont = this;
    int lineWidth = x;
    int newlinePos = -1;
    int lastCopyPos = 0;
    std::string out;

    for (size_t pos = 0; pos < inString.size();) {
        size_t charPos = pos;
        char32_t c = txt::nextRune(inString, &pos);
        switch (c) {
        case U'\t': // Move to next cell, offset from the left border.
            lineWidth = parseMarkupNumber(inString, &pos, 3) + x;
            break;
        case U'\n': // New line.
            lineWidth = x;
            newlinePos = -1;
            out += inString.substr(lastCopyPos, charPos - lastCopyPos);
            out += "\n";
            lastCopyPos = pos;
            currentFont = this;
            break;
        case U'\f': // Color tag.
            parseMarkupNumber(inString, &pos, 5);
            break;
        case U'\r': // Surprise! Here it's just a \r\n!
            break;
        case U' ': {
            int glyph = currentFont->glyphIndex(U' ');
            if (glyph != -1)
                lineWidth += currentFont->_font.metrics(glyph).width;
            newlinePos = charPos;
            newlineFont = currentFont;
            break;
        }
        case U'_': // Use alternative font.
            currentFont = pFontSecond;
            break;
        default: {
            int glyph = currentFont->glyphIndex(c);
            if (glyph == -1)
                break;

            const GlyphMetrics &metrics = currentFont->_font.metrics(glyph);
            if ((lineWidth + metrics.width + metrics.leftSpacing + metrics.rightSpacing) < width) {
                if (static_cast<int>(charPos) > newlinePos)
                    lineWidth += metrics.leftSpacing;
                lineWidth += metrics.width;
                if (pos < inString.size())
                    lineWidth += metrics.rightSpacing;
            } else {
                lineWidth = x;
                currentFont = newlineFont;
                if (newlinePos >= 0) {
                    out += inString.substr(lastCopyPos, newlinePos - lastCopyPos);
                    out += "\n";
                    lastCopyPos = newlinePos + 1;
                    pos = newlinePos + 1;
                } else {
                    out += inString.substr(lastCopyPos, charPos - lastCopyPos);
                    out += "\n";
                    lastCopyPos = charPos;
                    pos = charPos; // Reprocess this character on the new line.
                }
                if (currentFont == pFontSecond)
                    out += "_";
                newlinePos = -1;
            }
        }
        }
    }

    if (lastCopyPos < inString.length()) {
        out += inString.substr(lastCopyPos, inString.length() - lastCopyPos);
    }

    return out;
}

int GUIFont::GetStringHeightWithSecondFont(GUIFont *secondFont, std::string_view text_str, int width, int x) {
    if (text_str.empty()) {
        return 0;
    }

    int uAllHeght = GetHeight() - 3;
    std::string test_string = FitTwoFontStringInWindow(text_str, secondFont, width, x);
    size_t uStringLen = test_string.length();
    for (size_t i = 0; i < uStringLen; ++i) {
        char c = test_string[i];
        switch (c) {
        case '\n': // New line.
            uAllHeght += GetHeight() - 3;
            break;
        case '\f': // Color tag.
            i += 5;
            break;
        default:
            break;
        }
    }

    return uAllHeght;
}
