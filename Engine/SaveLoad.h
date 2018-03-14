#pragma once
#include <array>

#include "Engine/Time.h"

#pragma pack(push, 1)
struct SaveFile_
{
    char field_0[20];
    char pSaveFileName[260];
};
#pragma pack(pop)

/*  243 */
struct SavegameList {
    static void Initialize();
    SavegameList();

    void Reset();

    SaveFile_ pFileList[45];
};


/*  244 */
#pragma pack(push, 1)
struct SavegameHeader
{
    char pName[20];
    char pLocationName[20];
    GameTime playing_time;// unsigned __int64 uWordTime;
    char field_30[52];
};
#pragma pack(pop)


void LoadThumbnailLloydTexture(unsigned int uSlot, unsigned int uPlayer);
void LoadGame(unsigned int uSlot);
void SaveGame(bool IsAutoSAve, bool NotSaveWorld);
void DoSavegame(unsigned int uSlot);
bool Initialize_GamesLOD_NewLOD();
void SaveNewGame();

extern unsigned int uNumSavegameFiles;
extern std::array<unsigned int, 45> pSavegameUsedSlots;
extern struct SavegameList *pSavegameList;
extern std::array<SavegameHeader, 45>  pSavegameHeader;

extern std::array<class Image *, 45> pSavegameThumbnails;