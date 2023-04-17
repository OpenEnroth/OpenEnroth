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

/*  244 */
#pragma pack(push, 1)
struct SavegameHeader {
    char pName[20]{};
    char pLocationName[20]{};
    GameTime playing_time{};  // uint64_t uWordTime;
    char field_30[52]{};
};
#pragma pack(pop)

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
extern std::array<SavegameHeader, MAX_SAVE_SLOTS> pSavegameHeader;

extern std::array<class Image *, MAX_SAVE_SLOTS> pSavegameThumbnails;
