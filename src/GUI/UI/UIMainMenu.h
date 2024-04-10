#pragma once

#include <functional>

#include "GUI/GUIWindow.h"

class UiSystem;

class GUIWindow_MainMenu : public GUIWindow {
 public:
    GUIWindow_MainMenu();
    virtual ~GUIWindow_MainMenu();

    virtual void Update() override;

    void EventLoop();

    /**
     * @brief Performs secondary initialization while showing copyright window.
     *
     * @param initFunc     Pointer to code which is executed while copyright window is showed.
     */
    static void drawCopyrightAndInit(std::function<void()> initFunc);
    static void loop(UiSystem &uiSystem);

 protected:
    static void drawMM7CopyrightWindow();

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

extern GUIWindow_MainMenu *pWindow_MainMenu;
