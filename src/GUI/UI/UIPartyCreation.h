#pragma once

#include "GUI/GUIWindow.h"

class GraphicsImage;

bool PartyCreationUI_Loop();

class GUIWindow_PartyCreation : public GUIWindow {
 public:
    GUIWindow_PartyCreation();
    virtual ~GUIWindow_PartyCreation();

    virtual void Update();

 protected:
    GraphicsImage *main_menu_background;
};
