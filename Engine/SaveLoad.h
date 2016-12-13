#pragma once
#include<array>

#pragma pack(push, 1)
struct SaveFile_
	{
	char field_0[20];
	char pSaveFileName[260];
	};
#pragma pack(pop)

/*  243 */
#pragma pack(push, 1)
struct SavegameList
{
  static void Initialize(unsigned int a1);
  SavegameList() {Reset();}

  inline void Reset()
  {
    for ( int j = 0; j < 45; j++ )
    {
      for (int i = 0; i < 20; ++i)
        this->pFileList[j].field_0[i] = 0;
      for (int i = 0; i < 260; ++i)
        this->pFileList[j].pSaveFileName[i] = 0;
    }
  }
  SaveFile_ pFileList[45];
  
};
#pragma pack(pop)


/*  244 */
#pragma pack(push, 1)
struct SavegameHeader
{
  char pName[20];
  char pLocationName[20];
  unsigned __int64 uWordTime;
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