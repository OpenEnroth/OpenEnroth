#pragma once

#include <memory>

class FSM;
class FSMBuilder;

class GameFSMBuilder {
 public:
    static std::unique_ptr<FSM> buildFSM();

 private:
    static void _buildIntroVideoSequence(FSMBuilder &fsmBuilder);
    static void _buildMainMenu(FSMBuilder &fsmBuilder);
    static void _setStartingState(FSM &fsm);
};
