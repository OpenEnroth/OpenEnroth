#pragma once

#include <Library/Fsm/FSMState.h>

#include <memory>

class GUICredits;

class CreditsState : public FSMState {
 public:
    CreditsState();
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;

 private:
    void _goBack();

    std::unique_ptr<GUICredits> _uiCredits;
};
