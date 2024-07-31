#pragma once

#include <string_view>
#include <string>
#include <cassert>

class FsmAction {
 public:
    static FsmAction none();
    static FsmAction transition(std::string_view transitionName);

    operator bool() const;
    bool operator!() const;
    const std::string &transitionName() const;

 private:
    std::string _transitionName;
};
