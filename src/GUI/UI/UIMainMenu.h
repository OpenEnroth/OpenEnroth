#pragma once

#include <functional>

#include "GUI/GUIWindow.h"

class GUIWindow_MainMenu : public GUIWindow {
 public:
    GUIWindow_MainMenu();
    virtual ~GUIWindow_MainMenu();

    virtual void Update() override;

    void processMessage(UIMessageType messageType);

 protected:
    GUIButton *pBtnExit;
    GUIButton *pBtnCredits;
    GUIButton *pBtnLoad;
    GUIButton *pBtnNew;

    GraphicsImage *main_menu_background;

    GraphicsImage *ui_mainmenu_new;
    GraphicsImage *ui_mainmenu_load;
    GraphicsImage *ui_mainmenu_credits;
    GraphicsImage *ui_mainmenu_exit;
};
