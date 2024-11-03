#pragma once

#include <Library/Fsm/FsmState.h>
#include <GUI/UI/UISaveLoad.h>

#include <memory>

class LoadSlotState : public FsmState {
 public:
    LoadSlotState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUIWindow_Load> _uiLoadSaveSlot;
};
