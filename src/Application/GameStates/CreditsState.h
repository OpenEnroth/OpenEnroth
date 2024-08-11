#pragma once

#include <Library/Fsm/FsmState.h>
#include <GUI/UI/UICredits.h>

#include <memory>

class CreditsState : public FsmState {
 public:
    CreditsState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUICredits> _uiCredits;
};
