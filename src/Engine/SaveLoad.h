#pragma once

#include <array>
#include <unordered_map>
#include <string>
#include <utility>

#include "Engine/Graphics/Overlays.h"
#include "Engine/Party.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Time/Time.h"
#include "Engine/Time/Timer.h"

#include "Utility/Memory/Blob.h"

class GraphicsImage;

constexpr int MAX_SAVE_SLOTS = 45;

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

/** Runtime storage for map deltas from the currently loaded save. */
extern std::unordered_map<std::string, Blob> pMapDeltas;

struct SavegameList {
    static void Initialize();
    SavegameList();

    void Reset();

    std::array<std::string, MAX_SAVE_SLOTS> pFileList;
    std::array<bool, MAX_SAVE_SLOTS> pSavegameUsedSlots;
    std::array<SaveGameHeader, MAX_SAVE_SLOTS> pSavegameHeader;
    std::array<GraphicsImage *, MAX_SAVE_SLOTS> pSavegameThumbnails;

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
