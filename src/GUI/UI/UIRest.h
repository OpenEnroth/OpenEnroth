#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Rest : public GUIWindow {
 public:
    GUIWindow_Rest();
    virtual ~GUIWindow_Rest() {}

    virtual void Update();
};

class GUIWindow_RestWindow : public GUIWindow {
 public:
    GUIWindow_RestWindow(Pointi position, Sizei dimensions, WindowData data, const char *hint) :
        GUIWindow(WINDOW_Rest, position, dimensions, data, hint)
    {}
    virtual ~GUIWindow_RestWindow() {}

    virtual void Update();
};

extern class Image *rest_ui_sky_frame_current;
extern class Image *rest_ui_hourglass_frame_current;
