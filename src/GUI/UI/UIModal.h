#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Modal : public GUIWindow {
 public:
    GUIWindow_Modal(const char *message, UIMessageType on_release_event);
    virtual ~GUIWindow_Modal() {}

    virtual void Update();
    virtual void Release();

 protected:
    CURRENT_SCREEN prev_screen_type;
};
