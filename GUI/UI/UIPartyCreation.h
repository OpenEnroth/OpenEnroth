#pragma once
#include "GUI/GUIWindow.h"

bool PartyCreationUI_Loop();

struct GUIWindow_PartyCreation : public GUIWindow
{
             GUIWindow_PartyCreation();
    virtual ~GUIWindow_PartyCreation() {}

    virtual void Update();
};