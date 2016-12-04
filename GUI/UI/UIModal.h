#pragma once
#include "GUI/GUIWindow.h"

struct GUIWindow_Modal : public GUIWindow
{
             GUIWindow_Modal(const char *message, enum UIMessageType on_release_event);
    virtual ~GUIWindow_Modal() {}

    virtual void Update();
    virtual void Release();

    protected:
        enum CURRENT_SCREEN prev_screen_type;
};