#pragma once

#include <Library/Fsm/FsmState.h>

#include <memory>

class GUIWindow_Load;

class LoadSlotState : public FsmState {
 public:
    LoadSlotState();
    ~LoadSlotState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    GUIWindow_Load* _uiLoadSaveSlot;
};
