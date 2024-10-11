#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    explicit GUIWindow_GameOver(UIMessageType releaseEvent = UIMSG_OnGameOverWindowClose);
    virtual ~GUIWindow_GameOver() {}

    virtual void Update() override;
    virtual void Release() override;

    bool toggleAndTestFinished();

 protected:
    UIMessageType _releaseEvent = UIMSG_0;
    bool _showPopUp = false;
};
