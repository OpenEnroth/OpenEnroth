#pragma once

#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Graphics/Overlays.h"
#include "Engine/Party.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Time/Time.h"
#include "Engine/Time/Timer.h"

#include "Utility/Memory/Blob.h"

class GraphicsImage;

// Autosave file name. Note that it's not localized, unlike the autosave title displayed in-game.
constexpr std::string_view autosaveFileName = "autosave.mm7";

struct SaveGameHeader {
    std::string name; // Save name, as displayed in the save list in-game.
    std::string locationName; // Name of the map, e.g. "out01.odm".
    Time playingTime; // Game time of the save.
};

struct SaveGame {
    SaveGameHeader header;
    Party party;
    Timer eventTimer;
    ActiveOverlayList overlays;
    std::array<NPCData, 501> npcData;
    std::array<uint16_t, 51> npcGroups;
    std::unordered_map<std::string, Blob> mapDeltas;
    Blob thumbnail;
};

struct SaveGameLite {
    SaveGameHeader header;
    Blob thumbnail;
};

struct SavegameSlot {
    std::string fileName;
    SaveGameHeader header;
    GraphicsImage *thumbnail = nullptr;
    bool isUsed = false; // True for slots backed by an actual save file.
};

/** Runtime storage for map deltas from the currently loaded save. */
extern std::unordered_map<std::string, Blob> pMapDeltas;

class SavegameList {
 public:
    SavegameList();

    /** Rescans the saves folder. Headers & thumbnails are not loaded because that means reading every save file,
     *  which the quickload path doesn't need. Menus call `loadHeaders` for that. */
    void refresh();

    /** Rescans the saves folder, loads headers & thumbnails for all saves, sorts the slots by display name &
     *  rebuilds the menu slot mappings. */
    void loadHeaders();

    void releaseThumbnails();

    void setSlotName(int slot, std::string_view name) {
        _slots[slot].header.name = name;
    }

    [[nodiscard]] const std::vector<SavegameSlot> &slots() const {
        return _slots;
    }

    [[nodiscard]] bool isSlotUsed(int slot) const {
        return slot >= 0 && slot < std::ssize(_slots) && _slots[slot].isUsed;
    }

    // Slot indices shown in the load menu - all save files.
    [[nodiscard]] const std::vector<int> &loadMenuSlots() const {
        return _loadMenuSlots;
    }

    // Slot indices shown in the save menu - the new save slot first, then the saves. Autosave & quicksaves are
    // not shown in the save menu.
    [[nodiscard]] const std::vector<int> &saveMenuSlots() const {
        return _saveMenuSlots;
    }

 private:
    // Number of save files, slots [0, saveFileCount()) are backed by them.
    [[nodiscard]] int saveFileCount() const {
        return std::ssize(_slots) - 1;
    }

 private:
    // Slots for the save files, plus an always-present trailing empty slot used by the save menu for creating a
    // new save. This way reinitializing the list mid-menu can never shrink it below what menus index.
    std::vector<SavegameSlot> _slots;

    std::vector<int> _loadMenuSlots;
    std::vector<int> _saveMenuSlots;
};

void loadGame(int uSlot);
std::pair<SaveGameHeader, Blob> createSaveData(bool resetWorld, std::string_view title);
SaveGameHeader saveGame(bool isAutoSave, bool resetWorld, std::string_view path, std::string_view title = {});
void autoSave();
void doSavegame(int uSlot);
void saveNewGame();

void quickSaveGame();
int getQuickSaveSlot();
void quickLoadGame();
std::string getCurrentQuickSave();

extern SavegameList *pSavegameList;
