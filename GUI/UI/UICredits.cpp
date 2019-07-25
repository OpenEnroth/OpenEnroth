#include "GUI/UI/UICredits.h"

#include <cstdint>
#include <sstream>

#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIMainMenu.h"
#include "Media/Audio/AudioPlayer.h"
#include "Platform/Api.h"
#include "Platform/OSWindow.h"

GUICredits::GUICredits() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0) {
    pFontQuick = GUIFont::LoadFont("quick.fnt", "FONTPAL", NULL);
    pFontCChar = GUIFont::LoadFont("cchar.fnt", "FONTPAL", NULL);

    mm6title = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");

    char *text = (char *)pEvents_LOD->LoadCompressedTexture("credits.txt");

    GUIWindow credit_window;
    credit_window.uFrameWidth = 250;
    credit_window.uFrameHeight = 440;
    credit_window.uFrameX = 389;
    credit_window.uFrameY = 19;

    width = 250;
    height = pFontQuick->GetStringHeight2(pFontCChar, text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
    // cred_texture = Image::Create(width, height, IMAGE_FORMAT_A8R8G8B8);
    cred_texture = render->CreateTexture_Blank(width, height, IMAGE_FORMAT_A8R8G8B8);

    // дать шрифт и цвета тексту
    pFontQuick->DrawCreditsEntry(pFontCChar, 0, credit_window.uFrameHeight, width, height,
        Color16(0x70u, 0x8Fu, 0xFEu), Color16(0xECu, 0xE6u, 0x9Cu),
        text, cred_texture);

    render->Update_Texture(cred_texture);
    free(text);

    move_Y = 0;
    tick = 0;

    CreateButton(0, 0, 0, 0, 1, 0, UIMSG_Escape, 0, 27, "");
}

GUICredits::~GUICredits() {
    pIcons_LOD->RemoveTexturesPackFromTextureList();
    mm6title->Release();
    cred_texture->Release();
}

void GUICredits::Update() {
    GUIWindow credit_window;
    credit_window.uFrameWidth = 250;
    credit_window.uFrameHeight = 440;
    credit_window.uFrameX = 389;
    credit_window.uFrameY = 19;

    render->DrawTextureNew(0, 0, mm6title);
    render->SetUIClipRect(credit_window.uFrameX, credit_window.uFrameY,
    credit_window.uFrameX + credit_window.uFrameWidth,
    credit_window.uFrameY + credit_window.uFrameHeight);
    render->DrawTextureOffset(credit_window.uFrameX, credit_window.uFrameY, 0, move_Y, cred_texture);
    render->ResetUIClipRect();
    tick++;
    if (tick == 4) {
        tick = 0;
        ++move_Y;
    }
    if (move_Y >= cred_texture->GetHeight()) {
        SetCurrentMenuID(MENU_MAIN);
    }
}

void GUICredits::EventLoop() {
    while (!pMessageQueue_50CBD0->Empty()) {
        UIMessageType pUIMessageType;
        int pParam;
        int param2;
        pMessageQueue_50CBD0->PopMessage(&pUIMessageType, &pParam, &param2);

        switch (pUIMessageType) {  // For buttons of window MainMenu
            case UIMSG_Escape:
                SetCurrentMenuID(MENU_MAIN);
                break;
            default:
                break;
        }
    }
}

void GUICredits::ExecuteCredits() {
    pMessageQueue_50CBD0->Flush();

    pAudioPlayer->MusicPlayTrack(MUSIC_Credits);

    GUICredits *pWindow_Credits = new GUICredits();
    current_screen_type = SCREEN_CREATORS;
    SetCurrentMenuID(MENU_CREDITSPROC);

    while (GetCurrentMenuID() == MENU_CREDITSPROC) {
        window->PeekMessageLoop();
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
            OS_WaitMessage();
            continue;
        }

        render->BeginScene();
        {
            pWindow_Credits->EventLoop();
            GUI_UpdateWindows();
        }
        render->EndScene();
        render->Present();
    }

    pAudioPlayer->MusicStop();
    pAudioPlayer->StopAll(1);

    pWindow_Credits->Release();
    delete pWindow_Credits;
}
