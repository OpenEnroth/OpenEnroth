#pragma once

#include <Library/Fsm/FSMState.h>

class StartState : public FSMState {
 public:
    StartState();
    virtual FSMAction update() override;
    virtual FSMAction enter() override;
    virtual void exit() override;
};
