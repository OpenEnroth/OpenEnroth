#pragma once

#include <Library/Fsm/FSMState.h>

#include <memory>

class GUIWindow_MainMenu;

class MainMenuState : public FSMState {
 public:
    MainMenuState();
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;

 private:
    std::unique_ptr<GUIWindow_MainMenu> _mainMenuUI;
};
