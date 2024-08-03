#pragma once

#include <array>
#include <string>

#include "Engine/Time/Time.h"

class GraphicsImage;

constexpr int MAX_SAVE_SLOTS = 45;

struct SaveGameHeader {
    std::string name; // Save name, as displayed in the save list in-game.
    std::string locationName; // Name of the map, e.g. "out01.odm".
    Time playingTime; // Game time of the save.
};

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

void LoadGame(int uSlot);
SaveGameHeader SaveGame(bool isAutoSave, bool resetWorld, std::string_view path, std::string_view title = {});
void AutoSave();
void DoSavegame(int uSlot);
bool Initialize_GamesLOD_NewLOD();
void SaveNewGame();

void QuickSaveGame();
int GetQuickSaveSlot();
void QuickLoadGame();
std::string GetCurrentQuickSave();

extern SavegameList *pSavegameList;
