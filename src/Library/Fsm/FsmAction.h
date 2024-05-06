#pragma once

#include <string>
#include <variant>

class Fsm;

class FsmActionNone {
 public:
    void execute(Fsm &fsm);
};

class FsmActionTransition {
 public:
    explicit FsmActionTransition(std::string_view transitionName);
    void execute(Fsm &fsm);

 private:
    std::string _transitionName;
};

using FsmAction = std::variant<FsmActionNone, FsmActionTransition>;
