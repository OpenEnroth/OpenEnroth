#pragma once

#include "FSMState.h"

class NullState : public FSMState {
 public:
    virtual void enter() {}
    virtual void exit() {}
    virtual void update() {}
};
