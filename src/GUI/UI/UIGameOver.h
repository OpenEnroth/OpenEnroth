#pragma once

#include <stdint.h>

#include "GUI/GUIWindow.h"
#include "GUI/GUIEnums.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    explicit GUIWindow_GameOver(UIMessageType on_release_event = UIMSG_OnGameOverWindowClose);
    virtual ~GUIWindow_GameOver() {}

    virtual void Update() override;
    virtual void Release() override;

 protected:
    ScreenType prev_screen_type;
    int64_t _tickcount{};
};
