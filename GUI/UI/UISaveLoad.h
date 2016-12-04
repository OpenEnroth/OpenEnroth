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


extern class Image *saveload_ui_save_up;
extern class Image *saveload_ui_load_up;
extern class Image *saveload_ui_loadsave;
extern class Image *saveload_ui_saveu;
extern class Image *saveload_ui_loadu;
extern class Image *saveload_ui_x_u;