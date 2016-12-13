#pragma once
#include "GUI/GUIWindow.h"

struct GUIWindow_Travel : public GUIWindow
{
    GUIWindow_Travel();
    virtual ~GUIWindow_Travel() {}

    virtual void Update();
    virtual void Release();
};


struct GUIWindow_Transition : public GUIWindow
{
    GUIWindow_Transition(uint32_t anim_id, uint32_t exit_pic_id, int x, int y, int z, int directiony, int directionx, int a8, const char *pLocationName);
    virtual ~GUIWindow_Transition() {}

    virtual void Update();
    virtual void Release();
};

extern std::string transition_button_label;