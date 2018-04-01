#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <io.h>
#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"

#include "MMT.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

#include "IO/Mouse.h"

#include "Game/MainMenu.h"

void MMT_MainMenu_Loop()
{
  /*GUIButton *pButton; // eax@27
  unsigned int pControlParam; // ecx@35
  unsigned int pX;
  unsigned int pY; // [sp-18h] [bp-54h]@39
  Texture_MM7 *pTexture; // [sp-14h] [bp-50h]@39
  char pContainerName[64];
  MSG msg;

  current_screen_type = SCREEN_GAME;

  pGUIWindow2 = 0;
  pAudioPlayer->StopChannels(-1, -1);

  if (!bNoSound )
    PlayAudio(L"Sounds\\New_Sounds/Stronghold_Theme.mp3");
  //if (!bNoVideo )
    //pVideoPlayer->PlayMovie(L"Anims\\New_Video/3DOLOGO.smk");

  pMouse->RemoveHoldingItem();

  pIcons_LOD->_inlined_sub2();

  //Create new window
  //WINDOW_MainMenu included in GUIWindow.h
  pWindow_MMT_MainMenu = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, 0);

  //load buttons
  //Texture_MM7* MMT_MM6      = pIcons_LOD->LoadTexturePtr("title_new", TEXTURE_16BIT_PALETTE);

  sprintf(pContainerName, "data\\New_Icons/%s", "mm6_button_oval.png");
  Texture_MM7* MMT_MM6 = LoadPNG(pContainerName);
								
  Texture_MM7* MMT_MM7      = pIcons_LOD->LoadTexturePtr("title_load", TEXTURE_16BIT_PALETTE);
  Texture_MM7* MMT_MM8      = pIcons_LOD->LoadTexturePtr("title_cred", TEXTURE_16BIT_PALETTE);
  Texture_MM7* MMT_Continue = pIcons_LOD->LoadTexturePtr("title_exit", TEXTURE_16BIT_PALETTE);
  Texture_MM7* MMT_Exit     = pIcons_LOD->LoadTexturePtr("title_exit", TEXTURE_16BIT_PALETTE);

  pMMT_MainMenu_BtnMM6      = pWindow_MMT_MainMenu->CreateButton(0,                 0,                                MMT_MM6->uTextureWidth,      MMT_MM6->uTextureHeight,      1, 0, UIMSG_MMT_MainMenu_MM6,      0, 0, "", MMT_MM6, 0);
  pMMT_MainMenu_BtnMM7      = pWindow_MMT_MainMenu->CreateButton(window->GetWidth() - (window->GetWidth()  / 4), window->GetHeight() / 4,                                MMT_MM7->uTextureWidth,      MMT_MM7->uTextureHeight,      1, 0, UIMSG_MMT_MainMenu_MM7,      1, 0, "", MMT_MM7, 0);
  pMMT_MainMenu_BtnMM8      = pWindow_MMT_MainMenu->CreateButton(window->GetWidth() - (window->GetWidth()  / 4), window->GetHeight() - ((window->GetHeight() / 4) + 50), MMT_MM8->uTextureWidth,      MMT_MM8->uTextureHeight,      1, 0, UIMSG_MMT_MainMenu_MM8,      2, 0, "", MMT_MM8, 0);
  pMMT_MainMenu_BtnContinue = pWindow_MMT_MainMenu->CreateButton((window->GetWidth() / 4) - 100,                 window->GetHeight() - ((window->GetHeight() / 4) + 50), MMT_Continue->uTextureWidth, MMT_Continue->uTextureHeight, 1, 0, UIMSG_MMT_MainMenu_Continue, 3, 0, "", MMT_Continue, 0);
  pMMT_MainMenu_BtnExit     = pWindow_MMT_MainMenu->CreateButton(window->GetWidth() - 130,                       window->GetHeight() - 35,                               MMT_Exit->uTextureWidth,     MMT_Exit->uTextureHeight,     1, 0, UIMSG_ExitToWindows,         4, 0, "", MMT_Exit, 0);

  main_menu_background.Release();

  sprintf(pContainerName, "data\\New_Icons/%s", "MMTTITLE.pcx");
  if (main_menu_background.LoadPCXFile(pContainerName, 0) == 1)
	Error("File not found: %s", pContainerName);

  SetCurrentMenuID(MENU_MMT_MAIN_MENU);
  SetForegroundWindow(window->GetApiHandle());
  SendMessageW(window->GetApiHandle(), WM_ACTIVATEAPP, 1, 0);
  while (GetCurrentMenuID() == MENU_MMT_MAIN_MENU )
  {
    POINT cursor;
    pMouse->GetCursorPos(&cursor);

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
        Engine_DeinitializeAndTerminate(0);
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)
    {
      WaitMessage();
      continue;
    }

      render->BeginScene();
      render->DrawTextureRGB(0, 0, &main_menu_background);

      MMT_MenuMessageProc();//for ÌÌÒ menu
      GUI_UpdateWindows();

      if ( !pModalWindow )// ???
      {
          pButton = pWindow_MMT_MainMenu->pControlsHead;
          for ( pButton = pWindow_MMT_MainMenu->pControlsHead; pButton; pButton = pButton->pNext )
          {
           if ( cursor.x >= (signed int)pButton->uX && cursor.x <= (signed int)pButton->uZ
             && cursor.y >= (signed int)pButton->uY && cursor.y <= (signed int)pButton->uW )
           {
            pControlParam = pButton->msg_param;
            switch (pControlParam) // backlight for buttons
            {
              case 0:
                pTexture = MMT_MM6;
                pX = 0;
                pY = 0;
                break;
              case 1:
                pTexture = MMT_MM7;
                pX = window->GetWidth() - (window->GetWidth() / 4);
                pY = window->GetHeight() / 4;
                break;
              case 2:
                pTexture = MMT_MM8;
                pX = window->GetWidth() - (window->GetWidth() / 4);
                pY = window->GetHeight() - ((window->GetHeight() / 4) + 50);
                break;
              case 3:
                pTexture = MMT_Continue;
                pX = (window->GetWidth() / 4) - 100;
                pY = window->GetHeight() - ((window->GetHeight() / 4) + 50);
                break;
              case 4:
                pTexture = MMT_Exit;
                pX = window->GetWidth() - 130;
                pY = window->GetHeight() - 35;
                break;
            }
            render->DrawTextureTransparentColorKey(pX, pY, pTexture);
           }
          }
        }
      //}
      render->EndScene();
      render->Present();
  }
  MMT_MenuMessageProc();
  render->BeginScene();
  GUI_UpdateWindows();
  render->EndScene();
  render->Present();

  //remove resource
  if ( pGUIWindow2 )
  {
    pGUIWindow2->Release();
    pGUIWindow2 = 0;
  }
  pWindow_MMT_MainMenu->Release();
  pIcons_LOD->RemoveTexturesPackFromTextureList();*/
}

void MMT_MenuMessageProc()
{
  int pParam;
  int pParam2;
  UIMessageType pUIMessageType;

  if ( pMessageQueue_50CBD0->uNumMessages )
  {
    do
    {
      pMessageQueue_50CBD0->PopMessage(&pUIMessageType, &pParam, &pParam2);

      switch (pUIMessageType)
      {
        case UIMSG_MMT_MainMenu_MM6:
          //video
          //SetCurrentMenuID(MENU_MAIN_MM6);
		  alSourcef (mSourceID, AL_GAIN,    0.5f);
          pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);//temporarily
          break;
        case UIMSG_MMT_MainMenu_MM7: //new button for ÌÌ7
          //GUIWindow::Create(495, 172, 0, 0, WINDOW_PressedButton2, (int)pMainMenu_BtnNew, 0);
          alSourceStop(mSourceID);
          pMediaPlayer->ShowMM7IntroVideo_and_LoadingScreen();
          SetCurrentMenuID(MENU_MAIN);
          break;
        case UIMSG_MMT_MainMenu_MM8:
          //video
          //SetCurrentMenuID(MENU_MAIN_MM8);
		  alSourcei (mSourceID, AL_LOOPING,  1);
          pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);//temporarily
          break;
        case UIMSG_MMT_MainMenu_Continue:
          //video
          //SetCurrentMenuID(MENU_MAIN_Continue);
		  alSourcef (mSourceID, AL_GAIN,    1.0f);
          pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);//temporarily
          break;
        case UIMSG_ExitToWindows:
            extern GUIButton *pMainMenu_BtnExit;
            new OnButtonClick2(495, 337, 0, 0, (int)pMainMenu_BtnExit, 0);
          SetCurrentMenuID(MENU_EXIT_GAME);

        default:
          break;
      }
    }
    while ( pMessageQueue_50CBD0->uNumMessages );
  }
}
void DrawMMTCopyrightWindow()
{
  GUIWindow Dst; // [sp+8h] [bp-54h]@1

  memset(&Dst, 0, 0x54u);
  Dst.uFrameWidth = 624;
  Dst.uFrameHeight = 256;
  Dst.uFrameX = 8;
  Dst.uFrameY = 30;
  Dst.uFrameHeight = pFontSmallnum->CalcTextHeight("Text Verification: Here we can write an explanation of the project", Dst.uFrameWidth, 24)
                   + 2 * pFontSmallnum->GetHeight()
                   + 24;
  Dst.uFrameY = 470 - Dst.uFrameHeight;
  Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
  Dst.uFrameW = 469;
  //Dst.sHint = "abcagfdsgsg ljsrengvlkjesnfkjwnef";
  Dst.DrawMessageBox(0);

  Dst.uFrameWidth -= 24;
  Dst.uFrameX += 12;
  Dst.uFrameY += 12;
  Dst.uFrameHeight -= 12;
  Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
  Dst.uFrameW = Dst.uFrameY + Dst.uFrameHeight - 1;
  Dst.DrawTitleText(pFontSmallnum, 0, 0xCu, ui_mainmenu_copyright_color, "Text Verification: Here we can write an explanation of the project", 3);
}
