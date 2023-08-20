#include "GUI/UI/UICredits.h"

#include <string>

#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/GameResourceManager.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"

#include "Media/Audio/AudioPlayer.h"

GUICredits::GUICredits() :
    GUIWindow(WINDOW_Credits, {0, 0}, render->GetRenderDimensions(), 0) {
    pFontQuick = GUIFont::LoadFont("quick.fnt", "FONTPAL");
    pFontCChar = GUIFont::LoadFont("cchar.fnt", "FONTPAL");

    mm6title = assets->getImage_PCXFromIconsLOD("mm6title.pcx");

    std::string text{ engine->_gameResourceManager->getEventsFile("credits.txt").string_view() };

    GUIWindow credit_window;
    credit_window.uFrameWidth = 250;
    credit_window.uFrameHeight = 440;
    credit_window.uFrameX = 389;
    credit_window.uFrameY = 19;

    width = 250;
    height = pFontQuick->GetStringHeight2(pFontCChar.get(), text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
    cred_texture = GraphicsImage::Create(width, height);

    pFontQuick->DrawCreditsEntry(pFontCChar.get(), 0, credit_window.uFrameHeight, width, height, colorTable.CornFlowerBlue, colorTable.Primrose, text, cred_texture);

    render->Update_Texture(cred_texture);

    move_Y = 0;

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Escape);
}

GUICredits::~GUICredits() {
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

    move_Y += 0.25;

    if (move_Y >= cred_texture->height()) {
        SetCurrentMenuID(MENU_MAIN);
    }
}

void GUICredits::EventLoop() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType pUIMessageType;
        int pParam;
        int param2;
        engine->_messageQueue->popMessage(&pUIMessageType, &pParam, &param2);

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
    engine->_messageQueue->clear();

    pAudioPlayer->MusicPlayTrack(MUSIC_Credits);

    GUICredits *pWindow_Credits = new GUICredits();
    current_screen_type = SCREEN_CREATORS;
    SetCurrentMenuID(MENU_CREDITSPROC);

    while (GetCurrentMenuID() == MENU_CREDITSPROC) {
        MessageLoopWithWait();

        render->BeginScene2D();
        {
            pWindow_Credits->EventLoop();
            GUI_UpdateWindows();
        }
        render->Present();
    }

    pAudioPlayer->MusicStop();
    pAudioPlayer->stopSounds();

    pWindow_Credits->Release();
    delete pWindow_Credits;
}
