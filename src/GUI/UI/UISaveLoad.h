#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Save : public GUIWindow {
 public:
    GUIWindow_Save();
    virtual ~GUIWindow_Save() {}

    virtual void Update();

 protected:
    // Image * main_menu_background;

    Image *saveload_ui_save_up;
    Image *saveload_ui_loadsave;
    Image *saveload_ui_saveu;
    Image *saveload_ui_x_u;
};

class GUIWindow_Load : public GUIWindow {
 public:
    explicit GUIWindow_Load(bool ingame);
    virtual ~GUIWindow_Load() {}

    virtual void Update();

 protected:
    Image *main_menu_background;

    Image *saveload_ui_load_up;
    Image *saveload_ui_loadsave;
    Image *saveload_ui_loadu;
    Image *saveload_ui_x_u;
};

void MainMenuLoad_Loop();
void MainMenuLoad_EventLoop();
