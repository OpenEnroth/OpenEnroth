#pragma once

#include <Library/Fsm/FsmState.h>

#include <memory>

class GUIWindow_Load;

class LoadSlotState : public FsmState {
 public:
    LoadSlotState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUIWindow_Load> _uiLoadSaveSlot;
};
