#pragma once

#include <Library/Fsm/FSMState.h>

#include <memory>

class GUIWindow_Load;

class LoadSlotState : public FSMState {
 public:
    LoadSlotState();
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;

 private:
    void _goBack();

    std::unique_ptr<GUIWindow_Load> _uiLoadSaveSlot;
};
