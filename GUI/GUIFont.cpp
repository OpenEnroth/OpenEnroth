#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"


extern LODFile_IconsBitmaps *pIcons_LOD;


struct GUIFont *pAutonoteFont;
struct GUIFont *pSpellFont;
struct GUIFont *pFontArrus;
struct GUIFont *pFontLucida;
struct GUIFont *pBook2Font;
struct GUIFont *pBookFont;
struct GUIFont *pFontCreate;
struct GUIFont *pFontCChar;
struct GUIFont *pFontComic;
struct GUIFont *pFontSmallnum;

char temp_string[2048];

std::array<char, 10000> pTmpBuf3;

#pragma pack(push, 1)
struct GUICharMetric {
  uint32_t uLeftSpacing;
  uint32_t uWidth;
  uint32_t uRightSpacing;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct FontData {
  uint8_t cFirstChar;  //0
  uint8_t cLastChar;  //1
  uint8_t field_2;
  uint8_t field_3;
  uint8_t field_4;
  uint16_t uFontHeight;  //5-6
  uint8_t field_7;
  uint32_t palletes_count;
  uint16_t *pFontPalettes[5];
  GUICharMetric pMetrics[256];
  uint32_t font_pixels_offset[256];
  uint8_t pFontData[0]; //array of font pixels
};
#pragma pack(pop)

GUIFont *GUIFont::LoadFont(const char *pFontFile, const char *pFontPalette, ...) {
  static_assert(sizeof(GUICharMetric) == 12, "Wrong GUICharMetric type size");
  static_assert(sizeof(FontData) == 4128, "Wrong FontData type size");

  unsigned int palletes_count = 0;
  va_list palettes_ptr;

  GUIFont *pFont = new GUIFont;
  pFont->pData = (FontData*)pIcons_LOD->LoadRaw(pFontFile, 0);
  va_start(palettes_ptr, pFontFile);

  while (NULL != (pFontPalette = va_arg(palettes_ptr, const char *))) {
    int pallete_index = pIcons_LOD->LoadTexture(pFontPalette, TEXTURE_16BIT_PALETTE);
    if (pallete_index == -1)
      Error("Unable to open %s", pFontPalette);

    pFont->pData->pFontPalettes[palletes_count] = pIcons_LOD->pTextures[pallete_index].pPalette16;
    ++palletes_count;
  }
  va_end(palettes_ptr);
  pFont->pData->palletes_count = palletes_count;
  return pFont;
}

bool GUIFont::IsCharValid(unsigned char c) const {
  return (c >= pData->cFirstChar) && (c <= pData->cLastChar) || (c == '\f') || (c == '\r') || (c == '\t') || (c == '\n');
}

unsigned int GUIFont::GetHeight() const {
  return pData->uFontHeight;
}

void GUIFont::DrawTextLine(const String &text, uint16_t uDefaultColor, int uX, int uY, int max_len_pix)
{
  if (text.empty()) {
    return;
  }

  uint16_t text_color = ui_current_text_color;
  size_t text_length = text.size();
  int uX_pos = uX;
  for (int i = 0; i<text_length; ++i) {
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
            draw_color = -1;
          }
          render->DrawText(uX_pos, uY, pCharPixels, uCharWidth, pData->uFontHeight,
                           pData->pFontPalettes[0], draw_color, 0);
          uX_pos += uCharWidth;
          if (i < text_length) {
            uX_pos += pData->pMetrics[c].uRightSpacing;
          }
        }
      }
    }
  }
}

void DrawCharToBuff(uint16_t *draw_buff, uint8_t *pCharPixels, int uCharWidth, int uCharHeight,
                    uint16_t *pFontPalette, uint16_t draw_color, int line_width)
{
  uint8_t *pPixels = pCharPixels;
  for (int y = 0; y < uCharHeight; ++y) {
    for (int x = 0; x < uCharWidth; ++x) {
      uint8_t char_pxl = *pPixels++;
      if (char_pxl) {
        *draw_buff = (char_pxl == 1) ? pFontPalette[1] : draw_color;
      }
      ++draw_buff;
    }
    draw_buff += line_width - uCharWidth;
  }
}

void GUIFont::DrawTextLineToBuff(int uColor, unsigned short* uX_buff_pos, const char *text, int line_width)
{
  unsigned short* uX_pos; // edi@3
  unsigned char c; // cl@4
  unsigned __int16 draw_color; // cx@12
  unsigned __int8 *pCharPixels; // eax@12
  char color_code[20]; // [sp+Ch] [bp-1Ch]@16
  int text_length; // [sp+20h] [bp-8h]@2
  int text_color; // [sp+24h] [bp-4h]@1
  int uCharWidth; // [sp+30h] [bp+8h]@9

  if (!text)
      return;
  text_color = ui_current_text_color;
  text_length = strlen(text);
  uX_pos=uX_buff_pos;
  for (int i=0; i<text_length; ++i ) {
      c = text[i];
      if ( IsCharValid(c) )
          {
          switch (c)
              {
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
              uCharWidth = pData->pMetrics[c].uWidth;
              if ( uCharWidth )
                  {
                  if ( i > 0 )
                      uX_pos += pData->pMetrics[c].uLeftSpacing;
                  draw_color = text_color;
                  pCharPixels = &pData->pFontData[pData->font_pixels_offset[c]];
                  if ( !text_color )
                      draw_color = -1;
                  DrawCharToBuff(uX_pos, pCharPixels, uCharWidth, pData->uFontHeight, pData->pFontPalettes[0], draw_color, line_width);
                  uX_pos += uCharWidth;
                  if ( i < text_length )
                      uX_pos += pData->pMetrics[c].uRightSpacing;
                  }
              }
          }
      }
}

String GUIFont::GetPageTop(const String &pInString, GUIWindow *pWindow, unsigned int uX, int a5)
{
  if (pInString.empty()) {
    return nullptr;
  }

  int text_height = 0;

  String text_str = FitTextInAWindow(pInString, pWindow->uFrameWidth, uX);
  int text_length = text_str.length();
  for ( int i = 0; i < text_length; ++i ) {
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

unsigned int GUIFont::CalcTextHeight(const String &pString, unsigned int width,
                                     int uXOffset, bool return_on_carriage) {
  if (pString.empty()) {
    return 0;
  }

  unsigned int uAllHeght = pData->uFontHeight - 6;
  String test_string = FitTextInAWindow(pString, width, uXOffset);
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

unsigned int GUIFont::GetLineWidth(const String &inString) {
  size_t str_len = inString.length();
  unsigned int string_line_width = 0;
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

unsigned int GUIFont::AlignText_Center(unsigned int width, const String &pString) {
  int position = ((int)width - (int)GetLineWidth(pString)) / 2;
  return (position < 0) ? 0 : position;
}

String GUIFont::FitTextInAWindow(const String &inString, unsigned int width, int uX, bool return_on_carriage) {
  size_t uInStrLen = inString.length();
  strcpy(&temp_string[0], inString.c_str());
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
      case '\t': {  // Horizontal tab 09
        char digits[4];
        strncpy(digits, &temp_string[i + 1], 3);
        digits[3] = 0;
        string_pixel_Width = atoi(digits) + uX;
        i += 3;
        break;
      }
      case  '\n': {  // Line Feed 0A 10 (конец строки)
        string_pixel_Width = start_pixel_offset;
        possible_transition_point = i;
        break;
      }
      case  '\f': {  // Form Feed, page eject  0C 12
        i += 5;
        break;
      }
      case  '\r': {  // Carriage Return 0D 13
        if (!return_on_carriage) {
          return inString;
        }
        break;
      }
      case ' ': {  // Space
        string_pixel_Width += pData->pMetrics[c].uWidth;
        possible_transition_point = i;
        break;
      }
      default:
        if ((string_pixel_Width + pData->pMetrics[c].uWidth + pData->pMetrics[c].uLeftSpacing +
          pData->pMetrics[c].uRightSpacing) < width) {  // наращивание длины строки или перенос
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

void GUIFont::DrawText(GUIWindow *pWindow, int uX, int uY, unsigned short uFontColor, const char *Str, bool present_time_transparency, int max_text_height, int uFontShadowColor) {
  int v14; // edx@9
  char Dest[6]; // [sp+Ch] [bp-2Ch]@32
  size_t v30; // [sp+2Ch] [bp-Ch]@4

  int left_margin = 0;
  if (!Str)
  {
    logger->Warning(L"Invalid string passed!");
    return;
  }
  if (!strcmp(Str, "null"))
    return;

  v30 = strlen(Str);
  if (!uX)
    uX = 12;

  String string_begin = Str;
  if (max_text_height == 0) {
    string_begin = FitTextInAWindow(Str, pWindow->uFrameWidth, uX);
  }
  auto string_end = string_begin;
  auto string_base = string_begin;

  int out_x = uX + pWindow->uFrameX;
  int out_y = uY + pWindow->uFrameY;
  v14 = 0;

  if (max_text_height != 0 && out_y + pData->uFontHeight > max_text_height)
    return;

  if ((signed int)v30 > 0)
  {
    do
    {
      unsigned char c = string_base[v14];
      if (c >= pData->cFirstChar && c <= pData->cLastChar
        || c == '\f'
        || c == '\r'
        || c == '\t'
        || c == '\n')
      {
        switch (c)
        {
        case '\t':
          strncpy(Dest, &string_base[v14 + 1], 3);
          Dest[3] = 0;
          v14 += 3;
          left_margin = atoi(Dest);
          out_x = uX + pWindow->uFrameX + left_margin;
          break;
        case '\n':
          uY = uY + pData->uFontHeight - 3;
          out_y = uY + pWindow->uFrameY;
          out_x = uX + pWindow->uFrameX + left_margin;
          if (max_text_height != 0)
          {
            if (pData->uFontHeight + out_y - 3 > max_text_height)
              return;
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
          out_y = uY + pWindow->uFrameY;
          if (max_text_height != 0)
          {
            if (pData->uFontHeight + out_y - 3 > max_text_height)
              return;
            break;
          }
          break;

        default:
          if (c == '\"' && string_base[v14 + 1] == '\"')
            ++v14;

          c = (unsigned __int8)string_base[v14];
          if (v14 > 0)
            out_x += pData->pMetrics[c].uLeftSpacing;

          unsigned char *letter_pixels = &pData->pFontData[pData->font_pixels_offset[c]];
          if (uFontColor)
            render->DrawText(out_x, out_y, letter_pixels, pData->pMetrics[c].uWidth, pData->uFontHeight,
              pData->pFontPalettes[0], uFontColor, uFontShadowColor);
          else
            render->DrawTextAlpha(out_x, out_y, letter_pixels, pData->pMetrics[c].uWidth, pData->uFontHeight,
              pData->pFontPalettes[0], present_time_transparency);

          out_x += pData->pMetrics[c].uWidth;
          if ((signed int)v14 < (signed int)v30)
            out_x += pData->pMetrics[c].uRightSpacing;
          break;
        }
      }
    } while ((signed int)++v14 < (signed int)v30);
  }
}

int GUIFont::DrawTextInRect(GUIWindow *pWindow, unsigned int uX, unsigned int uY, unsigned int uColor, String &str, int rect_width, int reverse_text)
{
  int pLineWidth; // ebx@1
  int text_width; // esi@3
  unsigned __int8 v12; // cl@7
  signed int v13; // esi@19
  signed int v14; // ebx@19
  unsigned __int8 v15; // cl@21
  unsigned int v20; // ecx@26
  unsigned char* v21; // eax@28
  int v23; // eax@34
  int v24; // ebx@36
  char Str[6]; // [sp+Ch] [bp-20h]@34
  int v28; // [sp+20h] [bp-Ch]@17
  size_t pNumLen; // [sp+28h] [bp-4h]@1
  size_t Str1a; // [sp+40h] [bp+14h]@5
  int i;

  char text[4096];
  Assert(str.length() < sizeof(text));
  strcpy(text, str.c_str());

  pNumLen = strlen(text);
  pLineWidth = this->GetLineWidth(text);
  if (pLineWidth < rect_width) {
    pWindow->DrawText(this, uX, uY, uColor, text, 0, 0, 0);
    return pLineWidth;
  }

  text_width = 0;
  if (reverse_text)
    _strrev(text);
  Str1a = 0;
  for (i = 0; i < pNumLen; ++i)
  {
    if (text_width >= rect_width)
      break;
    v12 = text[i];
    if (this->IsCharValid(v12))
    {
      switch (v12)
      {
      case '\t':// Horizontal tab 09
      case '\n': //Line Feed 0A 10
      case '\r': //Form Feed, page eject  0C 12
        break;
      case '\f': //Carriage Return 0D 13
        i += 5;
        break;
      default:
        if (i > 0)
          text_width += pData->pMetrics[v12].uLeftSpacing;
        text_width += pData->pMetrics[v12].uWidth;
        if (i < pNumLen)
          text_width += pData->pMetrics[v12].uRightSpacing;
      }
    }
  }
  text[i - 1] = 0;


  pNumLen = strlen(text);
  v28 = this->GetLineWidth(text);
  if (reverse_text)
    _strrev(text);

  v13 = uX + pWindow->uFrameX;
  v14 = uY + pWindow->uFrameY;
  for (i = 0; i<pNumLen; ++i) {
    v15 = text[i];
    if (this->IsCharValid(v15)) {
      switch (v12) {
      case '\t':// Horizontal tab 09
      {
        strncpy(Str, &text[i + 1], 3);
        Str[3] = 0;
        //   atoi(Str);
        i += 3;
        break;
      }
      case '\n': //Line Feed 0A 10
      {
        v24 = pData->uFontHeight;
        v13 = uX;
        uY = uY + pData->uFontHeight - 3;
        v14 = uY + pData->uFontHeight - 3;
        break;
      }
      case '\r': //Form Feed, page eject  0C 12
      {
        strncpy(Str, &text[i + 1], 5);
        Str[5] = 0;
        i += 5;
        uColor = atoi(Str);
        break;
      }
      case '\f': //Carriage Return 0D 13
      {
        strncpy(Str, &text[i + 1], 3);
        Str[3] = 0;
        i += 3;
        v23 = this->GetLineWidth(&text[i]);
        v13 = pWindow->uFrameZ - v23 - atoi(Str);
        v14 = uY;
        break;
      }
      default:
        v20 = pData->pMetrics[v15].uWidth;
        if (i > 0)
          v13 += pData->pMetrics[v15].uLeftSpacing;
        v21 = &pData->pFontData[pData->font_pixels_offset[v15]];
        if (uColor)
          render->DrawText(v13, v14, v21, v20, pData->uFontHeight, pData->pFontPalettes[0], uColor, 0);
        else
          render->DrawTextAlpha(v13, v14, v21, v20, pData->uFontHeight, pData->pFontPalettes[0], false);
        v13 += v20;
        if (i < (signed int)pNumLen)
          v13 += pData->pMetrics[v15].uRightSpacing;
      }
    }
  }
  return v28;
}

void GUIFont::DrawCreditsEntry(GUIFont *firstFont, GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h, uint16_t firstColor, uint16_t secondColor, const char *pString, uint16_t *pPixels, unsigned int uPixelsWidth) {
  GUIWindow draw_window;
  draw_window.uFrameHeight = h;
  draw_window.uFrameW = uFrameY + h - 1;
  draw_window.uFrameWidth = w;
  draw_window.uFrameZ = uFrameX + w - 1;
  ui_current_text_color = firstColor;
  draw_window.uFrameX = uFrameX;
  draw_window.uFrameY = uFrameY;

  char *work_string = FitTwoFontStringINWindow(pString, firstFont, pSecondFont, &draw_window, 0, 1);
  work_string = strtok(work_string, "\n");
  uint16_t *curr_pixel_pos = &pPixels[uPixelsWidth * uFrameY];
  if (work_string) {
    int half_frameX = uFrameX >> 1;
    while (true) {
      GUIFont *currentFont = firstFont;
      ui_current_text_color = firstColor;
      int start_str_pos = 0;
      int currentColor = firstColor;
      if (*work_string == '_') {
        currentFont = pSecondFont;
        currentColor = secondColor;
        ui_current_text_color = secondColor;
        start_str_pos = 1;
      }
      int line_w = (int)(w - currentFont->GetLineWidth(&work_string[start_str_pos])) / 2;
      if (line_w < 0) {
        line_w = 0;
      }
      currentFont->DrawTextLineToBuff(currentColor, &curr_pixel_pos[line_w + half_frameX], work_string, uPixelsWidth);
      curr_pixel_pos += uPixelsWidth * (currentFont->GetHeight() - 3);
      work_string = strtok(0, "\n");
      if (!work_string) {
        break;
      }
    }
  }
}

char *GUIFont::FitTwoFontStringINWindow(const char *pString, GUIFont *pFontMain, GUIFont *pFontSecond, GUIWindow* pWindow, int startPixlOff, int a6) {
  if (!pString) {
    return 0;
  }
  GUIFont *currentFont = pFontMain; // esi@3
  int uInStrLen = strlen(pString);
  Assert(uInStrLen < sizeof(pTmpBuf3));
  strcpy(pTmpBuf3.data(), pString);
  if (uInStrLen == 0) {
    return pTmpBuf3.data();
  }

  int string_pixel_Width = startPixlOff;
  int start_pixel_offset = startPixlOff;
  int possible_transition_point = 0;
  for (int i = 0; i < uInStrLen; ++i) {
    unsigned char c = pTmpBuf3[i];
    if (pFontMain->IsCharValid(c)) {
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
        currentFont = pFontMain;
        break;
      }
      case  '\f': {  //Form Feed, page eject  0C 12
        i += 5;
        break;
      }
      case  '\r': {  //Carriage Return 0D 13
        if (!a6)
          return (char*)pString;
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

        if ((string_pixel_Width + currentFont->pData->pMetrics[c].uWidth + currentFont->pData->pMetrics[c].uLeftSpacing + currentFont->pData->pMetrics[c].uRightSpacing)<pWindow->uFrameWidth) {
          if (i>possible_transition_point)
            string_pixel_Width += currentFont->pData->pMetrics[c].uLeftSpacing;
          string_pixel_Width += currentFont->pData->pMetrics[c].uWidth;
          if (i<uInStrLen)
            string_pixel_Width += currentFont->pData->pMetrics[c].uRightSpacing;
        }
        else {
          pTmpBuf3[possible_transition_point] = '\n';
          if (currentFont == pFontSecond) {
            for (int k = uInStrLen - 1; k >= possible_transition_point + 1; --k)
              pTmpBuf3[k] = pTmpBuf3[k - 1];
            ++uInStrLen;
            ++possible_transition_point;
            pTmpBuf3[possible_transition_point] = '_';
          }
          string_pixel_Width = start_pixel_offset;

          for (int j = possible_transition_point; j < i; ++j) {
            c = pTmpBuf3[j];
            if (pFontMain->IsCharValid(c)) {
              if (j>possible_transition_point)
                string_pixel_Width += pFontMain->pData->pMetrics[c].uLeftSpacing;
              string_pixel_Width += pFontMain->pData->pMetrics[c].uWidth;
              if (j < i)
                string_pixel_Width += pFontMain->pData->pMetrics[c].uRightSpacing;
            }
          }
        }
      }
    }
  }
  return pTmpBuf3.data();
}

int GUIFont::GetStringHeight2(GUIFont *firstFont, GUIFont *secondFont, const char *text_str, GUIWindow* pWindow, int startX, int a6) {
  if (!text_str)
    return 0;
  int uAllHeght = firstFont->GetHeight() - 3;
  char *test_string = FitTwoFontStringINWindow(text_str, firstFont, secondFont, pWindow, startX, 0);
  int uStringLen = strlen(test_string);
  for (int i = 0; i < uStringLen; ++i) {
    unsigned char c = test_string[i];
    if (firstFont->IsCharValid(c)) {
      switch (c) {
      case '\n':  // Line Feed 0A 10:
        uAllHeght += firstFont->GetHeight() - 3;
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
