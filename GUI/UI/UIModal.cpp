#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Time.h"

#include "GUI/UI/UIModal.h"

#include "Platform/OSWindow.h"

enum CURRENT_SCREEN modal_window_prev_screen_type;

GUIWindow_Modal::GUIWindow_Modal(const char *message,
                                 enum UIMessageType on_release_event)
    : GUIWindow(WINDOW_ModalWindow, 0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)on_release_event, message) {
    pEventTimer->Pause();
    prev_screen_type = current_screen_type;
    current_screen_type = CURRENT_SCREEN::SCREEN_MODAL_WINDOW;
}

void GUIWindow_Modal::Update() {
    GUIWindow pWindow;
    pWindow.sHint = StringPrintf("%s\n \n%s", pModalWindow->sHint.c_str(),
                                 localization->GetString(61));
    pWindow.uFrameWidth = 400;
    pWindow.uFrameHeight = 100;
    pWindow.uFrameX = 120;
    pWindow.uFrameY = 140;
    pWindow.uFrameZ = 519;
    pWindow.uFrameW = 239;
    pWindow.DrawMessageBox(0);
}

void GUIWindow_Modal::Release() {
    pMessageQueue_50CBD0->AddGUIMessage((UIMessageType)par1C, 0, 0);

    current_screen_type = prev_screen_type;
    pEventTimer->Resume();

    GUIWindow::Release();
}
