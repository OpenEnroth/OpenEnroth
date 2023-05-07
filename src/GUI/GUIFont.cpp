#include "GUI/GUIFont.h"

#include <cstdarg>

#include <sstream>

#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Serialization/LegacyImages.h"

#include "GUI/GUIWindow.h"

extern LODFile_IconsBitmaps *pIcons_LOD;

// TODO(pskelton): Move these to asset manager
GUIFont *pAutonoteFont = nullptr;
GUIFont *pSpellFont = nullptr;
GUIFont *pFontArrus = nullptr;
GUIFont *pFontLucida = nullptr;
GUIFont *pBook2Font = nullptr;
GUIFont *pBookFont = nullptr;
GUIFont *pFontCreate = nullptr;
GUIFont *pFontCChar = nullptr;
GUIFont *pFontComic = nullptr;
GUIFont *pFontSmallnum = nullptr;

void ReloadFonts() {
    if (pAutonoteFont)
        pAutonoteFont->CreateFontTex();
    if (pSpellFont)
        pSpellFont->CreateFontTex();
    if (pFontArrus)
        pFontArrus->CreateFontTex();
    if (pFontLucida)
        pFontLucida->CreateFontTex();
    if (pBook2Font)
        pBook2Font->CreateFontTex();
    if (pBookFont)
        pBookFont->CreateFontTex();
    if  (pFontCreate)
        pFontCreate->CreateFontTex();
    if (pFontCChar)
        pFontCChar->CreateFontTex();
    if (pFontComic)
        pFontComic->CreateFontTex();
    if (pFontSmallnum)
        pFontSmallnum->CreateFontTex();
}

char temp_string[2048];

std::array<char, 10000> pTmpBuf3;

GUIFont *GUIFont::LoadFont(const char *pFontFile, const char *pFontPalette) {
    // static_assert(sizeof(GUICharMetric) == 12, "Wrong GUICharMetric type size");
    // static_assert(sizeof(FontData) == 4128, "Wrong FontData type size");

    GUIFont *pFont = new GUIFont;

    // pFont->pData = (FontData*)pIcons_LOD->LoadCompressedTexture(pFontFile);
    Blob tmp_font = pIcons_LOD->LoadCompressedTexture(pFontFile);
    deserialize(*static_cast<const FontData_MM7 *>(tmp_font.data()), tmp_font.size(), pFont->pData);

    int pallete_index = pIcons_LOD->LoadTexture(pFontPalette, TEXTURE_24BIT_PALETTE);
    if (pallete_index == -1)
        Error("Unable to open %s", pFontPalette);

    pFont->pData->pFontPalettes[0] = pIcons_LOD->pTextures[pallete_index].pPalette24;
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
    this->fonttex = render->CreateTexture_Blank(512, 512, IMAGE_FORMAT_A8B8G8R8);
    this->fontshadow = render->CreateTexture_Blank(512, 512, IMAGE_FORMAT_A8B8G8R8);
    uint32_t *pPixelsfont = (uint32_t*)this->fonttex->GetPixels(IMAGE_FORMAT_A8B8G8R8);
    uint32_t *pPixelsshadow = (uint32_t*)this->fontshadow->GetPixels(IMAGE_FORMAT_A8B8G8R8);

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
                        pPixelsfont[offset + x + y * 512] = colorTable.White.c32();
                    }
                    if (*pCharPixels == 1) {
                        // add to shadow
                        pPixelsshadow[offset + x + y * 512] = colorTable.White.c32();
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
        render->DeleteTexture(this->fonttex);
    if (this->fontshadow)
        render->DeleteTexture(this->fontshadow);
}

bool GUIFont::IsCharValid(unsigned char c) const {
    return (c >= pData->cFirstChar) && (c <= pData->cLastChar) || (c == '\f') || (c == '\r') || (c == '\t') || (c == '\n');
}

int GUIFont::GetHeight() const {
    return pData->uFontHeight;
}

void GUIFont::DrawTextLine(const std::string &text, uint16_t uDefaultColor, Pointi position, int max_len_pix) {
    if (text.empty()) {
        return;
    }
    render->BeginTextNew(fonttex, fontshadow);

    uint16_t text_color = ui_current_text_color;
    size_t text_length = text.size();
    int uX_pos = position.x;
    for (int i = 0; i < text_length; ++i) {
        unsigned char c = text[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10:
                return;
                break;
            case '\f': {  // Form Feed, page eject  0C 12
                char color_code[20];
                strncpy(color_code, &text[i + 1], 5);
                color_code[5] = 0;
                text_color = atoi(color_code);
                ui_current_text_color = text_color;
                i += 5;
                break;
            }
            case '\t':  // Horizontal tab 09
            case '\r':  // Carriage Return 0D 13
                break;
            default:
                int uCharWidth = pData->pMetrics[c].uWidth;
                if (uCharWidth) {
                    if (i > 0) {
                        uX_pos += pData->pMetrics[c].uLeftSpacing;
                    }
                    uint16_t draw_color = text_color;
                    uint8_t *pCharPixels = &pData->pFontData[pData->font_pixels_offset[c]];
                    if (!text_color) {
                        draw_color = colorTable.White.c16();
                    }

                    int xsq = c % 16;
                    int ysq = c / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + pData->pMetrics[c].uWidth) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                    render->DrawTextNew(uX_pos, position.y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, 0);
                    render->DrawTextNew(uX_pos, position.y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, draw_color);

                    uX_pos += uCharWidth;
                    if (i < text_length) {
                        uX_pos += pData->pMetrics[c].uRightSpacing;
                    }
                }
            }
        }
    }
    // render->EndTextNew();
}

void DrawCharToBuff(uint32_t *draw_buff, uint8_t *pCharPixels, int uCharWidth, int uCharHeight,
    uint8_t *pFontPalette, uint16_t draw_color, int line_width) {
    uint8_t *pPixels = pCharPixels;
    for (int y = 0; y < uCharHeight; ++y) {
        for (int x = 0; x < uCharWidth; ++x) {
            uint8_t char_pxl = *pPixels++;
            if (char_pxl) {
                if (char_pxl == 1) {
                    uint8_t r = pFontPalette[3];
                    uint8_t g = pFontPalette[4];
                    uint8_t b = pFontPalette[5];
                    *draw_buff = color32(r, g, b);
                } else {
                    *draw_buff = color32(draw_color);
                }
            }
            ++draw_buff;
        }
        draw_buff += line_width - uCharWidth;
    }
}

void GUIFont::DrawTextLineToBuff(uint16_t uColor, uint32_t *uX_buff_pos, const std::string &text, int line_width) {
    if (text.empty()) {
        return;
    }

    char color_code[20];

    uint16_t text_color = ui_current_text_color;
    size_t text_length = text.length();
    uint32_t *uX_pos = uX_buff_pos;
    for (size_t i = 0; i < text_length; ++i) {
        uint8_t c = text[i];
        if (IsCharValid(c)) {
            switch (c) {
            case '\n':  // Line Feed 0A 10:
                return;
                break;
            case '\f':  // Form Feed, page eject  0C 12
                strncpy(color_code, &text[i + 1], 5);
                color_code[5] = 0;
                text_color = atoi(color_code);
                ui_current_text_color = text_color;
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
                    uint16_t draw_color = text_color;
                    uint8_t *pCharPixels = &pData->pFontData[pData->font_pixels_offset[c]];
                    if (!text_color) {
                        draw_color = -1;
                    }
                    DrawCharToBuff(uX_pos, pCharPixels, uCharWidth, pData->uFontHeight, pData->pFontPalettes[0], draw_color, line_width);
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
                if (i < str_len) {
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
                    if (i < uInStrLen)
                        string_pixel_Width += pData->pMetrics[c].uRightSpacing;
                } else {  // перенос строки и слова
                    temp_string[possible_transition_point] = '\n';
                    string_pixel_Width = start_pixel_offset;
                    if (i > possible_transition_point) {
                        for (int j = possible_transition_point; j < i; ++j) {
                            c = temp_string[j];
                            if (IsCharValid(c)) {
                                if (j > possible_transition_point)
                                    string_pixel_Width += pData->pMetrics[c].uLeftSpacing;
                                string_pixel_Width += pData->pMetrics[c].uWidth;
                                if (j < i)
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

void GUIFont::DrawText(GUIWindow *pWindow, Pointi position, uint16_t uFontColor, const std::string &str,
    bool present_time_transparency, int max_text_height, int uFontShadowColor) {
    int left_margin = 0;
    if (str.empty()) {
        return;
    }
    if (str == "null") {
        return;
    }

    render->BeginTextNew(fonttex, fontshadow);

    size_t v30 = str.length();
    if (!position.x) {
        position.x = 12;
    }

    std::string string_begin = str;
    if (max_text_height == 0) {
        string_begin = FitTextInAWindow(str, pWindow->uFrameWidth, position.x);
    }
    auto string_end = string_begin;
    auto string_base = string_begin;

    int out_x = position.x + pWindow->uFrameX;
    int out_y = position.y + pWindow->uFrameY;

    if (max_text_height != 0 && out_y + pData->uFontHeight > max_text_height) {
        return;
    }

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
                    out_x = position.x + pWindow->uFrameX + left_margin;
                    break;
                case '\n':
                    position.y = position.y + pData->uFontHeight - 3;
                    out_y = position.y + pWindow->uFrameY;
                    out_x = position.x + pWindow->uFrameX + left_margin;
                    if (max_text_height != 0) {
                        if (pData->uFontHeight + out_y - 3 > max_text_height) {
                            return;
                        }
                    }
                    break;
                case '\f':
                    strncpy(Dest, &string_base[v14 + 1], 5);
                    Dest[5] = 0;
                    uFontColor = atoi(Dest);
                    v14 += 5;
                    break;
                case '\r':
                    strncpy(Dest, &string_base[v14 + 1], 3);
                    Dest[3] = 0;
                    v14 += 3;
                    left_margin = atoi(Dest);
                    out_x = pWindow->uFrameZ - this->GetLineWidth(&string_base[v14]) - left_margin;
                    out_y = position.y + pWindow->uFrameY;
                    if (max_text_height != 0) {
                        if (pData->uFontHeight + out_y - 3 > max_text_height) {
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

                    if (present_time_transparency) __debugbreak();


                    unsigned char *letter_pixels = &pData->pFontData[pData->font_pixels_offset[c]];
                    if (uFontColor) {
                        int xsq = c % 16;
                        int ysq = c / 16;
                        float u1 = (xsq * 32.0f) / 512.0f;
                        float u2 = (xsq * 32.0f + pData->pMetrics[c].uWidth) / 512.0f;
                        float v1 = (ysq * 32.0f) / 512.0f;
                        float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                        render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, uFontShadowColor);
                        render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, uFontColor);
                    } else {
                        // pallette24 check for colour
                        unsigned char *pix = letter_pixels;
                        uint8_t *pPalette = pData->pFontPalettes[0];
                        uint8_t r = 0, g = 0, b = 0;
                        for (int t = 0; t < (pData->pMetrics[c].uWidth * pData->uFontHeight); t++) {
                            if (*pix) {
                                uint8_t index = *pix;
                                // if (index != 255 && index != 1) __debugbreak();
                                r = pPalette[index * 3 + 0];
                                g = pPalette[index * 3 + 1];
                                b = pPalette[index * 3 + 2];
                                break;
                            }
                            pix++;
                        }

                        int xsq = c % 16;
                        int ysq = c / 16;
                        float u1 = (xsq * 32.0f) / 512.0f;
                        float u2 = (xsq * 32.0f + pData->pMetrics[c].uWidth) / 512.0f;
                        float v1 = (ysq * 32.0f) / 512.0f;
                        float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                        render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, color16(r, g, b));
                        render->DrawTextNew(out_x, out_y, pData->pMetrics[c].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, colorTable.Black.c16());
                    }

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

int GUIFont::DrawTextInRect(GUIWindow *pWindow, Pointi position, uint16_t uColor, const std::string &str, int rect_width, int reverse_text) {
    char text[4096];
    Assert(str.length() < sizeof(text));
    strcpy(text, str.c_str());

    size_t pNumLen = strlen(text);
    if (pNumLen == 0) return 0;

    unsigned int pLineWidth = this->GetLineWidth(text);
    if (pLineWidth < rect_width) {
        this->DrawText(pWindow, position, uColor, text, 0, 0, 0);
        return pLineWidth;
    }

    render->BeginTextNew(fonttex, fontshadow);

    unsigned int text_width = 0;
    if (reverse_text)
        std::reverse(text, text + pNumLen);

    size_t Str1a = 0;
    size_t i = 0;
    for (i = 0; i < pNumLen; ++i) {
        if (text_width >= rect_width) {
            break;
        }
        uint8_t v12 = text[i];
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
    text[i - 1] = 0;

    pNumLen = strlen(text);
    unsigned int v28 = this->GetLineWidth(text);
    if (reverse_text)
        std::reverse(text, text + pNumLen);

    int text_pos_x = position.x + pWindow->uFrameX;
    int text_pos_y = position.y + pWindow->uFrameY;
    for (i = 0; i < pNumLen; ++i) {
        uint8_t v15 = text[i];
        if (this->IsCharValid(v15)) {
            switch (v15) {
            case '\t': {  // Horizontal tab 09
                char Str[6];
                strncpy(Str, &text[i + 1], 3);
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
            case '\r': {  // Form Feed, page eject  0C 12
                char Str[6];
                strncpy(Str, &text[i + 1], 5);
                Str[5] = 0;
                i += 5;
                uColor = atoi(Str);
                break;
            }
            case '\f': {  // Carriage Return 0D 13
                char Str[6];
                strncpy(Str, &text[i + 1], 3);
                Str[3] = 0;
                i += 3;
                unsigned int v23 = this->GetLineWidth(&text[i]);
                text_pos_x = pWindow->uFrameZ - v23 - atoi(Str);
                text_pos_y = position.y;
                break;
            }
            default: {
                unsigned int char_width = pData->pMetrics[v15].uWidth;
                if (i > 0) {
                    text_pos_x += pData->pMetrics[v15].uLeftSpacing;
                }
                uint8_t *char_pix_ptr = &pData->pFontData[pData->font_pixels_offset[v15]];
                if (uColor) {
                    int xsq = v15 % 16;
                    int ysq = v15 / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + pData->pMetrics[v15].uWidth) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                    render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, 0);
                    render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, uColor);
                } else {
                    // pallette24 check for colour
                    unsigned char *pix = char_pix_ptr;
                    uint8_t *pPalette = pData->pFontPalettes[0];
                    uint8_t r = 0, g = 0, b = 0;
                    for (int t = 0; t < (pData->pMetrics[v15].uWidth * pData->uFontHeight); t++) {
                        if (*pix) {
                            uint8_t index = *pix;
                            // if (index != 255 && index != 1) __debugbreak();
                            r = pPalette[index * 3 + 0];
                            g = pPalette[index * 3 + 1];
                            b = pPalette[index * 3 + 2];
                            break;
                        }
                        pix++;
                    }

                    int xsq = v15 % 16;
                    int ysq = v15 / 16;
                    float u1 = (xsq * 32.0f) / 512.0f;
                    float u2 = (xsq * 32.0f + pData->pMetrics[v15].uWidth) / 512.0f;
                    float v1 = (ysq * 32.0f) / 512.0f;
                    float v2 = (ysq * 32.0f + pData->uFontHeight) / 512.0f;

                    render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 0, color16(r, g, b));
                    render->DrawTextNew(text_pos_x, text_pos_y, pData->pMetrics[v15].uWidth, pData->uFontHeight, u1, v1, u2, v2, 1, colorTable.Black.c16());
                }
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
    uint16_t firstColor, uint16_t secondColor, const std::string &pString,
    Image *image) {
    GUIWindow draw_window;
    draw_window.uFrameHeight = h;
    draw_window.uFrameW = uFrameY + h - 1;
    draw_window.uFrameWidth = w;
    draw_window.uFrameZ = uFrameX + w - 1;
    ui_current_text_color = firstColor;
    draw_window.uFrameX = uFrameX;
    draw_window.uFrameY = uFrameY;

    std::string work_string = FitTwoFontStringINWindow(pString, pSecondFont, &draw_window, 0, 1);
    std::istringstream stream(work_string);
    std::getline(stream, work_string);

    uint32_t *pPixels = (uint32_t*)image->GetPixels(IMAGE_FORMAT_A8B8G8R8);
    uint32_t *curr_pixel_pos = &pPixels[image->GetWidth() * uFrameY];
    if (!work_string.empty()) {
        int half_frameX = uFrameX >> 1;
        while (!stream.eof()) {
            GUIFont *currentFont = this;
            ui_current_text_color = firstColor;
            int start_str_pos = 0;
            int currentColor = firstColor;
            if (work_string[0] == '_') {
                currentFont = pSecondFont;
                currentColor = secondColor;
                ui_current_text_color = secondColor;
                start_str_pos = 1;
            }
            int line_w = (int)(w - currentFont->GetLineWidth(&work_string[start_str_pos])) / 2;
            if (line_w < 0) {
                line_w = 0;
            }
            currentFont->DrawTextLineToBuff(currentColor, &curr_pixel_pos[line_w + half_frameX],
                work_string, image->GetWidth());
            curr_pixel_pos += image->GetWidth() * (currentFont->GetHeight() - 3);
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
