#include "GUI/GUIFont.h"

#include <cstdarg>

#include <sstream>

#include "Engine/Engine.h"
#include "Engine/LodTextureCache.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"

#include "Engine/Snapshots/EntitySnapshots.h"

#include "GUI/GUIWindow.h"

// TODO(pskelton): Move these to asset manager
GUIFont *pFontBookOnlyShadow = nullptr;
GUIFont *pFontBookLloyds = nullptr;
GUIFont *pFontArrus = nullptr;
GUIFont *pFontLucida = nullptr;
GUIFont *pFontBookTitle = nullptr;
GUIFont *pFontBookCalendar = nullptr;
GUIFont *pFontCreate = nullptr;
GUIFont *pFontCChar = nullptr;
GUIFont *pFontComic = nullptr;
GUIFont *pFontSmallnum = nullptr;

void ReloadFonts() {
    if (pFontBookOnlyShadow)
        pFontBookOnlyShadow->CreateFontTex();
    if (pFontBookLloyds)
        pFontBookLloyds->CreateFontTex();
    if (pFontArrus)
        pFontArrus->CreateFontTex();
    if (pFontLucida)
        pFontLucida->CreateFontTex();
    if (pFontBookTitle)
        pFontBookTitle->CreateFontTex();
    if (pFontBookCalendar)
        pFontBookCalendar->CreateFontTex();
    if  (pFontCreate)
        pFontCreate->CreateFontTex();
    if (pFontCChar)
        pFontCChar->CreateFontTex();
    if (pFontComic)
        pFontComic->CreateFontTex();
    if (pFontSmallnum)
        pFontSmallnum->CreateFontTex();
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

char temp_string[2048];

std::array<char, 10000> pTmpBuf3;

GUIFont *GUIFont::LoadFont(const char *pFontFile, const char *pFontPalette) {
    // static_assert(sizeof(GUICharMetric) == 12, "Wrong GUICharMetric type size");
    // static_assert(sizeof(FontData) == 4128, "Wrong FontData type size");

    GUIFont *pFont = new GUIFont;

    // pFont->pData = (FontData*)pIcons_LOD->LoadCompressedTexture(pFontFile);
    Blob tmp_font = pIcons_LOD->LoadCompressedTexture(pFontFile);
    reconstruct(*static_cast<const FontData_MM7 *>(tmp_font.data()), tmp_font.size(), pFont->pData);

    Texture_MM7 *pallete_texture = pIcons_LOD->loadTexture(pFontPalette);
    if (!pallete_texture)
        Error("Unable to open %s", pFontPalette);

    pFont->pData->pFontPalettes[0] = pallete_texture->pPalette24;
    pFont->pData->palletes_count = 1;

    // get max xhar width
    pFont->maxcharwidth = 0;
    for (int l = 0; l < 256; l++) {
        if (pFont->pData->pMetrics[l].uWidth > pFont->maxcharwidth) pFont->maxcharwidth = pFont->pData->pMetrics[l].uWidth;
    }

    pFont->CreateFontTex();

    return pFont;
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
        uint8_t *pCharPixels = &this->pData->pFontData[this->pData->font_pixels_offset[l]];

        for (uint y = 0; y < this->pData->uFontHeight; ++y) {
            for (uint x = 0; x < this->pData->pMetrics[l].uWidth; ++x) {
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
    if (this->fonttex)
        this->fonttex->releaseRenderId();
    if (this->fontshadow)
        this->fontshadow->releaseRenderId();
}

bool GUIFont::IsCharValid(unsigned char c) const {
    return (c >= pData->cFirstChar) && (c <= pData->cLastChar) || (c == '\f') || (c == '\r') || (c == '\t') || (c == '\n');
}

int GUIFont::GetHeight() const {
    return pData->uFontHeight;
}

Color GUIFont::DrawTextLine(const std::string &text, Color color, Color defaultColor, Pointi position, int max_len_pix) {
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
                int uCharWidth = pData->pMetrics[c].uWidth;
                if (uCharWidth) {
                    if (i > 0) {
                        uX_pos += pData->pMetrics[c].uLeftSpacing;
                    }
                    uint8_t *pCharPixels = &pData->pFontData[pData->font_pixels_offset[c]];

                    int xsq = c % 16;
                    int ysq = c / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + pData->pMetrics[c].uWidth) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                    render->DrawTextNew(uX_pos, position.y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, colorTable.Black);
                    render->DrawTextNew(uX_pos, position.y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, text_color);

                    uX_pos += uCharWidth;
                    if (i < text_length) {
                        uX_pos += pData->pMetrics[c].uRightSpacing;
                    }
                }
            }
        }
    }
    return text_color;
}

void DrawCharToBuff(Color *draw_buff, const uint8_t *pCharPixels, int uCharWidth, int uCharHeight,
                    uint8_t *pFontPalette, Color draw_color, int line_width) {
    assert(draw_color.a > 0);

    const uint8_t *pPixels = pCharPixels;
    for (int y = 0; y < uCharHeight; ++y) {
        for (int x = 0; x < uCharWidth; ++x) {
            uint8_t char_pxl = *pPixels++;
            if (char_pxl) {
                if (char_pxl == 1) {
                    uint8_t r = pFontPalette[3];
                    uint8_t g = pFontPalette[4];
                    uint8_t b = pFontPalette[5];
                    *draw_buff = Color(r, g, b);
                } else {
                    *draw_buff = draw_color;
                }
            }
            ++draw_buff;
        }
        draw_buff += line_width - uCharWidth;
    }
}

void GUIFont::DrawTextLineToBuff(Color color, Color *uX_buff_pos, const std::string &text, int line_width) {
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
                int uCharWidth = pData->pMetrics[c].uWidth;
                if (uCharWidth) {
                    if (i > 0) {
                        uX_pos += pData->pMetrics[c].uLeftSpacing;
                    }
                    const uint8_t *pCharPixels = &pData->pFontData[pData->font_pixels_offset[c]];
                    DrawCharToBuff(uX_pos, pCharPixels, uCharWidth, pData->uFontHeight, pData->pFontPalettes[0], text_color, line_width);
                    uX_pos += uCharWidth;
                    if (i < text_length) {
                        uX_pos += pData->pMetrics[c].uRightSpacing;
                    }
                }
            }
            }
        }
    }
}

std::string GUIFont::GetPageTop(const std::string &pInString, GUIWindow *pWindow, unsigned int uX, int a5) {
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
                text_height += (pData->uFontHeight - 3);
                if (text_height >= (int)(a5 * (pWindow->uFrameHeight - (pData->uFontHeight - 3)))) {
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

int GUIFont::CalcTextHeight(const std::string &pString, int width, int uXOffset, bool return_on_carriage) {
    if (pString.empty()) {
        return 0;
    }

    int uAllHeght = pData->uFontHeight - 6;
    std::string test_string = FitTextInAWindow(pString, width, uXOffset);
    size_t uStringLen = pString.length();
    for (int i = 0; i < uStringLen; ++i) {
        unsigned char c = test_string[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10
                uAllHeght += pData->uFontHeight - 3;
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

int GUIFont::GetLineWidth(const std::string &inString) {
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
                    string_line_width += pData->pMetrics[c].uLeftSpacing;
                }
                string_line_width += pData->pMetrics[c].uWidth;
                if (i < str_len - 1) {
                    string_line_width += pData->pMetrics[c].uRightSpacing;
                }
            }
        }
    }
    return string_line_width;
}

int GUIFont::AlignText_Center(int width, const std::string &pString) {
    int position = (width - GetLineWidth(pString)) / 2;
    return (position < 0) ? 0 : position;
}

std::string GUIFont::FitTextInAWindow(const std::string &inString, unsigned int width, int uX, bool return_on_carriage) {
    size_t uInStrLen = inString.length();
    strcpy(temp_string, inString.c_str());
    if (uInStrLen == 0) {
        return &temp_string[0];
    }

    int start_pixel_offset = uX;
    int string_pixel_Width = uX;
    int possible_transition_point = 0;
    for (int i = 0; i < uInStrLen; ++i) {
        unsigned char c = temp_string[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\t':
            {  // Horizontal tab 09
                char digits[4];
                strncpy(digits, &temp_string[i + 1], 3);
                digits[3] = 0;
                string_pixel_Width = atoi(digits) + uX;
                i += 3;
                break;
            }
            case  '\n':
            {  // Line Feed 0A 10 (конец строки)
                string_pixel_Width = start_pixel_offset;
                possible_transition_point = i;
                break;
            }
            case  '\f':
            {  // Form Feed, page eject  0C 12
                i += 5;
                break;
            }
            case  '\r':
            {  // Carriage Return 0D 13
                if (!return_on_carriage) {
                    return inString;
                }
                break;
            }
            case ' ':
            {  // Space
                string_pixel_Width += pData->pMetrics[c].uWidth;
                possible_transition_point = i;
                break;
            }
            default:
                if ((string_pixel_Width + pData->pMetrics[c].uWidth + pData->pMetrics[c].uLeftSpacing + pData->pMetrics[c].uRightSpacing) < width) {  // наращивание длины строки или перенос
                    if (i > possible_transition_point)
                        string_pixel_Width += pData->pMetrics[c].uLeftSpacing;
                    string_pixel_Width += pData->pMetrics[c].uWidth;
                    if (i < uInStrLen - 1)
                        string_pixel_Width += pData->pMetrics[c].uRightSpacing;
                } else {  // перенос строки и слова
                    temp_string[possible_transition_point] = '\n';
                    string_pixel_Width = start_pixel_offset;
                    if (i > possible_transition_point) {
                        for (int j = possible_transition_point; j <= i; ++j) {
                            c = temp_string[j];
                            if (IsCharValid(c)) {
                                if (j > possible_transition_point)
                                    string_pixel_Width += pData->pMetrics[c].uLeftSpacing;
                                string_pixel_Width += pData->pMetrics[c].uWidth;
                                string_pixel_Width += pData->pMetrics[c].uRightSpacing;
                            }
                        }
                    }
                }
            }
        }
    }
    return temp_string;
}

void GUIFont::DrawText(GUIWindow *window, Pointi position, Color color, const std::string &text, int maxHeight, Color shadowColor) {
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

    std::string string_begin = text;
    if (maxHeight == 0) {
        string_begin = FitTextInAWindow(text, window->uFrameWidth, position.x);
    }
    auto string_end = string_begin;
    auto string_base = string_begin;

    int out_x = position.x + window->uFrameX;
    int out_y = position.y + window->uFrameY;

    if (maxHeight != 0 && out_y + pData->uFontHeight > maxHeight) {
        return;
    }

    Color draw_color = color;

    char Dest[6] = { 0 };
    size_t v14 = 0;
    if (v30 > 0) {
        do {
            uint8_t c = string_base[v14];
            if (c >= pData->cFirstChar && c <= pData->cLastChar
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
                    position.y = position.y + pData->uFontHeight - 3;
                    out_y = position.y + window->uFrameY;
                    out_x = position.x + window->uFrameX + left_margin;
                    if (maxHeight != 0) {
                        if (pData->uFontHeight + out_y - 3 > maxHeight) {
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
                        if (pData->uFontHeight + out_y - 3 > maxHeight) {
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
                        out_x += pData->pMetrics[c].uLeftSpacing;
                    }

                    unsigned char *letter_pixels = &pData->pFontData[pData->font_pixels_offset[c]];
                    int xsq = c % 16;
                    int ysq = c / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + pData->pMetrics[c].uWidth) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                    render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, shadowColor);
                    render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, draw_color);

                    out_x += pData->pMetrics[c].uWidth;
                    if (v14 < v30) {
                        out_x += pData->pMetrics[c].uRightSpacing;
                    }
                    break;
                }
            }
        } while (++v14 < v30);
    }
    // render->EndTextNew();
}

int GUIFont::DrawTextInRect(GUIWindow *window, Pointi position, Color color, const std::string &text, int rect_width, int reverse_text) {
    assert(color.a > 0);

    char buf[4096];
    Assert(text.length() < sizeof(buf));
    strcpy(buf, text.c_str());

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
        uint8_t v12 = buf[i];
        if (this->IsCharValid(v12)) {
            switch (v12) {
            case '\t':  // Horizontal tab 09
            case '\n':  // Line Feed 0A 10
            case '\r':  // Form Feed, page eject  0C 12
                break;
            case '\f':  // Carriage Return 0D 13
                i += 5;
                break;
            default:
                if (i > 0) {
                    text_width += pData->pMetrics[v12].uLeftSpacing;
                }
                text_width += pData->pMetrics[v12].uWidth;
                if (i < pNumLen) {
                    text_width += pData->pMetrics[v12].uRightSpacing;
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
        uint8_t v15 = buf[i];
        if (this->IsCharValid(v15)) {
            switch (v15) {
            case '\t': {  // Horizontal tab 09
                char Str[6];
                strncpy(Str, &buf[i + 1], 3);
                Str[3] = 0;
                //   atoi(Str);
                i += 3;
                break;
            }
            case '\n': {  // Line Feed 0A 10
                unsigned int v24 = pData->uFontHeight;
                text_pos_x = position.x;
                position.y = position.y + pData->uFontHeight - 3;
                text_pos_y = position.y + pData->uFontHeight - 3;
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
                unsigned int char_width = pData->pMetrics[v15].uWidth;
                if (i > 0) {
                    text_pos_x += pData->pMetrics[v15].uLeftSpacing;
                }
                uint8_t *char_pix_ptr = &pData->pFontData[pData->font_pixels_offset[v15]];
                int xsq = v15 % 16;
                int ysq = v15 / 16;
                float u1 = (xsq * 32.0f) / 512.0f;
                float u2 = (xsq * 32.0f + pData->pMetrics[v15].uWidth) / 512.0f;
                float v1 = (ysq * 32.0f) / 512.0f;
                float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, colorTable.Black);
                render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, draw_color);

                text_pos_x += char_width;
                if (i < (int)pNumLen) {
                    text_pos_x += pData->pMetrics[v15].uRightSpacing;
                }
            }
            }
        }
    }
    render->EndTextNew();
    return v28;
}

void GUIFont::DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h,
                               Color firstColor, Color secondColor, const std::string &pString,
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
            currentFont->DrawTextLineToBuff(currentColor, &curr_pixel_pos[line_w + half_frameX],
                work_string, image->width());
            curr_pixel_pos += image->width() * (currentFont->GetHeight() - 3);
            std::getline(stream, work_string);
            if (work_string.empty()) {
                break;
            }
        }
    }
}

std::string GUIFont::FitTwoFontStringINWindow(const std::string &pString, GUIFont *pFontSecond, GUIWindow *pWindow, int startPixlOff, int a6) {
    if (pString.empty()) {
        return std::string();
    }
    GUIFont *currentFont = this;
    size_t uInStrLen = pString.length();
    Assert(uInStrLen < sizeof(pTmpBuf3));
    strcpy(pTmpBuf3.data(), pString.c_str());
    if (uInStrLen == 0) {
        return pTmpBuf3.data();
    }

    int string_pixel_Width = startPixlOff;
    int start_pixel_offset = startPixlOff;
    int possible_transition_point = 0;
    for (int i = 0; i < uInStrLen; ++i) {
        unsigned char c = pTmpBuf3[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\t': {  // Horizontal tab 09
                char digits[4];
                strncpy(digits, &pTmpBuf3[i + 1], 3);
                digits[3] = 0;
                string_pixel_Width = atoi(digits) + startPixlOff;
                i += 3;
                break;
            }
            case  '\n': {  // Line Feed 0A 10
                string_pixel_Width = start_pixel_offset;
                possible_transition_point = i;
                currentFont = this;
                break;
            }
            case  '\f': {  // Form Feed, page eject  0C 12
                i += 5;
                break;
            }
            case  '\r': {  // Carriage Return 0D 13
                if (!a6)
                    return pString;
                break;
            }
            case ' ': {
                string_pixel_Width += currentFont->pData->pMetrics[c].uWidth;
                possible_transition_point = i;
                break;
            }
            case '_':
                currentFont = pFontSecond;
                break;
            default:

                if ((string_pixel_Width + currentFont->pData->pMetrics[c].uWidth + currentFont->pData->pMetrics[c].uLeftSpacing + currentFont->pData->pMetrics[c].uRightSpacing) < pWindow->uFrameWidth) {
                    if (i > possible_transition_point)
                        string_pixel_Width += currentFont->pData->pMetrics[c].uLeftSpacing;
                    string_pixel_Width += currentFont->pData->pMetrics[c].uWidth;
                    if (i < uInStrLen)
                        string_pixel_Width += currentFont->pData->pMetrics[c].uRightSpacing;
                } else {
                    pTmpBuf3[possible_transition_point] = '\n';
                    if (currentFont == pFontSecond) {
                        for (int k = uInStrLen - 1; k >= possible_transition_point + 1; --k) {
                            pTmpBuf3[k] = pTmpBuf3[k - 1];
                        }
                        ++uInStrLen;
                        ++possible_transition_point;
                        pTmpBuf3[possible_transition_point] = '_';
                    }
                    string_pixel_Width = start_pixel_offset;

                    for (int j = possible_transition_point; j < i; ++j) {
                        c = pTmpBuf3[j];
                        if (IsCharValid(c)) {
                            if (j > possible_transition_point) {
                                string_pixel_Width += pData->pMetrics[c].uLeftSpacing;
                            }
                            string_pixel_Width += pData->pMetrics[c].uWidth;
                            if (j < i) {
                                string_pixel_Width += pData->pMetrics[c].uRightSpacing;
                            }
                        }
                    }
                }
            }
        }
    }

    return std::string(pTmpBuf3.data());
}

int GUIFont::GetStringHeight2(GUIFont *secondFont, const std::string &text_str, GUIWindow *pWindow, int startX, int a6) {
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
