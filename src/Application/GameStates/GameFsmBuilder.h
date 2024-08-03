#pragma once

#include <memory>
#include <string_view>

class Fsm;
class FsmBuilder;

class GameFsmBuilder {
 public:
    static std::unique_ptr<Fsm> buildFsm(std::string_view startingState);

 private:
    static void _buildIntroVideoSequence(FsmBuilder &fsmBuilder);
    static void _buildMainMenu(FsmBuilder &fsmBuilder);
};
