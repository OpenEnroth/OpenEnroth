#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Save : public GUIWindow {
 public:
  GUIWindow_Save();
  virtual ~GUIWindow_Save() {}

  virtual void Update();
};

class GUIWindow_Load : public GUIWindow {
 public:
  GUIWindow_Load(bool ingame);
  virtual ~GUIWindow_Load() {}

  virtual void Update();
};

void MainMenuLoad_Loop();
void MainMenuLoad_EventLoop();

class Image;
extern Image *saveload_ui_save_up;
extern Image *saveload_ui_load_up;
extern Image *saveload_ui_loadsave;
extern Image *saveload_ui_saveu;
extern Image *saveload_ui_loadu;
extern Image *saveload_ui_x_u;
