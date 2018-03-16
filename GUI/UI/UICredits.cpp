#include "gui/ui/UICredits.h"

#include <cstdint>

#include "engine/Engine.h"
#include "engine/AssetsManager.h"
#include "engine/LOD.h"
#include "engine/Graphics/IRender.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "Media/Audio/AudioPlayer.h"
#include "Platform/OSWindow.h"
#include "lib/OpenAL/al.h"
#include "Game/MainMenu.h"

void fill_pixels_fast(uint16_t color, uint16_t *pPixels, unsigned int uNumPixels) {
  for (unsigned int i = 0; i < uNumPixels / 2; i++) {
    *pPixels++ = color;
  }
}

extern bool use_music_folder;

bool GUICredits::ExecuteCredits() {
  GUIFont *pFontQuick = LoadFont("quick.fnt", "FONTPAL", NULL);
  GUIFont *pFontCChar = LoadFont("cchar.fnt", "FONTPAL", NULL);

  pMessageQueue_50CBD0->Flush();
  ++pIcons_LOD->uTexturePacksCount;
  if (!pIcons_LOD->uNumPrevLoadedFiles) {
    pIcons_LOD->uNumPrevLoadedFiles = pIcons_LOD->uNumLoadedFiles;
  }

  if (use_music_folder) {
    PlayAudio(L"Music\\15.mp3");
  } else {
    pAudioPlayer->PlayMusicTrack(MUSIC_Credits);
  }

  Image *mm6title = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");

  char *text = (char*)pEvents_LOD->LoadRaw("credits.txt", true);

  GUIWindow credit_window;
  credit_window.uFrameWidth = 250;
  credit_window.uFrameHeight = 440;
  credit_window.uFrameX = 389;
  credit_window.uFrameY = 19;

  int width = 250;
  int height = GetStringHeight2(pFontQuick, pFontCChar, text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
  unsigned int uNumPixels = width * height * 2;
  uint16_t *pPixels = (uint16_t*)malloc(uNumPixels);
  fill_pixels_fast(Color16(0x00, 0xFF, 0xFF), pPixels, uNumPixels);

  // дать шрифт и цвета тексту
  char *pString = (char*)malloc(2 * strlen(text));
  strncpy(pString, text, strlen(text));
  pString[strlen(text)] = 0;
  DrawCreditsEntry(pFontQuick, pFontCChar, 0, credit_window.uFrameHeight, width, height,
                   Color16(0x70u, 0x8Fu, 0xFEu), Color16(0xECu, 0xE6u, 0x9Cu),
                   pString, pPixels, width);
  free(pString);
  Image *cred_texture = Image::Create(250, height, IMAGE_FORMAT_R5G6B5, pPixels);
  free(pPixels);

  GUIWindow *pWindow_Credits = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, text);
  pWindow_Credits->CreateButton(0, 0, 0, 0, 1, 0, UIMSG_Escape, 0, 27, "", 0);
  current_screen_type = SCREEN_CREATORS;
  SetCurrentMenuID(MENU_CREDITSPROC);

  int move_Y = 0;
  do {
    OS_PeekMessageLoop();
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
      OS_WaitMessage();
    } else {
      render->BeginScene();
      render->DrawTextureNew(0, 0, mm6title);
      render->SetUIClipRect(credit_window.uFrameX, credit_window.uFrameY,
                            credit_window.uFrameX + credit_window.uFrameWidth,
                            credit_window.uFrameY + credit_window.uFrameHeight);
      render->DrawTextureOffset(credit_window.uFrameX, credit_window.uFrameY, 0, move_Y, cred_texture);
      render->ResetUIClipRect();
      render->EndScene();
      ++move_Y;
      if (move_Y >= cred_texture->GetHeight()) {
        SetCurrentMenuID(MENU_MAIN);
      }
      render->Present();
      current_screen_type = SCREEN_GAME;//Ritor1: temporarily, must be corrected MainMenu_EventLoop()
      MainMenu_EventLoop();
    }
  } while (GetCurrentMenuID() == MENU_CREDITSPROC);

  if (use_music_folder) {
    alSourceStop(mSourceID);
  }
  pAudioPlayer->StopAll(1);
  free(text);
  pWindow_Credits->Release();
  pIcons_LOD->RemoveTexturesPackFromTextureList();
  if (mm6title) {
    mm6title->Release();
    mm6title = nullptr;
  }    
  cred_texture->Release();

  return true;
}

void GUICredits::DrawCreditsEntry(GUIFont *firstFont, GUIFont *pSecondFont, int uFrameX, int uFrameY, unsigned int w, unsigned int h, uint16_t firstColor, uint16_t secondColor, const char *pString, uint16_t *pPixels, unsigned int uPixelsWidth) {
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
      currentFont->DrawTextLineToBuff(currentColor, secondColor, &curr_pixel_pos[line_w + half_frameX], work_string, uPixelsWidth);
      curr_pixel_pos += uPixelsWidth * (currentFont->GetFontHeight() - 3);
      work_string = strtok(0, "\n");
      if (!work_string) {
        break;
      }
    }
  }
}

char *GUICredits::FitTwoFontStringINWindow(const char *pString, GUIFont *pFontMain, GUIFont *pFontSecond, GUIWindow* pWindow, int startPixlOff, int a6) {
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
        string_pixel_Width += currentFont->GetCharMetric(c).uWidth;
        possible_transition_point = i;
        break;
      }
      case '_':
        currentFont = pFontSecond;
        break;
      default:

        if ((string_pixel_Width + currentFont->GetCharMetric(c).uWidth + currentFont->GetCharMetric(c).uLeftSpacing + currentFont->GetCharMetric(c).uRightSpacing)<pWindow->uFrameWidth) {
          if (i>possible_transition_point)
            string_pixel_Width += currentFont->GetCharMetric(c).uLeftSpacing;
          string_pixel_Width += currentFont->GetCharMetric(c).uWidth;
          if (i<uInStrLen)
            string_pixel_Width += currentFont->GetCharMetric(c).uRightSpacing;
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
                string_pixel_Width += pFontMain->GetCharMetric(c).uLeftSpacing;
              string_pixel_Width += pFontMain->GetCharMetric(c).uWidth;
              if (j < i)
                string_pixel_Width += pFontMain->GetCharMetric(c).uRightSpacing;
            }
          }
        }
      }
    }
  }
  return pTmpBuf3.data();
}

int GUICredits::GetStringHeight2(GUIFont *firstFont, GUIFont *secondFont, const char *text_str, GUIWindow* pWindow, int startX, int a6) {
  if (!text_str)
    return 0;
  int uAllHeght = firstFont->GetFontHeight() - 3;
  char *test_string = FitTwoFontStringINWindow(text_str, firstFont, secondFont, pWindow, startX, 0);
  int uStringLen = strlen(test_string);
  for (int i = 0; i < uStringLen; ++i) {
    unsigned char c = test_string[i];
    if (firstFont->IsCharValid(c)) {
      switch (c) {
      case '\n':  // Line Feed 0A 10:
        uAllHeght += firstFont->GetFontHeight() - 3;
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
