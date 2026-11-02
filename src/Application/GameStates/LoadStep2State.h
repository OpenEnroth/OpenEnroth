#pragma once

#include <Library/Fsm/FsmState.h>
#include <memory>

class GraphicsImage;

class LoadStep2State : public FsmState {
 public:
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    void _drawMM7CopyrightWindow();
    std::shared_ptr<GraphicsImage> _fullscreenTexture;
    bool _isFirstPass = false;
};
