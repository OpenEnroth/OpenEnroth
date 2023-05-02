#pragma once

#include <array>
#include <string>

#include "Engine/Time.h"

constexpr unsigned int MAX_SAVE_SLOTS = 45;

struct SavegameList {
    static void Initialize();
    SavegameList();

    void Reset();

    std::array<std::string, MAX_SAVE_SLOTS> pFileList;
};

struct SaveGameHeader {
    std::string pName;
    std::string pLocationName;
    GameTime playing_time;
};

void LoadGame(unsigned int uSlot);
void SaveGame(bool IsAutoSAve, bool NotSaveWorld);
void DoSavegame(unsigned int uSlot);
bool Initialize_GamesLOD_NewLOD();
void SaveNewGame();

extern int pSaveListPosition;
extern unsigned int uLoadGameUI_SelectedSlot;
extern unsigned int uNumSavegameFiles;
extern std::array<unsigned int, MAX_SAVE_SLOTS> pSavegameUsedSlots;
extern struct SavegameList *pSavegameList;
extern std::array<SaveGameHeader, MAX_SAVE_SLOTS> pSavegameHeader;

extern std::array<class Image *, MAX_SAVE_SLOTS> pSavegameThumbnails;
