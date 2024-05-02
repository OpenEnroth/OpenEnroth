#pragma once

#include <Library/Fsm/FSM.h>

#include <memory>

class GameFSMBuilder {
 public:
    static std::unique_ptr<FSM> buildFSM();

 private:
    static void _buildIntroVideoSequence(FSM &fsm);
};