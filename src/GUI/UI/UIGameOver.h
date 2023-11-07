#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    explicit GUIWindow_GameOver(UIMessageType releaseEvent = UIMSG_OnGameOverWindowClose);
    virtual ~GUIWindow_GameOver() {}

    virtual void Update() override;
    virtual void Release() override;

 protected:
    UIMessageType _releaseEvent = UIMSG_0;
    ScreenType prev_screen_type;
    int64_t _tickcount = 0;
};
