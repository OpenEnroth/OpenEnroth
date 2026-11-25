#pragma once

#include <functional>
#include <memory>

#include "GUI/GUIWindow.h"

class GUIWindow_MainMenu : public GUIWindow {
 public:
    GUIWindow_MainMenu();
    virtual ~GUIWindow_MainMenu();

    virtual void Update() override;

    void processMessage(UIMessageType messageType);

 protected:
    GUIButton *pBtnExit = nullptr;
    GUIButton *pBtnCredits = nullptr;
    GUIButton *pBtnLoad = nullptr;
    GUIButton *pBtnNew = nullptr;

    std::shared_ptr<GraphicsImage> main_menu_background;
};
