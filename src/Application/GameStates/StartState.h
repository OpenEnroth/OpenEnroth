#pragma once

#include <Library/Fsm/FsmState.h>

class StartState : public FsmState {
 public:
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;
};
