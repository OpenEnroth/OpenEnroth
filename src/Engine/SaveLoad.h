#pragma once

#include <array>
#include <string>

#include "Engine/Time.h"

constexpr unsigned int MAX_SAVE_SLOTS = 45;

struct SaveGameHeader {
    std::string pName;
    std::string pLocationName;
    GameTime playing_time;
};

struct SavegameList {
    static void Initialize();
    SavegameList();

    void Reset();

    std::array<std::string, MAX_SAVE_SLOTS> pFileList;
    std::array<bool, MAX_SAVE_SLOTS> pSavegameUsedSlots;
    std::array<SaveGameHeader, MAX_SAVE_SLOTS> pSavegameHeader;
    std::array<class Image *, MAX_SAVE_SLOTS> pSavegameThumbnails;

    int numSavegameFiles = 0;
    int selectedSlot = 0;
    int saveListPosition = 0;
    std::string lastLoadedSave{};
};

void LoadGame(unsigned int uSlot);
void SaveGame(bool IsAutoSAve, bool NotSaveWorld);
void DoSavegame(unsigned int uSlot);
bool Initialize_GamesLOD_NewLOD();
void SaveNewGame();

extern struct SavegameList *pSavegameList;
