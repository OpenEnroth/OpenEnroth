#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_MainMenu : public GUIWindow {
 public:
  GUIWindow_MainMenu();
  virtual ~GUIWindow_MainMenu();

  virtual void Update();

  static void Loop();
  void EventLoop();

 protected:
  GUIButton *pBtnExit;
  GUIButton *pBtnCredits;
  GUIButton *pBtnLoad;
  GUIButton *pBtnNew;

  Image *ui_mainmenu_new;
  Image *ui_mainmenu_load;
  Image *ui_mainmenu_credits;
  Image *ui_mainmenu_exit;
};

extern class Image *main_menu_background;

extern GUIWindow_MainMenu *pWindow_MainMenu;
