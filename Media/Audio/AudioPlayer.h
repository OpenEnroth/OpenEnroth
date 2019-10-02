#pragma once

#include <fstream>
#include <map>
#include <string>

#include "Engine/Strings.h"
#include "Media/Media.h"

enum SoundID {
    SOUND_Invalid = 0,
    SOUND_enter = 6,
    SOUND_WoodDoorClosing = 7,
    SOUND_fireBall = 8,
    SOUND_ClickMinus = 20,
    SOUND_ClickMovingSelector = 21,
    SOUND_ClickPlus = 23,
    SOUND_ClickSkill = 24,
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
    SOUND_bricks_down = 120,
    SOUND_bricks_up = 121,
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
    SOUND_luteguitar = 133,
    SOUND_panflute = 134,
    SOUND_trumpet = 135,
    SOUND_gold01 = 200,
    SOUND_heal = 202,
    SOUND_fizzle = 203,
    SOUND_TurnPageU = 204,
    SOUND_batlleen = 206,
    SOUND_batllest = 207,
    SOUND_openchest0101 = 208,
    SOUND_spellfail0201 = 0xD1,
    SOUND_drink = 0xD2,
    SOUND_eat = 211,
    SOUND_gong = 0xD7,
    SOUND_hurp = 0xD9,
    SOUND_church = 218,
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

enum MusicID { MUSIC_MainMenu = 14, MUSIC_Credits = 15 };

class AudioPlayer {
 protected:
    typedef struct SoundHeader {
        size_t uFileOffset;
        size_t uCompressedSize;
        size_t uDecompressedSize;
    } SoundHeader;

 public:
    AudioPlayer() : bPlayerReady(false), currentMusicTrack(0) {}
    virtual ~AudioPlayer() {}

    void Initialize();

    void SetMasterVolume(int level);

    void MusicPlayTrack(enum MusicID eTrack);
    void MusicStart();
    void MusicStop();
    void MusicPause();
    void MusicResume();
    void SetMusicVolume(int music_level);
    float MusicGetVolume();

    void StopAll(int sample_id);
    void PlaySound(SoundID eSoundID, int pid, unsigned int uNumRepeats, int x, int y, int a7);
    void UpdateSounds();
    void StopChannels(int uStartChannel, int uEndChannel);
    void LoadAudioSnd();
    void MessWithChannels();
    bool FindSound(const std::string &pName, struct SoundHeader *header);
    PMemBuffer LoadSound(const std::string &pSoundName);
    PMemBuffer LoadSound(int uSoundID);
    void PlaySpellSound(unsigned int spell, unsigned int pid);

 protected:
    bool bPlayerReady;
    int currentMusicTrack;
    float uMasterVolume;
    PAudioTrack pCurrentMusicTrack;
    std::ifstream fAudioSnd;
    std::map<String, SoundHeader> mSoundHeaders;
};

struct SoundDesc;

class SoundList {
 public:
    inline SoundList() {}

    void Initialize();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
};

struct SoundData;

extern int sLastTrackLengthMS;
extern AudioPlayer *pAudioPlayer;
extern SoundList *pSoundList;

extern std::array<float, 10> pSoundVolumeLevels;

void PlayLevelMusic();
