#pragma once

#include <Library/Fsm/FsmState.h>

class StartState : public FsmState {
 public:
    StartState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;
};
