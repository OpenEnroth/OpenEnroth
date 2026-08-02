#pragma once

#include <vector>

#include "GUI/GUIWindow.h"

class GUIWindow_SaveLoad : public GUIWindow {
 public:
    GUIWindow_SaveLoad(WindowType type, Pointi position, Sizei dimensions);
    virtual ~GUIWindow_SaveLoad();

    [[nodiscard]] int selectedSlot() const {
        return _selectedSlot;
    }

    void setSelectedSlot(int slot) {
        _selectedSlot = slot;
    }

    [[nodiscard]] int scrollPosition() const {
        return _scrollPosition;
    }

    /** Handles a click on one of the visible slot rows. First click selects the slot, second click acts on it. */
    virtual void slotClicked(int slotIndex) = 0;

    void scrollUp();
    void scrollDown();
    void scrollWithMouse();

 protected:
    /** @return Savegame slot indices shown in this menu, in display order. */
    [[nodiscard]] virtual const std::vector<int> &menuSlots() const = 0;

    void drawSaveLoad();

 protected:
    int _selectedSlot = 0;
    int _scrollPosition = 0;
};

class GUIWindow_Save : public GUIWindow_SaveLoad {
 public:
    GUIWindow_Save();

    virtual void Update() override;

    virtual void slotClicked(int slotIndex) override;

 protected:
    [[nodiscard]] virtual const std::vector<int> &menuSlots() const override;

    GraphicsImage *saveload_ui_save_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_saveu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};

class GUIWindow_Load : public GUIWindow_SaveLoad {
 public:
    explicit GUIWindow_Load(bool ingame);

    virtual void Update() override;

    virtual void slotClicked(int slotIndex) override;
    void loadButtonPressed();
    void downArrowPressed();
    void upArrowPressed();
    void cancelButtonPressed();
    void quickLoad();

 protected:
    [[nodiscard]] virtual const std::vector<int> &menuSlots() const override;

    bool _loadSlotClicked = false;
    GraphicsImage *main_menu_background = nullptr;

    GraphicsImage *saveload_ui_load_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_loadu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};
