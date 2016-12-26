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

void DrawCharToBuff(unsigned short* uXpos,unsigned char* pCharPixels, int uCharWidth, int uCharHeight, unsigned __int16* pFontPalette, __int16 draw_color, int line_width);


//----- (0044C448) --------------------------------------------------------
GUIFont *LoadFont(const char *pFontFile, const char *pFontPalette, ...)
{
    int pallete_index; // eax@3
    GUIFont *pFont;
    unsigned int palletes_count = 0;
    va_list palettes_ptr;

    pFont = (GUIFont *)pIcons_LOD->LoadRaw(pFontFile, 0);
    va_start(palettes_ptr, pFontFile);

    while (NULL != (pFontPalette = va_arg(palettes_ptr, const char *)))
    {
        pallete_index = pIcons_LOD->LoadTexture(pFontPalette, TEXTURE_16BIT_PALETTE);
        if (pallete_index == -1)
            Error("Unable to open %s", pFontPalette);

        pFont->pFontPalettes[palletes_count] = pIcons_LOD->pTextures[pallete_index].pPalette16;
        ++palletes_count;
    }
    va_end(palettes_ptr);
    pFont->palletes_count = palletes_count;
    return pFont;
}

//----- (0044D2FD) --------------------------------------------------------
void GUIFont::_44D2FD_prolly_draw_credits_entry( GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h, 
                                                 unsigned __int16 firstColor, unsigned __int16 secondColor, const char *pString, 
                                                 unsigned __int16 *pPixels, unsigned int uPixelsWidth )
    {
  char *work_string; // eax@1
  unsigned __int16 *curr_pixel_pos; // esi@1
  GUIFont *currentFont; // edi@4
  signed int start_str_pos; // ecx@4
  signed int line_w; // eax@6
  GUIWindow draw_window; // [sp+Ch] [bp-5Ch]@
  int currentColor; // [sp+74h] [bp+Ch]@4
  int half_frameX; // [sp+80h] [bp+18h]@2

  draw_window.uFrameHeight = h;
  draw_window.uFrameW = uFrameY + h - 1;
  draw_window.uFrameWidth = w;
  draw_window.uFrameZ = uFrameX + w - 1;
  ui_current_text_color = firstColor;
  draw_window.uFrameX = uFrameX;
  draw_window.uFrameY = uFrameY;

  work_string = GUIFont::FitTwoFontStringINWindow(pString, this, pSecondFont, &draw_window, 0, 1);
  work_string = strtok(work_string, "\n");
  curr_pixel_pos = &pPixels[uPixelsWidth * uFrameY];
  if ( work_string )
  {
    half_frameX = uFrameX >> 1;
    while ( 1 )
    {
      currentFont = this;
      ui_current_text_color = firstColor;
      start_str_pos = 0;
      currentColor = firstColor;
      if ( *work_string == '_' )
      {
        currentFont = pSecondFont;
        currentColor = secondColor;
        ui_current_text_color = secondColor;
        start_str_pos = 1;
      }
      line_w = (signed int)(w - currentFont->GetLineWidth(&work_string[start_str_pos]))/2;
      if ( line_w < 0 )
        line_w = 0;
      currentFont->DrawTextLineToBuff(currentColor, secondColor, &curr_pixel_pos[line_w + half_frameX], work_string, uPixelsWidth);
      curr_pixel_pos += uPixelsWidth * (currentFont->uFontHeight - 3);
      work_string = strtok(0, "\n");
      if ( !work_string )
        break;
    }
  }
}

//----- (0044D1E7) --------------------------------------------------------
void GUIFont::DrawTextLine( unsigned int uDefaultColor, signed int uX, signed int uY, 
                            const char *text, int max_len_pix )
{
    signed int uX_pos; // edi@3
    unsigned char c; // cl@4
    unsigned __int16 draw_color; // cx@12
    unsigned __int8 *pCharPixels; // eax@12
    char color_code[20]; // [sp+Ch] [bp-1Ch]@16
    int text_length; // [sp+20h] [bp-8h]@2
    int text_color; // [sp+24h] [bp-4h]@1
    int uCharWidth; // [sp+30h] [bp+8h]@9

    if ( !text )
        return;
    text_color = ui_current_text_color;
    text_length = strlen(text);
    uX_pos=uX;
    for (int i=0; i<text_length; ++i )
        {
        c = text[i];
        if ( IsCharValid(c) )
            {
            switch (c)
                {
            case '\n':	//Line Feed 0A 10:
                return;
                break;
            case '\f':  //Form Feed, page eject  0C 12 
                strncpy(color_code, &text[i + 1], 5);
                color_code[5] = 0;
                text_color = atoi(color_code);
                ui_current_text_color = text_color;
                i += 5;	  
                break;
            case '\t':	// Horizontal tab 09
            case '\r':   //Carriage Return 0D 13                 
                break;
            default:
                uCharWidth = pMetrics[c].uWidth;
                if ( uCharWidth )
                    {
                    if ( i > 0 )
                        uX_pos += pMetrics[c].uLeftSpacing;
                    draw_color = text_color;
                    pCharPixels = &pFontData[font_pixels_offset[c]];
                    if ( !text_color )
                        draw_color = -1;
                    render->DrawText(uX_pos, uY, pCharPixels, uCharWidth, uFontHeight, pFontPalettes[0], draw_color, 0);
                    uX_pos += uCharWidth;
                    if ( i < text_length )
                        uX_pos += pMetrics[c].uRightSpacing;
                    }
                }
            }
        }
    
}

//----- (0040F845) --------------------------------------------------------
void DrawCharToBuff( unsigned short* uXpos,unsigned char* pCharPixels, int uCharWidth, int uCharHeight, 
                            unsigned __int16* pFontPalette, __int16 draw_color, int line_width )
    {
    unsigned __int16* draw_buff; // edi@1
    unsigned char* pPixels; // esi@1
    unsigned char char_pxl; // eax@3

    draw_buff = uXpos;
    pPixels = pCharPixels;
    for(int i=0; i<uCharHeight; ++i)
        {
        for(int j=0; j<uCharWidth; ++j)
            {
            char_pxl = *pPixels++;
            if ( char_pxl )
                {
                if ( char_pxl == 1 )
                    *draw_buff = pFontPalette[1];     
                else         
                    *draw_buff = draw_color;         
                }
            ++draw_buff;
            }
        draw_buff+=line_width-uCharWidth;
        }

}

//----- (0044D0B5) --------------------------------------------------------
void GUIFont::DrawTextLineToBuff( int uColor, int a3, unsigned short* uX_buff_pos, const char *text, int line_width )
    {
  
  unsigned short* uX_pos; // edi@3
  unsigned char c; // cl@4
  unsigned __int16 draw_color; // cx@12
  unsigned __int8 *pCharPixels; // eax@12
  char color_code[20]; // [sp+Ch] [bp-1Ch]@16
  int text_length; // [sp+20h] [bp-8h]@2
  int text_color; // [sp+24h] [bp-4h]@1
  int uCharWidth; // [sp+30h] [bp+8h]@9

  if ( !text )
      return;
  text_color = ui_current_text_color;
  text_length = strlen(text);
  uX_pos=uX_buff_pos;
  for (int i=0; i<text_length; ++i )
      {
      c = text[i];
      if ( IsCharValid(c) )
          {
          switch (c)
              {
          case '\n':	//Line Feed 0A 10:
              return;
              break;
          case '\f':  //Form Feed, page eject  0C 12 
              strncpy(color_code, &text[i + 1], 5);
              color_code[5] = 0;
              text_color = atoi(color_code);
              ui_current_text_color = text_color;
              i += 5;	  
              break;
          case '\t':	// Horizontal tab 09
          case '_':                   
              break;
          default:
              uCharWidth = pMetrics[c].uWidth;
              if ( uCharWidth )
                  {
                  if ( i > 0 )
                      uX_pos += pMetrics[c].uLeftSpacing;
                  draw_color = text_color;
                  pCharPixels = &pFontData[font_pixels_offset[c]];
                  if ( !text_color )
                      draw_color = -1;
                  DrawCharToBuff(uX_pos, pCharPixels, uCharWidth, uFontHeight, pFontPalettes[0], draw_color, line_width);
                  uX_pos += uCharWidth;
                  if ( i < text_length )
                      uX_pos += pMetrics[c].uRightSpacing;
                  }
              }
          }
      }
}



//----- (0044C933) --------------------------------------------------------
char * GUIFont::FitTwoFontStringINWindow(const char *pString, GUIFont *pFontMain, GUIFont *pFontSecond, GUIWindow* pWindow, int startPixlOff, int a6)
{

    GUIFont *currentFont; // esi@3
    unsigned char c;
    int uInStrLen;
    char digits[4];
    int possible_transition_point;
    int string_pixel_Width;
    int start_pixel_offset;

    if (!pString)
    {
        Log::Warning(L"Invalid string passed !");
        return 0;
    }
    currentFont = pFontMain; // esi@3
    uInStrLen = strlen(pString);
    Assert(uInStrLen < sizeof(pTmpBuf3));
    strcpy(pTmpBuf3.data(), pString);
    if (uInStrLen == 0)
        return pTmpBuf3.data();

    start_pixel_offset = string_pixel_Width = startPixlOff;
    possible_transition_point = 0;
    for (int i = 0; i < uInStrLen; ++i)
    {
        c = pTmpBuf3[i];
        if (pFontMain->IsCharValid(c))
        {
            switch (c)
            {
            case '\t':	// Horizontal tab 09
            {
                strncpy(digits, &pTmpBuf3[i + 1], 3);
                digits[3] = 0;
                string_pixel_Width = atoi(digits) + startPixlOff;
                i += 3;
                break;
            }
            case  '\n':	//Line Feed 0A 10
            {
                string_pixel_Width = start_pixel_offset;
                possible_transition_point = i;
                currentFont = pFontMain;
                break;
            }
            case  '\f':   //Form Feed, page eject  0C 12
            {
                i += 5;
                break;
            }
            case  '\r':   //Carriage Return 0D 13
            {
                if (!a6)
                    return (char*)pString;
                break;
            }
            case ' ':
            {
                string_pixel_Width += currentFont->pMetrics[c].uWidth;
                possible_transition_point = i;
                break;
            }
            case '_':
                currentFont = pFontSecond;
                break;
            default:

                if ((string_pixel_Width + currentFont->pMetrics[c].uWidth + currentFont->pMetrics[c].uLeftSpacing +
                    currentFont->pMetrics[c].uRightSpacing) < pWindow->uFrameWidth)
                {
                    if (i > possible_transition_point)
                        string_pixel_Width += currentFont->pMetrics[c].uLeftSpacing;
                    string_pixel_Width += currentFont->pMetrics[c].uWidth;
                    if (i < uInStrLen)
                        string_pixel_Width += currentFont->pMetrics[c].uRightSpacing;
                }
                else
                {
                    pTmpBuf3[possible_transition_point] = '\n';

                    if (currentFont == pFontSecond)
                    {

                        for (int k = uInStrLen - 1; k >= possible_transition_point + 1; --k)
                            pTmpBuf3[k] = pTmpBuf3[k - 1];

                        ++uInStrLen;
                        ++possible_transition_point;
                        pTmpBuf3[possible_transition_point] = '_';

                    }
                    string_pixel_Width = start_pixel_offset;

                    for (int j = possible_transition_point; j < i; ++j)
                    {
                        c = pTmpBuf3[j];
                        if (pFontMain->IsCharValid(c))
                        {
                            if (j > possible_transition_point)
                                string_pixel_Width += pFontMain->pMetrics[c].uLeftSpacing;
                            string_pixel_Width += pFontMain->pMetrics[c].uWidth;
                            if (j < i)
                                string_pixel_Width += pFontMain->pMetrics[c].uRightSpacing;

                        }
                    }
                }
            }
        }
    }
    return pTmpBuf3.data();
}


//----- (0044C6C2) --------------------------------------------------------
char *GUIFont::GetPageTop( const char *pInString, GUIWindow *pWindow, unsigned int uX, int a5 )
{
  int text_height; // edi@1
  char *text_str; // ebx@3
  unsigned char c; // cl@4
  int text_length; 

  text_height = 0;

  if ( !pInString )
    return 0;
  text_str = FitTextInAWindow(pInString, this, pWindow, uX);
  text_length = strlen(text_str);
  for ( int i = 0; i < text_length; ++i )
  {
    c = text_str[i];
    if ( IsCharValid(c) )
    {
      switch (c)
      {
        case '\n':	//Line Feed 0A 10:
          text_height = text_height + (uFontHeight - 3);
          if ( text_height >= (signed int)(a5 * (pWindow->uFrameHeight - (uFontHeight - 3))) )
            return &text_str[i];
          break;
        case '\f':  //Form Feed, page eject  0C 12
          i += 5;
          break;
        case '\t':	// Horizontal tab 09
        case '\r':   //Carriage Return 0D 13 
          i += 3;
          break;
      }
      if ( text_height >= (signed int)(a5 * pWindow->uFrameHeight) )
        break;
    }
  }
  return &text_str[0];
}

//----- (0044C62E) --------------------------------------------------------
int GUIFont::GetStringHeight2( GUIFont *secondFont, const char *text_str, GUIWindow* pWindow, int startX, int a6 )
    {
 
  int uAllHeght; 
  int uStringLen; 
  unsigned char c; 
  char *test_string; 

  if ( !text_str )
    return 0;
  uAllHeght = uFontHeight - 3;
  test_string = FitTwoFontStringINWindow(text_str, this, secondFont, pWindow, startX, 0);
  uStringLen = strlen(test_string);
  for (int i = 0; i < uStringLen; ++i)
      {
      c = test_string[i];
      if (IsCharValid(c))
          {
          switch (c)
              {
          case '\n':	//Line Feed 0A 10:
              uAllHeght+= uFontHeight - 3;
              break;
          case '\f':  //Form Feed, page eject  0C 12 
              i += 5;		  
              break;
          case '\t':	// Horizontal tab 09
          case '\r':   //Carriage Return 0D 13
              if (a6 != 1)
                  i += 3;
              break;
              }
          }
      }

  return uAllHeght;
}

int GUIFont::CalcTextHeight(const String &str, struct GUIWindow *window, int x_offset, bool return_on_carriage)
{
    return this->CalcTextHeight(str.c_str(), window, x_offset, return_on_carriage);
}

//----- (0044C59D) --------------------------------------------------------
int GUIFont::CalcTextHeight(const char *pString, struct GUIWindow *pWindow, int uXOffset, bool return_on_carriage)
{
    int uAllHeght;
    int uStringLen;
    unsigned char c;
    char *test_string;

    if (!pString)
        return 0;
    uAllHeght = uFontHeight - 6;
    test_string = FitTextInAWindow(pString, this, pWindow, uXOffset);
    uStringLen = strlen(pString);
    for (int i = 0; i < uStringLen; ++i)
    {
        c = test_string[i];
        if (IsCharValid(c))
        {
            switch (c)
            {
            case '\n':	//Line Feed 0A 10:
                uAllHeght += uFontHeight - 3;
                break;
            case '\f':  //Form Feed, page eject  0C 12 
                i += 5;
                break;
            case '\t':	// Horizontal tab 09
            case '\r':   //Carriage Return 0D 13
                if (!return_on_carriage)
                    i += 3;
                break;
            }
        }
    }
    return uAllHeght;
}

int GUIFont::GetLineWidth(const String &str)
{
    return this->GetLineWidth(str.c_str());
}

//----- (0044C51E) --------------------------------------------------------
int GUIFont::GetLineWidth(const char *pString)
{
    int str_len; // ebp@3
    int string_line_width; // esi@3
    unsigned char c;

    if (!pString)
        return 0;
    str_len = strlen(pString);
    string_line_width = 0;
    for (int i = 0; i < str_len; ++i)
    {
        c = pString[i];
        if (IsCharValid(c))
        {
            switch (c)
            {
            case '\t':
            case '\n':
            case '\r':
                return string_line_width;
            case '\f':
                i += 5;
                break;
            default:
                if (i > 0)
                    string_line_width += pMetrics[c].uLeftSpacing;
                string_line_width += pMetrics[c].uWidth;
                if (i < str_len)
                    string_line_width += pMetrics[c].uRightSpacing;
            }
        }
    }
    return string_line_width;
}


int GUIFont::AlignText_Center(unsigned int center_x, const String &str)
{
    return this->AlignText_Center(center_x, str.c_str());
}

//----- (0044C502) --------------------------------------------------------
int GUIFont::AlignText_Center(unsigned int uCenterX, const char *pString)
{
    signed int position; // esi@1

    position = (signed int)(uCenterX - GetLineWidth(pString)) >> 1;
    if (position >= 0)
        return position;
    else
        return  0;
}


String FitTextInAWindow(String &str, GUIFont *font, GUIWindow *window, int x, bool return_on_carriage)
{
    auto result = FitTextInAWindow(str.c_str(), font, window, x, return_on_carriage);
    return String(result);
}

//----- (0044C768) --------------------------------------------------------
char *FitTextInAWindow(const char *pInString, GUIFont *pFont, GUIWindow *pWindow, int uX, bool return_on_carriage)
{
    unsigned char c;
    int uInStrLen;
    char digits[4];
    int possible_transition_point;
    int string_pixel_Width;
    int start_pixel_offset;

    if (!pInString)
    {
        Log::Warning(L"Invalid string passed !");
        return 0;
    }
    uInStrLen = strlen(pInString);
    strcpy(&temp_string[0], pInString);
    if (uInStrLen == 0)
        return &temp_string[0];

    start_pixel_offset = string_pixel_Width = uX;
    possible_transition_point = 0;
    for (int i = 0; i < uInStrLen; ++i)
    {
        c = temp_string[i];
        if (pFont->IsCharValid(c))
        {
            switch (c)
            {
            case '\t':	// Horizontal tab 09
            {
                strncpy(digits, &temp_string[i + 1], 3);
                digits[3] = 0;
                string_pixel_Width = atoi(digits) + uX;
                i += 3;
                break;
            }
            case  '\n':	//Line Feed 0A 10 (конец строки)
            {
                string_pixel_Width = start_pixel_offset;
                possible_transition_point = i;
                break;
            }
            case  '\f':   //Form Feed, page eject  0C 12
            {
                i += 5;
                break;
            }
            case  '\r':   //Carriage Return 0D 13
            {
                if (!return_on_carriage)
                    return (char*)pInString;
                break;
            }
            case ' '://пробел
            {
                string_pixel_Width += pFont->pMetrics[c].uWidth;
                possible_transition_point = i;
                break;
            }
            default:
                if ((string_pixel_Width + pFont->pMetrics[c].uWidth + pFont->pMetrics[c].uLeftSpacing +
                    pFont->pMetrics[c].uRightSpacing) < pWindow->uFrameWidth)//наращивание длины строки или перенос
                {
                    if (i > possible_transition_point)
                        string_pixel_Width += pFont->pMetrics[c].uLeftSpacing;
                    string_pixel_Width += pFont->pMetrics[c].uWidth;
                    if (i < uInStrLen)
                        string_pixel_Width += pFont->pMetrics[c].uRightSpacing;
                }
                else//перенос строки и слова
                {
                    temp_string[possible_transition_point] = '\n';
                    string_pixel_Width = start_pixel_offset;
                    if (i > possible_transition_point)
                    {
                        for (int j = possible_transition_point; j < i; ++j)
                        {
                            c = temp_string[j];
                            if (pFont->IsCharValid(c))
                            {
                                if (j > possible_transition_point)
                                    string_pixel_Width += pFont->pMetrics[c].uLeftSpacing;
                                string_pixel_Width += pFont->pMetrics[c].uWidth;
                                if (j < i)
                                    string_pixel_Width += pFont->pMetrics[c].uRightSpacing;
                            }
                        }
                    }
                }
            }
        }
    }
    return &temp_string[0];
}

//----- (00414162) --------------------------------------------------------
void uGameUIFontMain_initialize()
{
    uGameUIFontMain = Color16(10, 0, 0);
}

//----- (00414174) --------------------------------------------------------
void uGameUIFontShadow_initialize()
{
  uGameUIFontShadow = Color16(0xE6u, 214, 193);
}