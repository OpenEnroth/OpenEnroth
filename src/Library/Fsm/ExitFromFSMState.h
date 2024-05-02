#pragma once

#include "FSMState.h"

class ExitFromFSMState : public FSMState {
 public:
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;
};
