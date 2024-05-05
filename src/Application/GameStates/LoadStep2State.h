#pragma once

#include <Library/Fsm/FSMState.h>

class GraphicsImage;

class LoadStep2State : public FSMState {
 public:
    virtual FSMAction update() override;
    virtual FSMAction enter() override;
    virtual void exit() override;

 private:
    void _drawMM7CopyrightWindow();
    GraphicsImage *_fullscreenTexture{};
    bool _isFirstPass{};
};
