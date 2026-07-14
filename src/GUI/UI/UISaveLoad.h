#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Save : public GUIWindow {
 public:
    GUIWindow_Save();
    virtual ~GUIWindow_Save() {}

    virtual void Update() override;

 protected:
    // Image * main_menu_background;

    GraphicsImage *saveload_ui_save_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_saveu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};

class GUIWindow_Load : public GUIWindow {
 public:
    explicit GUIWindow_Load(bool ingame);
    virtual ~GUIWindow_Load() {}

    virtual void Update() override;

    void slotSelected(int slotIndex);
    void loadButtonPressed();
    void downArrowPressed(int maxSlots);
    void upArrowPressed();
    void cancelButtonPressed();
    void scroll(int maxSlots);
    void quickLoad();

 protected:
    bool isLoadSlotClicked = false;
    GraphicsImage *main_menu_background = nullptr;

    GraphicsImage *saveload_ui_load_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_loadu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};
