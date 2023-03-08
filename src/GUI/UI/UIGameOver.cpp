#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/Time.h"
#include "Engine/Graphics/IRender.h"

#include "GUI/UI/UIGameOver.h"
#include "Application/GameOver.h"

GUIWindow_GameOver::GUIWindow_GameOver(UIMessageType on_release_event)
    : GUIWindow(WINDOW_GameOverWindow, {0, 0}, render->GetRenderDimensions(), on_release_event) {
    pEventTimer->Pause();
    prev_screen_type = current_screen_type;
    current_screen_type = CURRENT_SCREEN::SCREEN_GAMEOVER_WINDOW;
    _tickcount = platform->tickCount() + 5000;
    Application::GameOver_Loop(0);
    this->sHint = fmt::format(
        "{}\n \n{}\n \n{}",
        localization->GetString(LSTR_CONGRATULATIONS_ADVENTURER),
        localization->GetString(LSTR_WE_HOPE_YOU_ENJOYED_MM7),
        localization->GetString(LSTR_THE_MM7_DEV_TEAM));
    bGameOverWindowCheckExit = false;
}

void GUIWindow_GameOver::Update() {
    // draw winners certificate background
    assert(assets->WinnerCert);
    render->DrawTextureNew(0, 0, assets->WinnerCert);
    // check exit timer
    if (!bGameOverWindowCheckExit)
        if (platform->tickCount() > _tickcount) bGameOverWindowCheckExit = true;
    // draw pop up box
    if (bGameOverWindowCheckExit) {
        GUIWindow pWindow;
        pWindow.Init();
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
    pCurrentFrameMessageQueue->AddGUIMessage(static_cast<UIMessageType>(wData.val), 0, 0);

    current_screen_type = prev_screen_type;
    bGameoverLoop = false;
    pEventTimer->Resume();

    GUIWindow::Release();
}
