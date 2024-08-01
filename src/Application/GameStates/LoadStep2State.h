#pragma once

#include <Library/Fsm/FsmState.h>

class GraphicsImage;

class LoadStep2State : public FsmState {
 public:
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    void _drawMM7CopyrightWindow();
    GraphicsImage *_fullscreenTexture{};
    bool _isFirstPass{};
};
