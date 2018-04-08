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
#include "Game/MainMenu.h"
#include "Platform/Api.h"

bool GUICredits::ExecuteCredits() {
  GUIFont *pFontQuick = GUIFont::LoadFont("quick.fnt", "FONTPAL", NULL);
  GUIFont *pFontCChar = GUIFont::LoadFont("cchar.fnt", "FONTPAL", NULL);

  pMessageQueue_50CBD0->Flush();
  ++pIcons_LOD->uTexturePacksCount;
  if (!pIcons_LOD->uNumPrevLoadedFiles) {
    pIcons_LOD->uNumPrevLoadedFiles = pIcons_LOD->uNumLoadedFiles;
  }

  pAudioPlayer->MusicPlayTrack(MUSIC_Credits);

  Image *mm6title = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");

  char *text = (char*)pEvents_LOD->LoadRaw("credits.txt", true);

  GUIWindow credit_window;
  credit_window.uFrameWidth = 250;
  credit_window.uFrameHeight = 440;
  credit_window.uFrameX = 389;
  credit_window.uFrameY = 19;

  int width = 250;
  int height = pFontQuick->GetStringHeight2(pFontCChar, text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
  Image *cred_texture = Image::Create(width, height, IMAGE_FORMAT_A8R8G8B8);

  // дать шрифт и цвета тексту
  pFontQuick->DrawCreditsEntry(pFontCChar, 0, credit_window.uFrameHeight, width, height,
                               Color16(0x70u, 0x8Fu, 0xFEu), Color16(0xECu, 0xE6u, 0x9Cu),
                               text, cred_texture);

  GUIWindow *pWindow_Credits = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0);
  pWindow_Credits->CreateButton(0, 0, 0, 0, 1, 0, UIMSG_Escape, 0, 27, "");
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

  pAudioPlayer->MusicStop();
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
