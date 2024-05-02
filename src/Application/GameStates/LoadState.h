#pragma once

#include <Library/Fsm/FSMState.h>

class LoadState : public FSMState {
 public:
    explicit LoadState();
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;
};
