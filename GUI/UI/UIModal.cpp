#include "OSWindow.h"

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Timer.h"

#include "GUI/UI/UIModal.h"


enum CURRENT_SCREEN modal_window_prev_screen_type;

GUIWindow_Modal::GUIWindow_Modal(const char *message, enum UIMessageType on_release_event) :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), (int)on_release_event, message)
{
// ----- (004141CA) --------------------------------------------------------
// void ModalWindow(const char *pStrHint, UIMessageType OnRelease_message)
// {
    pEventTimer->Pause();
    prev_screen_type = current_screen_type;
    //modal_window_prev_screen_type = current_screen_type;
    //pModalWindow = new GUIWindow_Modal(0, 0, window->GetWidth(), window->GetHeight(), OnRelease_message, pStrHint);
    current_screen_type = SCREEN_MODAL_WINDOW;
}

void GUIWindow_Modal::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     ModalWindow_ShowHint();
// }

// ----- (0041420D) --------------------------------------------------------
// void  ModalWindow_ShowHint()
// {
    GUIWindow pWindow; // [sp+4h] [bp-54h]@1

    static String label_container;
    label_container = StringPrintf("%s\n \n%s", pModalWindow->Hint, localization->GetString(61)); // Press Escape

    pWindow.Hint = label_container.c_str();
    pWindow.uFrameWidth = 400;
    pWindow.uFrameHeight = 100;
    pWindow.uFrameX = 120;
    pWindow.uFrameY = 140;
    pWindow.uFrameZ = 519;
    pWindow.uFrameW = 239;
    pWindow.DrawMessageBox(0);
}



//----- (0041426F) --------------------------------------------------------
void GUIWindow_Modal::Release()
{
    pMessageQueue_50CBD0->AddGUIMessage((UIMessageType)par1C, 0, 0);

    //pModalWindow->Release();
    //pModalWindow = nullptr;

    current_screen_type = prev_screen_type;
    pEventTimer->Resume();

    GUIWindow::Release();
}