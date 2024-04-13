#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

class GraphicsImage;
class UiSystem;

bool PlayerCreation_Choose4Skills();
bool PartyCreationUI_Loop(UiSystem &uiSystem);

class GUIWindow_PartyCreation : public GUIWindow {
 public:
    GUIWindow_PartyCreation();
    virtual ~GUIWindow_PartyCreation();

    virtual void Update() override;

 protected:
    GraphicsImage *main_menu_background = nullptr;
    std::unique_ptr<GUIFont> ui_partycreation_font;
};
