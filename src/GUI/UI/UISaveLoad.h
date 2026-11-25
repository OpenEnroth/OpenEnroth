#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIWindow.h"

// MM8's in-game Save/Load background bakes a 10-row visible list (vs. MM7's 7) -
// shared between UISaveLoad.cpp's drawing code and the click-routing math, both of which need to agree on the
// same visible-row count.
inline constexpr int kMm8SaveLoadVisibleSlotsInGame = 10;

struct SavegameSlot {
    std::string fileName;
    SaveGameHeader header;
    std::unique_ptr<GraphicsImage> thumbnail;
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

    [[nodiscard]] int selectedSlotIndex() const {
        return _selectedSlot;
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
     * Handles a click on one of the visible slot rows. A single click selects the slot, and a double click on
     * the selected slot acts on it - loading it in the load menu, renaming it in the save menu.
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
    int _visibleSlots = 7; // Rows shown at once, MM8's in-game layout shows more.
    int _scrollTrackTop = 216; // Scroll bar offset from the top of the dialog.
    int _scrollTrackHeight = 107;
    bool _mm8IngameLayout = false; // MM8's in-game save & load screens share one full-screen layout.
};

class GUIWindow_Save : public GUIWindow_SaveLoad {
 public:
    GUIWindow_Save();

    virtual void Update() override;

    virtual void slotClicked(int slotIndex, bool isDoubleClick) override;

 protected:
    std::shared_ptr<GraphicsImage> saveload_ui_save_up;
    std::shared_ptr<GraphicsImage> saveload_ui_loadsave;
    std::shared_ptr<GraphicsImage> saveload_ui_saveu;
    std::shared_ptr<GraphicsImage> saveload_ui_x_u;
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
    bool ingame = false;
    std::shared_ptr<GraphicsImage> main_menu_background;

    std::shared_ptr<GraphicsImage> saveload_ui_load_up;
    std::shared_ptr<GraphicsImage> saveload_ui_loadsave;
    std::shared_ptr<GraphicsImage> saveload_ui_loadu;
    std::shared_ptr<GraphicsImage> saveload_ui_x_u;
};

/**
 * @return                      Currently open save/load menu. Should only be called when one is open.
 */
GUIWindow_SaveLoad *saveLoadMenu();
