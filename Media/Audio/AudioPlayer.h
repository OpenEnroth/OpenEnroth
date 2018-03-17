#pragma once
#include "Media/MediaPlayer.h"






#pragma pack(push, 1)
struct _PROVIDER {char unk_0;};
struct _SAMPLE   {char unk_0;};
//struct _STREAM   {char unk_0;};
struct _REDBOOK  {char unk_0;};
struct _DIG_DRIVER {char unk_0;};
struct _SEQUENCE {char unk_0;};
#pragma pack(pop)



/*   22 */
#pragma pack(push, 1)
struct AudioPlayer_3DSample
{
  inline AudioPlayer_3DSample()
  {
    hSample = 0;
    field_4 = 0;
    field_8 = 0;
    field_C = 0;
  }

  void *hSample;
  int field_4;
  int field_8;
  int field_C;
};
#pragma pack(pop)




/*   26 */
/*#pragma pack(push, 1)
struct SoundHeader
{
  char pSoundName[40];
  unsigned int uFileOffset;
  unsigned int uCompressedSize;
  unsigned int uDecompressedSize;
};
#pragma pack(pop)*/

/*   27 */
#pragma pack(push, 1)
struct MixerChannel
{
  inline MixerChannel():
    hSample(nullptr), source_pid(0), uSourceTrackIdx(0), uSourceTrackID(0)
  {}

  _SAMPLE *hSample;
  int source_pid;
  unsigned int uSourceTrackIdx;
  unsigned int uSourceTrackID;
};
#pragma pack(pop)





/*  308 */
enum SoundID
{
  SOUND_Invalid = 0,
  SOUND_enter = 6,
  SOUND_WoodDoorClosing = 7,
  SOUND_fireBall = 0x8,
  SOUND_ClickMinus = 0x14,//20
  SOUND_ClickMovingSelector = 0x15,//21
  SOUND_ClickPlus = 0x17,//23
  SOUND_ClickSkill = 0x18,//24
  SOUND_error = 27,
  SOUND_metal_vs_metal03h = 47,
  SOUND_48 = 0x30,
  SOUND_RunBadlands = 49,
  SOUND_RunCarpet = 50,
  SOUND_RunCooledLava = 51,
  SOUND_RunDesert = 52,
  SOUND_RunDirt = 53,
  SOUND_RunGrass = 54,
  SOUND_RunRoad = 57,
  SOUND_RunSnow = 58,
  SOUND_RunSwamp = 61,
  SOUND_RunWater = 62,
  SOUND_RunWaterIndoor = 63,
  SOUND_RunWood = 64,
  SOUND_SelectingANewCharacter = 66,
  SOUND_shoot_blaster01 = 67,
  SOUND_shoot_bow01 = 71,
  SOUND_StartMainChoice02 = 75,
  SOUND_swing_with_axe01 = 78,
  SOUND_swing_with_axe03 = 80,
  SOUND_swing_with_blunt_weapon01 = 81,
  SOUND_swing_with_blunt_weapon03 = 83,
  SOUND_swing_with_sword01 = 84,
  SOUND_swing_with_sword02 = 85,
  SOUND_WalkBadlands = 88,
  SOUND_WalkCarpet = 89,
  SOUND_WalkCooledLava = 90,
  SOUND_WalkDesert = 91,
  SOUND_WalkDirt = 92,
  SOUND_WalkGrass = 93,
  SOUND_WalkRoad = 96,
  SOUND_WalkSnow = 97,
  SOUND_WalkSwamp = 100,
  SOUND_WalkWater = 101,
  SOUND_WalkWaterIndoor = 102,
  SOUND_WalkWood = 103,
  SOUND_bricks_down = 0x78,//120
  SOUND_bricks_up = 0x79,
  SOUND_damage = 0x7A,
  SOUND_deal = 0x7B,
  SOUND_defeat = 0x7C,
  SOUND_querry_up = 0x7D,
  SOUND_querry_down = 0x7E,
  SOUND_shuffle = 0x7F,
  SOUND_title = 0x80,
  SOUND_tower_up = 0x81,
  SOUND_typing = 0x82,
  SOUND_victory = 0x83,
  SOUND_wall_up = 0x84,
  SOUND_luteguitar = 133, // 85
  SOUND_panflute = 134, // 86
  SOUND_trumpet = 135, // 87
  SOUND_gold01 = 0xC8,//200
  SOUND_heal = 202,
  SOUND_fizzle = 203,
  SOUND_TurnPageU = 204,
  SOUND_batlleen = 206,
  SOUND_batllest = 207,
  SOUND_openchest0101 = 208,
  SOUND_spellfail0201 = 0xD1,
  SOUND_drink = 0xD2,
  SOUND_eat = 211, // D3
  SOUND_gong = 0xD7,
  SOUND_hurp = 0xD9,
  SOUND_church = 0xDA, //218
  SOUND_chimes = 0xDB,
  SOUND_splash = 220,
  SOUND_star1 = 0xDD,
  SOUND_star2 = 0xDE,
  SOUND_star4 = 0xE0,
  SOUND_eradicate = 0xE1,
  SOUND_eleccircle = 0xE2,
  SOUND_encounter = 0xE3,
  SOUND_openbook = 230,
  SOUND_closebook = 231,
  SOUND_hf445a = 5788,
  SOUND_Haste = 10040,
  SOUND_21fly03 = 11090,
  SOUND_WaterWalk = 12040,
  SOUND_Stoneskin = 13040,
  SOUND_Bless = 14010,
  SOUND_Fate = 14020,
  SOUND_51heroism03 = 14060,
  SOUND_94dayofprotection03 = 17070,
  SOUND_9armageddon01 = 17080,
  SOUND_Sacrifice2 = 18060,
  SOUND_quest = 20001,
  
};


enum MusicID: unsigned __int32
{
    MUSIC_MainMenu = 14,
    MUSIC_Credits = 15
};

/*   20 */
#pragma pack(push, 1)
struct AudioPlayer
{
  //----- (004A9669) --------------------------------------------------------
  AudioPlayer():
    bPlayerReady(false), b3DSoundInitialized(false),
    hAILRedbook(nullptr), hStream(nullptr),
    h3DSoundProvider(nullptr)
  {
	/*AudioPlayer_3DSample *v0; //ecx@1
	signed int v1; //edi@1

	v0 = p3DSamples;
	v1 = 32;
	do
	{
		v0->field_4 = 0;
		v0->field_8 = 0;
		v0->field_C = 0;
		v0->hSample = 0;
		++v0;
		--v1;
	}
	while (v1);*/
    uMixerChannels = 16;
    field_2D0_time_left = 256;
    uNumRedbookTracks = 0;
    uCurrentMusicTrackLength = 0;
    field_2D4 = 0;
    s3DSoundVolume = 127;
  }
  inline ~AudioPlayer(){ Release(); };
  void SetMusicVolume(int vol);
  void SetMasterVolume(float fVolume);
  void StopAll(int sample_id);
  void PlaySound(SoundID eSoundID, signed int a3, unsigned int uNumRepeats, signed int a5, signed int a6, int a7, float uVolume, int sPlaybackRate);
  void UpdateSounds();
  void StopChannels(int uStartChannel, int uEndChannel);
  void LoadAudioSnd();//
  void Initialize();//
  void CheckA3DSupport(bool query);
  void Release();
  void FreeChannel(MixerChannel *pChannel);
  void _4ABF23(AudioPlayer_3DSample *a2);
  void SetEAXPreferences();
  void SetMapEAX();
  int _4AC0A2();
  void PlayMusicTrack(enum MusicID eTrack);
  void  MessWithChannels();


  unsigned int bEAXSupported;
  unsigned int b3DSoundInitialized;
  int s3DSoundVolume;
  struct _PROVIDER *h3DSoundProvider;
  int uNum3DSamples;
  struct AudioPlayer_3DSample p3DSamples[32];
  int field_214;
  int sRedbookVolume;
  char p3DSoundProvider[128];
  unsigned int bPlayerReady;
  //HWND hWindow;
  class OSWindow *window;
  struct _REDBOOK *hAILRedbook;
  struct _DIG_DRIVER *hDigDriver;
  int dword_0002AC;
  struct _SEQUENCE *hSequence;
  int dword_0002B4;
  struct SoundHeader *pSoundHeaders;
  FILE *hAudioSnd;
  unsigned int uNumSoundHeaders;
  unsigned int uMasterVolume;
  int dword_0002C8;
  int dword_0002CC;
  int field_2D0_time_left;
  int field_2D4;
  unsigned int uCurrentMusicTrackLength;
  unsigned int uNumRedbookTracks;
  unsigned int uCurrentMusicTrackStartMS;
  unsigned int uCurrentMusicTrackEndMS;
  struct MixerChannel pMixerChannels[16];
  int uMixerChannels;
  int field_3EC;
  char pDeviceNames[16][128];
  int pFrequency[16];
  int array_000C30[16];
  unsigned int uNumDevices;
  struct _STREAM *hStream;
  char field_C78[8];
  int cGameCDDriveLetter;
};
#pragma pack(pop)








/*  325 */
enum SOUND_DESC_TYPE : __int32
{
  SOUND_DESC_LEVEL = 0x0,
  SOUND_DESC_SYSTEM = 0x1,
  SOUND_DESC_SWAP = 0x2,
  SOUND_DESC_3 = 0x3,
  SOUND_DESC_LOCK = 0x4,
};


/*  326 */
enum SOUND_DESC_FLAGS
{
  SOUND_DESC_LOCKED = 0x1,
  SOUND_DESC_3D = 0x2,
};


#pragma pack(push, 1)
struct SoundData
{
  unsigned int uDataSize;
  char         pData[1];
};

struct SoundDesc_mm6
{
  inline bool Is3D()  {return (uFlags & SOUND_DESC_3D) != 0;}

  char pSoundName[32];
  unsigned int uSoundID;
  SOUND_DESC_TYPE eType;
  int uFlags;
  SoundData *pSoundData[17];
};

struct SoundDesc: public SoundDesc_mm6
{
  void *p3DSound;
  int bDecompressed;
};
#pragma pack(pop)



#pragma pack(push, 1)
struct SoundList
{
  inline SoundList():
    sNumSounds(0), pSL_Sounds(nullptr), uTotalLoadedSoundSize(0)
  {}

  void Initialize();
  __int16 LoadSound(int a1, unsigned int a3);
  int LoadSound(unsigned int a2, void *lpBuffer, int uBufferSizeLeft, int *pOutSoundSize, int a6);
  SoundDesc *Release();
  void _4A9D79(int a2);
  void UnloadSound(unsigned int uSoundID, char a3);
  void ToFile();
  void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
  int FromFileTxt(const char *Args);

  signed int sNumSounds;
  SoundDesc *pSL_Sounds;
  unsigned int uTotalLoadedSoundSize;
};
#pragma pack(pop)





/*  241 */
#pragma pack(push, 1)
struct Sound
{
  unsigned int uID;
  char SoundName[120];
  SoundData *pSoundData;
};
#pragma pack(pop)



extern int uFindSound_BinSearch_ResultID;
extern int uLastLoadedSoundID;
extern int sLastTrackLengthMS;
extern std::array<Sound, 3000> pSounds;
extern AudioPlayer *pAudioPlayer;
extern SoundList *pSoundList;

extern unsigned __int8 uSoundVolumeMultiplier;
extern unsigned __int8 uVoicesVolumeMultiplier;
extern unsigned __int8 uMusicVolimeMultiplier;
extern int bWalkSound; // idb

extern std::array<float, 10> pSoundVolumeLevels; // idb








/*  379 */
#pragma pack(push, 1)
struct stru339_spell_sound
{
  int AddPartySpellSound(int uSoundID, int a6);

  char pSounds[44744];
  int field_AEC8[45];
  int field_AF7C[18];
  int field_AFC4;
  int pSoundsSizes[2];
  int pSoundsOffsets[2];
};
#pragma pack(pop)
extern std::array<stru339_spell_sound, 4> stru_A750F8;
extern std::array<stru339_spell_sound, 4> AA1058_PartyQuickSpellSound;

struct SoundHeader *FindSound_BinSearch(unsigned int uStart, unsigned int uEnd, const char *pName);
struct SoundData *LoadSound(const char *pSoundName, struct SoundData *pOutBuff, unsigned int uID);
int sub_4AB66C(int, int);
int GetSoundStrengthByDistanceFromParty(int x, int y, int z);
void PlayLevelMusic();
