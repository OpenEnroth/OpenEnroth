#pragma once

#include "GUI/GUIWindow.h"

extern struct GUIButton *pMainMenu_BtnExit;
extern struct GUIButton *pMainMenu_BtnCredits;
extern struct GUIButton *pMainMenu_BtnLoad;
extern struct GUIButton *pMainMenu_BtnNew;

class GUIWindow_MainMenu : public GUIWindow {
 public:
  GUIWindow_MainMenu();
  virtual ~GUIWindow_MainMenu() {}

  virtual void Update();
};
