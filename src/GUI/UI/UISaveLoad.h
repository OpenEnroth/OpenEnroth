#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Save : public GUIWindow {
 public:
    GUIWindow_Save();
    virtual ~GUIWindow_Save() {}

    virtual void Update() override;

 protected:
    // Image * main_menu_background;

    GraphicsImage *saveload_ui_save_up;
    GraphicsImage *saveload_ui_loadsave;
    GraphicsImage *saveload_ui_saveu;
    GraphicsImage *saveload_ui_x_u;
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
    bool isLoadSlotClicked{};
    GraphicsImage *main_menu_background{};

    GraphicsImage *saveload_ui_load_up{};
    GraphicsImage *saveload_ui_loadsave{};
    GraphicsImage *saveload_ui_loadu{};
    GraphicsImage *saveload_ui_x_u{};
};
