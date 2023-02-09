#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    explicit GUIWindow_GameOver(UIMessageType on_release_event = UIMSG_OnGameOverWindowClose);
    virtual ~GUIWindow_GameOver() {}

    virtual void Update();
    virtual void Release();

 protected:
    CURRENT_SCREEN prev_screen_type;
    int64_t _tickcount{};
};
