#pragma once

#include <string>
#include <variant>

class FSM;

class FSMActionNone {
 public:
    void execute(FSM &fsm);
};

class FSMActionTransition {
 public:
    explicit FSMActionTransition(std::string_view transitionName);
    void execute(FSM &fsm);

 private:
    std::string _transitionName;
};

using FSMAction = std::variant<FSMActionNone, FSMActionTransition>;
