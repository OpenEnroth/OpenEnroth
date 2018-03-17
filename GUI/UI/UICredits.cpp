#include "gui/ui/UICredits.h"

#include <cstdint>
#include <sstream>

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
  GUIFont *pFontQuick = GUIFont::LoadFont("quick.fnt", "FONTPAL", NULL);
  GUIFont *pFontCChar = GUIFont::LoadFont("cchar.fnt", "FONTPAL", NULL);

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
  int height = GUIFont::GetStringHeight2(pFontQuick, pFontCChar, text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
  unsigned int uNumPixels = width * height * 2;
  uint16_t *pPixels = (uint16_t*)malloc(uNumPixels);
  fill_pixels_fast(Color16(0x00, 0xFF, 0xFF), pPixels, uNumPixels);

  // дать шрифт и цвета тексту
  char *pString = (char*)malloc(2 * strlen(text));
  strncpy(pString, text, strlen(text));
  pString[strlen(text)] = 0;
  GUIFont::_44D2FD_prolly_draw_credits_entry(pFontQuick, pFontCChar, 0, credit_window.uFrameHeight, width, height, Color16(0x70u, 0x8Fu, 0xFEu), Color16(0xECu, 0xE6u, 0x9Cu), pString, pPixels, width);
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
