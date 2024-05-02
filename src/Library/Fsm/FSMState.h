#pragma once

#include <string_view>

#include "FSMEventHandler.h"

class FSMTransitionHandler;

class FSMState: public FSMEventHandler {
 public:
    virtual ~FSMState() = default;
    virtual void update() = 0;
    virtual void enter() = 0;
    virtual void exit() = 0;

    void setTransitionHandler(FSMTransitionHandler *transitionHandler);

 protected:
    void executeTransition(std::string_view transition);
    void exitFromFSM();

 private:
    FSMTransitionHandler *_transitionHandler;
};
