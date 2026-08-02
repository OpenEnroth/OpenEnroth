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

struct SavegameList {
    static void Initialize();
    SavegameList();

    void Reset();
    void releaseThumbnails();

    bool isSlotUsed(int slot) const {
        return slot >= 0 && slot < std::ssize(slots) && slots[slot].isUsed;
    }

    // Maps save menu positions to slot indices.
    int saveMenuSlot(int position) const {
        return saveMenuSlots[position];
    }

    // Number of positions in the save menu.
    int saveMenuSlotCount() const {
        return static_cast<int>(saveMenuSlots.size());
    }

    // Slots [0, numSavegameFiles) are the saves, slot numSavegameFiles is the always-present empty slot used by
    // the save menu for creating a new save. This way reinitializing the list mid-menu can never shrink it below
    // what menus index.
    std::vector<SavegameSlot> slots;

    // Slot indices shown in the save menu - the new save slot first, then the saves. Autosave & quicksaves are
    // not shown in the save menu.
    std::vector<int> saveMenuSlots;

    int numSavegameFiles = 0;
    int selectedSlot = 0;
    int saveListPosition = 0;
    std::string lastLoadedSave{};
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
