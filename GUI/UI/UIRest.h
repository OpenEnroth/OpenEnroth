#pragma once
#include "GUI/GUIWindow.h"


struct GUIWindow_Rest : public GUIWindow
{
             GUIWindow_Rest();
    virtual ~GUIWindow_Rest() {}

    virtual void Update();
};


struct GUIWindow_RestWindow : public GUIWindow
{
    GUIWindow_RestWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
        GUIWindow(x, y, width, height, button, hint)
    {}
    virtual ~GUIWindow_RestWindow() {}

    virtual void Update();
    virtual void Release();
};


extern class Image *rest_ui_sky_frame_current;
extern class Image *rest_ui_hourglass_frame_current;