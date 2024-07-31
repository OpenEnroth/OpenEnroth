#pragma once

#include <Library/Fsm/FsmState.h>

#include <memory>

class GUIWindow_MainMenu;

class MainMenuState : public FsmState {
 public:
    MainMenuState();
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUIWindow_MainMenu> _mainMenuUI;
};
