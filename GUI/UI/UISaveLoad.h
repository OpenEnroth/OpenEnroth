#pragma once
#include "GUI/GUIWindow.h"

struct GUIWindow_Save : public GUIWindow
{
             GUIWindow_Save();
    virtual ~GUIWindow_Save() {}

    virtual void Update();
};

struct GUIWindow_Load : public GUIWindow
{
             GUIWindow_Load(bool ingame);
    virtual ~GUIWindow_Load() {}

    virtual void Update();
};


class Image;
extern Image *saveload_ui_save_up;
extern Image *saveload_ui_load_up;
extern Image *saveload_ui_loadsave;
extern Image *saveload_ui_saveu;
extern Image *saveload_ui_loadu;
extern Image *saveload_ui_x_u;