#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIWindow.h"

struct SavegameSlot {
    std::string fileName;
    SaveGameHeader header;
    GraphicsImagePtr thumbnail;
};

class GUIWindow_SaveLoad : public GUIWindow {
 public:
    GUIWindow_SaveLoad(WindowType type, Pointi position, Sizei dimensions);
    virtual ~GUIWindow_SaveLoad();

    /**
     * @return                      Slots shown in this menu, in display order.
     */
    [[nodiscard]] const std::vector<SavegameSlot> &slots() const {
        return _slots;
    }

    /**
     * @return                      Whether a slot is selected. False only for a load menu with no saves to show.
     */
    [[nodiscard]] bool hasSelectedSlot() const {
        return _selectedSlot < std::ssize(_slots);
    }

    [[nodiscard]] const SavegameSlot &selectedSlot() const {
        assert(hasSelectedSlot());
        return _slots[_selectedSlot];
    }

    void setSelectedSlotName(std::string_view name) {
        assert(hasSelectedSlot());
        _slots[_selectedSlot].header.name = name;
    }

    /**
     * Handles a click on one of the visible slot rows. First click selects the slot, a repeat click starts
     * renaming in the save menu, and a double click loads in the load menu.
     *
     * @param slotIndex             Index of the clicked row in the visible part of the list, in [0, 7).
     * @param isDoubleClick         Whether this click is the second click of a double click.
     */
    virtual void slotClicked(int slotIndex, bool isDoubleClick) = 0;

    void scrollUp();
    void scrollDown();
    void scrollWithMouse(Pointi mousePos);

 protected:
    void preselectSlot(std::string_view fileName); // Selects & scrolls to the slot for the given file. No-op if there's no such slot.
    void drawSaveLoad();

 protected:
    std::vector<SavegameSlot> _slots;
    int _selectedSlot = 0;
    int _scrollPosition = 0;
};

class GUIWindow_Save : public GUIWindow_SaveLoad {
 public:
    GUIWindow_Save();

    virtual void Update() override;

    virtual void slotClicked(int slotIndex, bool isDoubleClick) override;

 protected:
    GraphicsImage *saveload_ui_save_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_saveu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};

class GUIWindow_Load : public GUIWindow_SaveLoad {
 public:
    explicit GUIWindow_Load(bool ingame);

    virtual void Update() override;

    virtual void slotClicked(int slotIndex, bool isDoubleClick) override;
    void loadButtonPressed();
    void downArrowPressed();
    void upArrowPressed();
    void cancelButtonPressed();
    void quickLoad();

 protected:
    GraphicsImage *main_menu_background = nullptr;

    GraphicsImage *saveload_ui_load_up = nullptr;
    GraphicsImage *saveload_ui_loadsave = nullptr;
    GraphicsImage *saveload_ui_loadu = nullptr;
    GraphicsImage *saveload_ui_x_u = nullptr;
};

/**
 * @return                      Currently open save/load menu. Should only be called when one is open.
 */
GUIWindow_SaveLoad *saveLoadMenu();
