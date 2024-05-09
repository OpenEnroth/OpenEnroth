#pragma once

#include <memory>

class Fsm;
class FsmBuilder;

class GameFsmBuilder {
 public:
    static std::unique_ptr<Fsm> buildFsm();

 private:
    static void _buildIntroVideoSequence(FsmBuilder &fsmBuilder);
};
