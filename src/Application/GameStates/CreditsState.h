#pragma once

#include <Library/Fsm/FsmState.h>

#include <memory>

class GUICredits;

class CreditsState : public FsmState {
 public:
    CreditsState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUICredits> _uiCredits;
    bool _scheduleBackTransition = false;
};
