#include "GUI/UI/UIGameOver.h"

#include "GUI/GUIMessageQueue.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/Time/Timer.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"

#include "Application/GameOver.h"

GUIWindow_GameOver::GUIWindow_GameOver(UIMessageType releaseEvent) : GUIWindow(WINDOW_GameOverWindow, {0, 0}, render->GetRenderDimensions()), _releaseEvent(releaseEvent) {
    pEventTimer->setPaused(true);
    prev_screen_type = current_screen_type;
    current_screen_type = SCREEN_GAMEOVER_WINDOW;
    GameOver_Setup();
    this->sHint = fmt::format(
        "{}\n \n{}\n \n{}",
        localization->GetString(LSTR_CONGRATULATIONS_ADVENTURER),
        localization->GetString(LSTR_WE_HOPE_THAT_YOUVE_ENJOYED_PLAYING_MIGHT),
        localization->GetString(LSTR_THE_MIGHT_AND_MAGIC_VII_DEVELOPMENT_TEAM));
}

void GUIWindow_GameOver::Update() {
    // draw winners certificate background
    assert(assets->winnerCert);
    render->DrawTextureNew(0, 0, assets->winnerCert);

    // draw pop up box
    if (_showPopUp) {
        GUIWindow pWindow;
        pWindow.sHint = fmt::format("{}\n \n{}", pGameOverWindow->sHint, localization->GetString(LSTR_PRESS_ESCAPE));
        pWindow.uFrameWidth = 400;
        pWindow.uFrameHeight = 100;
        pWindow.uFrameX = 120;
        pWindow.uFrameY = 140;
        pWindow.uFrameZ = 519;
        pWindow.uFrameW = 239;
        pWindow.DrawMessageBox(0);
    }
}

void GUIWindow_GameOver::Release() {
    engine->_messageQueue->addMessageCurrentFrame(_releaseEvent, 0, 0);

    current_screen_type = prev_screen_type;
    GameOverNoSound = false;
    pEventTimer->setPaused(false);

    GUIWindow::Release();
}

bool GUIWindow_GameOver::toggleAndTestFinished() {
    if (_showPopUp) return true;

    _showPopUp = true;
    return false;
}
