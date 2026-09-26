#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

class GUIWindow_GameOver : public GUIWindow {
 public:
    explicit GUIWindow_GameOver(UIMessageType releaseEvent = UIMSG_OnGameOverWindowClose);
    virtual ~GUIWindow_GameOver();

    virtual void Update() override;

    bool toggleAndTestFinished();

 protected:
    UIMessageType _releaseEvent = UIMSG_0;
    bool _showPopUp = false;
    std::unique_ptr<GraphicsImage> _winnerCert;
};
