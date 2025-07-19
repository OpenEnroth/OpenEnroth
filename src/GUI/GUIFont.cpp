#include "GUIFont.h"

#include <sstream>
#include <memory>
#include <algorithm>
#include <ranges>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/LodTextureCache.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIWindow.h"
#include "Library/LodFormats/LodImage.h"

void ReloadFonts() {
    if (assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow->CreateFontTex();
    if (assets->pFontBookLloyds)
        assets->pFontBookLloyds->CreateFontTex();
    if (assets->pFontArrus)
        assets->pFontArrus->CreateFontTex();
    if (assets->pFontLucida)
        assets->pFontLucida->CreateFontTex();
    if (assets->pFontBookTitle)
        assets->pFontBookTitle->CreateFontTex();
    if (assets->pFontBookCalendar)
        assets->pFontBookCalendar->CreateFontTex();
    if (assets->pFontCreate)
        assets->pFontCreate->CreateFontTex();
    if (assets->pFontCChar)
        assets->pFontCChar->CreateFontTex();
    if (assets->pFontComic)
        assets->pFontComic->CreateFontTex();
    if (assets->pFontSmallnum)
        assets->pFontSmallnum->CreateFontTex();
}

static Color parseColorTag(const char *tag, const Color &defaultColor) {
    char color_code[20];
    strncpy(color_code, tag, 5);
    color_code[5] = 0;
    int color16 = atoi(color_code);
    if (color16 == 0) {
        return defaultColor; // Back to default color.
    } else {
        return Color::fromC16(color16);
    }
}

GUIFont::GUIFont() = default;

GUIFont::~GUIFont() {
    ReleaseFontTex();
}

std::unique_ptr<GUIFont> GUIFont::LoadFont(std::string_view pFontFile) {
    std::unique_ptr<GUIFont> result = std::make_unique<GUIFont>();

    result->_font = lod::decodeFont(pIcons_LOD->LoadCompressedTexture(pFontFile));
    result->CreateFontTex();

    return result;
}

// TODO(pskelton): Save built atlas so it doesnt get recalcualted on reload?
void GUIFont::CreateFontTex() {
    this->ReleaseFontTex();
    // create blank textures
    this->fonttex = GraphicsImage::Create(512, 512);
    this->fontshadow = GraphicsImage::Create(512, 512);
    Color *pPixelsfont = this->fonttex->rgba().pixels().data();
    Color *pPixelsshadow = this->fontshadow->rgba().pixels().data();

    // load in char pixels into squares within texture
    for (int l = 0; l < 256; l++) {
        int xsq = l % 16;
        int ysq = l / 16;
        int offset = 32 * xsq + 32 * ysq * 512;
        GrayscaleImageView image = _font.image(l);
        const uint8_t *pCharPixels = image.pixels().data();

        for (unsigned y = 0; y < image.height(); ++y) {
            for (unsigned x = 0; x < image.width(); ++x) {
                if (*pCharPixels) {
                    if (*pCharPixels != 1) {
                        // add to normal
                        pPixelsfont[offset + x + y * 512] = colorTable.White;
                    }
                    if (*pCharPixels == 1) {
                        // add to shadow
                        pPixelsshadow[offset + x + y * 512] = colorTable.White;
                    }
                }
                ++pCharPixels;
            }
        }
    }

    render->Update_Texture(this->fonttex);
    render->Update_Texture(this->fontshadow);
}

void GUIFont::ReleaseFontTex() {
    if (this->fonttex) {
        this->fonttex->releaseRenderId();
        this->fonttex->Release();
    }
    if (this->fontshadow) {
        this->fontshadow->releaseRenderId();
        this->fontshadow->Release();
    }
}

bool GUIFont::IsCharValid(unsigned char c) const {
    return _font.supports(c) || (c == '\f') || (c == '\r') || (c == '\t') || (c == '\n');
}

int GUIFont::GetHeight() const {
    return _font.height();
}

Color GUIFont::DrawTextLine(std::string_view text, Color color, Color defaultColor, Pointi position, int max_len_pix) {
    assert(color.a > 0);

    if (text.empty()) {
        return color;
    }
    render->BeginTextNew(fonttex, fontshadow);

    Color text_color = color;
    size_t text_length = text.size();
    int uX_pos = position.x;
    for (int i = 0; i < text_length; ++i) {
        unsigned char c = text[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10:
                return text_color;
                break;
            case '\f':  // Form Feed, page eject  0C 12
                text_color = parseColorTag(&text[i + 1], defaultColor);
                i += 5;
                break;
            case '\t':  // Horizontal tab 09
            case '\r':  // Carriage Return 0D 13
                break;
            default:
                int uCharWidth = _font.metrics(c).width;
                if (uCharWidth) {
                    if (i > 0) {
                        uX_pos += _font.metrics(c).leftSpacing;
                    }

                    int xsq = c % 16;
                    int ysq = c / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + _font.metrics(c).width) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + _font.height()) / 512.0f;

                    render->DrawTextNew(uX_pos, position.y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 1, colorTable.Black);
                    render->DrawTextNew(uX_pos, position.y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 0, text_color);

                    uX_pos += uCharWidth;
                    if (i < text_length) {
                        uX_pos += _font.metrics(c).rightSpacing;
                    }
                }
            }
        }
    }
    return text_color;
}

void DrawCharToBuff(Color *draw_buff, const uint8_t *pCharPixels, int uCharWidth, int uCharHeight,
                    Color draw_color, Color shadowColor, int line_width) {
    assert(draw_color.a > 0);

    const uint8_t *pPixels = pCharPixels;
    for (int y = 0; y < uCharHeight; ++y) {
        for (int x = 0; x < uCharWidth; ++x) {
            uint8_t char_pxl = *pPixels++;
            if (char_pxl) {
                if (char_pxl == 1) {
                    *draw_buff = shadowColor;
                } else {
                    *draw_buff = draw_color;
                }
            }
            ++draw_buff;
        }
        draw_buff += line_width - uCharWidth;
    }
}

void GUIFont::DrawTextLineToBuff(Color color, Color shadowColor, Color *uX_buff_pos, std::string_view text, int line_width) {
    assert(color.a > 0);

    if (text.empty()) {
        return;
    }

    Color text_color = color;
    size_t text_length = text.length();
    Color *uX_pos = uX_buff_pos;
    for (size_t i = 0; i < text_length; ++i) {
        uint8_t c = text[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10:
                return;
                break;
            case '\f':  // Form Feed, page eject  0C 12
                text_color = parseColorTag(&text[i + 1], color);
                i += 5;
                break;
            case '\t':  // Horizontal tab 09
            case '_':
                break;
            default:
            {
                int uCharWidth = _font.metrics(c).width;
                if (uCharWidth) {
                    if (i > 0) {
                        uX_pos += _font.metrics(c).leftSpacing;
                    }
                    const uint8_t *pCharPixels = _font.image(c).pixels().data();
                    DrawCharToBuff(uX_pos, pCharPixels, uCharWidth, _font.height(), text_color, shadowColor, line_width);
                    uX_pos += uCharWidth;
                    if (i < text_length) {
                        uX_pos += _font.metrics(c).rightSpacing;
                    }
                }
            }
            }
        }
    }
}

std::string GUIFont::GetPageTop(std::string_view pInString, GUIWindow *pWindow, unsigned int uX, int a5) {
    if (pInString.empty()) {
        return "";
    }

    int text_height = 0;

    std::string text_str = FitTextInAWindow(pInString, pWindow->uFrameWidth, uX);
    int text_length = text_str.length();
    for (int i = 0; i < text_length; ++i) {
        unsigned char c = text_str[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10
                text_height += (_font.height() - 3);
                if (text_height >= (int)(a5 * (pWindow->uFrameHeight - (_font.height() - 3)))) {
                    return &text_str[i];
                }
                break;
            case '\f':  // Form Feed, page eject 0C 12
                i += 5;
                break;
            case '\t':  // Horizontal tab 09
            case '\r':  // Carriage Return 0D 13
                i += 3;
                break;
            }
            if (text_height >= (int)(a5 * pWindow->uFrameHeight)) {
                break;
            }
        }
    }
    return text_str;
}

int GUIFont::CalcTextHeight(std::string_view pString, int width, int uXOffset, bool return_on_carriage) {
    if (pString.empty()) {
        return 0;
    }

    int uAllHeght = _font.height() - 6;
    std::string test_string = FitTextInAWindow(pString, width, uXOffset);
    size_t uStringLen = pString.length();
    for (int i = 0; i < uStringLen; ++i) {
        unsigned char c = test_string[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10
                uAllHeght += _font.height() - 3;
                break;
            case '\f':  // Form Feed, page eject  0C 12
                i += 5;
                break;
            case '\t':  // Horizontal tab 09
            case '\r':  // Carriage Return 0D 13
                if (!return_on_carriage) {
                    i += 3;
                }
                break;
            }
        }
    }

    return uAllHeght;
}

int GUIFont::GetLineWidth(std::string_view inString) {
    size_t str_len = inString.length();
    int string_line_width = 0;
    for (int i = 0; i < str_len; ++i) {
        unsigned char c = inString[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\t':
            case '\n':
            case '\r':
                return string_line_width;
            case '\f':
                i += 5;
                break;
            default:
                if (i > 0) {
                    string_line_width += _font.metrics(c).leftSpacing;
                }
                string_line_width += _font.metrics(c).width;
                if (i < str_len - 1) {
                    string_line_width += _font.metrics(c).rightSpacing;
                }
            }
        }
    }
    return string_line_width;
}

int GUIFont::AlignText_Center(int width, std::string_view pString) {
    int position = (width - GetLineWidth(pString)) / 2;
    return (position < 0) ? 0 : position;
}

std::string GUIFont::FitTextInAWindow(std::string_view inString, int width, int uX, bool return_on_carriage) {
    assert(uX < width);

    if (inString.empty()) {
        return {};
    }

    int lineWidth = uX;
    int newlinePos = -1;
    int lastCopyPos = 0;
    std::string out;

    for (int i = 0; i < inString.length(); i++) {
        unsigned char c = inString[i];

        if (IsCharValid(c)) {
            switch (c) {
              case '\t': // Horizontal tab
                {
                    char digits[4];
                    strncpy(digits, &inString[i + 1], 3);
                    digits[3] = 0;
                    lineWidth = atoi(digits) + uX;
                    i += 3;
                    break;
                }
              case '\n': // Line Feed
                lineWidth = uX;
                newlinePos = -1;
                out += inString.substr(lastCopyPos, i - lastCopyPos);
                out += "\n";
                lastCopyPos = i + 1;
                break;
              case '\f': // Form Feed, page eject
                i += 5;
                break;
              case '\r': // Carriage Return
                if (!return_on_carriage) {
                    return std::string(inString); // TODO(captainurist): this return is very sus.
                }
                break;
              case ' ': // Space
                lineWidth += _font.metrics(c).width;
                newlinePos = i;
                break;
              default:
                if ((lineWidth + _font.metrics(c).width + _font.metrics(c).leftSpacing + _font.metrics(c).rightSpacing) < width) {
                    if (i > newlinePos)
                        lineWidth += _font.metrics(c).leftSpacing;
                    lineWidth += _font.metrics(c).width;
                    if (i < inString.length() - 1)
                        lineWidth += _font.metrics(c).rightSpacing;
                } else {
                    lineWidth = uX;
                    if (newlinePos >= 0) {
                        out += inString.substr(lastCopyPos, newlinePos - lastCopyPos);
                        out += "\n";
                        i = newlinePos;
                        lastCopyPos = i + 1;
                    } else {
                        out += inString.substr(lastCopyPos, i - lastCopyPos);
                        out += "\n";
                        lastCopyPos = i;
                        i--;
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

void GUIFont::DrawText(GUIWindow *window, Pointi position, Color color, std::string_view text, int maxHeight, Color shadowColor) {
    assert(color.a > 0);

    int left_margin = 0;
    if (text.empty()) {
        return;
    }
    if (text == "null") {
        return;
    }

    render->BeginTextNew(fonttex, fontshadow);

    size_t v30 = text.length();
    if (!position.x) {
        position.x = 12;
    }

    std::string string_begin = std::string(text);
    if (maxHeight == 0) {
        string_begin = FitTextInAWindow(text, window->uFrameWidth, position.x);
    }
    auto string_end = string_begin;
    auto string_base = string_begin;

    int out_x = position.x + window->uFrameX;
    int out_y = position.y + window->uFrameY;

    if (maxHeight != 0 && out_y + _font.height() > maxHeight) {
        return;
    }

    Color draw_color = color;

    char Dest[6] = { 0 };
    size_t v14 = 0;
    if (v30 > 0) {
        do {
            uint8_t c = string_base[v14];
            if (_font.supports(c)
                || c == '\f'
                || c == '\r'
                || c == '\t'
                || c == '\n') {
                switch (c) {
                case '\t':
                    strncpy(Dest, &string_base[v14 + 1], 3);
                    Dest[3] = 0;
                    v14 += 3;
                    left_margin = atoi(Dest);
                    out_x = position.x + window->uFrameX + left_margin;
                    break;
                case '\n':
                    position.y = position.y + _font.height() - 3;
                    out_y = position.y + window->uFrameY;
                    out_x = position.x + window->uFrameX + left_margin;
                    if (maxHeight != 0) {
                        if (_font.height() + out_y - 3 > maxHeight) {
                            return;
                        }
                    }
                    break;
                case '\f':
                    draw_color = parseColorTag(&string_base[v14 + 1], color);
                    v14 += 5;
                    break;
                case '\r':
                    strncpy(Dest, &string_base[v14 + 1], 3);
                    Dest[3] = 0;
                    v14 += 3;
                    left_margin = atoi(Dest);
                    out_x = window->uFrameZ - this->GetLineWidth(&string_base[v14]) - left_margin;
                    out_y = position.y + window->uFrameY;
                    if (maxHeight != 0) {
                        if (_font.height() + out_y - 3 > maxHeight) {
                            return;
                        }
                        break;
                    }
                    break;

                default:
                    if (c == '\"' && string_base[v14 + 1] == '\"') {
                        ++v14;
                    }

                    c = (uint8_t)string_base[v14];
                    if (v14 > 0) {
                        out_x += _font.metrics(c).leftSpacing;
                    }

                    int xsq = c % 16;
                    int ysq = c / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + _font.metrics(c).width) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + _font.height()) / 512.0f;

                    render->DrawTextNew(out_x, out_y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 1, shadowColor);
                    render->DrawTextNew(out_x, out_y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 0, draw_color);

                    out_x += _font.metrics(c).width;
                    if (v14 < v30) {
                        out_x += _font.metrics(c).rightSpacing;
                    }
                    break;
                }
            }
        } while (++v14 < v30);
    }
    // render->EndTextNew();
}

int GUIFont::DrawTextInRect(GUIWindow *window, Pointi position, Color color, std::string_view text, int rect_width, int reverse_text) {
    assert(color.a > 0);

    char buf[4096];
    assert(text.length() < sizeof(buf));
    strncpy(buf, text.data(), text.size());
    buf[text.size()] = '\0';

    size_t pNumLen = strlen(buf);
    if (pNumLen == 0) return 0;

    unsigned int pLineWidth = this->GetLineWidth(buf);
    if (pLineWidth < rect_width) {
        this->DrawText(window, position, color, buf, 0, colorTable.Black);
        return pLineWidth;
    }

    render->BeginTextNew(fonttex, fontshadow);

    unsigned int text_width = 0;
    if (reverse_text)
        std::reverse(buf, buf + pNumLen);

    size_t Str1a = 0;
    size_t i = 0;
    for (i = 0; i < pNumLen; ++i) {
        if (text_width >= rect_width) {
            break;
        }
        uint8_t c = buf[i];
        if (this->IsCharValid(c)) {
            switch (c) {
            case '\t':  // Horizontal tab 09
            case '\n':  // Line Feed 0A 10
            case '\r':  // Form Feed, page eject  0C 12
                break;
            case '\f':  // Carriage Return 0D 13
                i += 5;
                break;
            default:
                if (i > 0) {
                    text_width += _font.metrics(c).leftSpacing;
                }
                text_width += _font.metrics(c).width;
                if (i < pNumLen) {
                    text_width += _font.metrics(c).rightSpacing;
                }
            }
        }
    }
    buf[i - 1] = 0;

    pNumLen = strlen(buf);
    unsigned int v28 = this->GetLineWidth(buf);
    if (reverse_text)
        std::reverse(buf, buf + pNumLen);

    Color draw_color = color;

    int text_pos_x = position.x + window->uFrameX;
    int text_pos_y = position.y + window->uFrameY;
    for (i = 0; i < pNumLen; ++i) {
        uint8_t c = buf[i];
        if (this->IsCharValid(c)) {
            switch (c) {
            case '\t': {  // Horizontal tab 09
                char Str[6];
                strncpy(Str, &buf[i + 1], 3);
                Str[3] = 0;
                //   atoi(Str);
                i += 3;
                break;
            }
            case '\n': {  // Line Feed 0A 10
                unsigned int v24 = _font.height();
                text_pos_x = position.x;
                position.y = position.y + _font.height() - 3;
                text_pos_y = position.y + _font.height() - 3;
                break;
            }
            case '\r':  // Form Feed, page eject  0C 12
                draw_color = parseColorTag(&buf[i + 1], color);
                i += 5;
                break;
            case '\f': {  // Carriage Return 0D 13
                char Str[6];
                strncpy(Str, &buf[i + 1], 3);
                Str[3] = 0;
                i += 3;
                unsigned int v23 = this->GetLineWidth(&buf[i]);
                text_pos_x = window->uFrameZ - v23 - atoi(Str);
                text_pos_y = position.y;
                break;
            }
            default: {
                unsigned int char_width = _font.metrics(c).width;
                if (i > 0) {
                    text_pos_x += _font.metrics(c).leftSpacing;
                }
                int xsq = c % 16;
                int ysq = c / 16;
                float u1 = (xsq * 32.0f) / 512.0f;
                float u2 = (xsq * 32.0f + _font.metrics(c).width) / 512.0f;
                float v1 = (ysq * 32.0f) / 512.0f;
                float v2 = (ysq * 32.0f + _font.height()) / 512.0f;

                render->DrawTextNew(text_pos_x, text_pos_y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 1, colorTable.Black);
                render->DrawTextNew(text_pos_x, text_pos_y, _font.metrics(c).width, _font.height(), u1, v1, u2, v2, 0, draw_color);

                text_pos_x += char_width;
                if (i < (int)pNumLen) {
                    text_pos_x += _font.metrics(c).rightSpacing;
                }
            }
            }
        }
    }
    render->EndTextNew();
    return v28;
}

void GUIFont::DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h,
                               Color firstColor, Color secondColor, Color shadowColor, std::string_view pString,
                               GraphicsImage *image) {
    GUIWindow draw_window;
    draw_window.uFrameHeight = h;
    draw_window.uFrameW = uFrameY + h - 1;
    draw_window.uFrameWidth = w;
    draw_window.uFrameZ = uFrameX + w - 1;
    draw_window.uFrameX = uFrameX;
    draw_window.uFrameY = uFrameY;

    std::string work_string = FitTwoFontStringINWindow(pString, pSecondFont, &draw_window, 0, 1);
    std::istringstream stream(work_string);
    std::getline(stream, work_string);

    Color *pPixels = image->rgba().pixels().data();
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

std::string GUIFont::FitTwoFontStringINWindow(std::string_view inString, GUIFont *pFontSecond, GUIWindow *pWindow, int startPixlOff, bool return_on_carriage) {
    if (inString.empty()) {
        return "";
    }

    GUIFont *currentFont = this;
    GUIFont *newlineFont = this;
    int lineWidth = startPixlOff;
    int newlinePos = -1;
    int lastCopyPos = 0;
    std::string out;

    for (int i = 0; i < inString.length(); i++) {
        unsigned char c = inString[i];

        if (IsCharValid(c)) {
            switch (c) {
              case '\t': // Horizontal tab
                {
                    char digits[4];
                    strncpy(digits, &inString[i + 1], 3);
                    digits[3] = 0;
                    lineWidth = atoi(digits) + startPixlOff;
                    i += 3;
                    break;
                }
              case '\n': // Line Feed
                lineWidth = startPixlOff;
                newlinePos = -1;
                out += inString.substr(lastCopyPos, i - lastCopyPos);
                out += "\n";
                lastCopyPos = i + 1;
                currentFont = this;
                break;
              case '\f': // Form Feed, page eject
                i += 5;
                break;
              case '\r': // Carriage Return
                if (!return_on_carriage) {
                    return std::string(inString); // TODO(captainurist): very sus return.
                }
                break;
              case ' ': // Space
                lineWidth += currentFont->_font.metrics(c).width;
                newlinePos = i;
                newlineFont = currentFont;
                break;
              case '_':
                currentFont = pFontSecond;
                break;
              default:
                if ((lineWidth + currentFont->_font.metrics(c).width + currentFont->_font.metrics(c).leftSpacing + currentFont->_font.metrics(c).rightSpacing) < pWindow->uFrameWidth) {
                    if (i > newlinePos)
                        lineWidth += currentFont->_font.metrics(c).leftSpacing;
                    lineWidth += currentFont->_font.metrics(c).width;
                    if (i < inString.length() - 1)
                        lineWidth += currentFont->_font.metrics(c).rightSpacing;
                } else {
                    lineWidth = startPixlOff;
                    currentFont = newlineFont;
                    if (newlinePos >= 0) {
                        out += inString.substr(lastCopyPos, newlinePos - lastCopyPos);
                        out += "\n";
                        i = newlinePos;
                        lastCopyPos = i + 1;
                    } else {
                        out += inString.substr(lastCopyPos, i - lastCopyPos);
                        out += "\n";
                        lastCopyPos = i;
                        i--;
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

int GUIFont::GetStringHeight2(GUIFont *secondFont, std::string_view text_str, GUIWindow *pWindow, int startX, int a6) {
    if (text_str.empty()) {
        return 0;
    }

    int uAllHeght = GetHeight() - 3;
    std::string test_string = FitTwoFontStringINWindow(text_str, secondFont, pWindow, startX, 0);
    size_t uStringLen = test_string.length();
    for (size_t i = 0; i < uStringLen; ++i) {
        unsigned char c = test_string[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10:
                uAllHeght += GetHeight() - 3;
                break;
            case '\f':  // Form Feed, page eject  0C 12
                i += 5;
                break;
            case '\t':  // Horizontal tab 09
            case '\r':  // Carriage Return 0D 13
                if (a6 != 1)
                    i += 3;
                break;
            }
        }
    }

    return uAllHeght;
}
