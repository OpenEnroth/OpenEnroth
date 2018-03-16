#pragma once
#include "GUI/GUIWindow.h"



struct GUIWindow_Chest : public GUIWindow
{
             GUIWindow_Chest(unsigned int chest_id);
    virtual ~GUIWindow_Chest() {}

    virtual void Update();
};