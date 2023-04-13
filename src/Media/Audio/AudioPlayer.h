#pragma once

#include <map>
#include <unordered_set>
#include <string>
#include <memory>
#include <list>

#include "Utility/Workaround/ToUnderlying.h"

#include "Utility/String.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileInputStream.h"
#include "Media/Media.h"
#include "Engine/MM7.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/ActorEnums.h"

enum SoundID {
    SOUND_Invalid = 0,
    SOUND_enter = 6,
    SOUND_WoodDoorClosing = 7,
    SOUND_fireBall = 8,
    SOUND_BoatCreaking = 18,
    SOUND_ClickMinus = 20,
    SOUND_ClickMovingSelector = 21,
    SOUND_ClickPlus = 23,
    SOUND_ClickSkill = 24,
    SOUND_error = 27,
    SOUND_dull_strike = 44,
    SOUND_metal_vs_metal01h = 45,
    SOUND_metal_vs_metal03h = 47,
    SOUND_48 = 48,
    SOUND_RunBadlands = 49,
    SOUND_RunCarpet = 50,
    SOUND_RunCooledLava = 51,
    SOUND_RunDesert = 52,
    SOUND_RunDirt = 53,
    SOUND_RunGrass = 54,
    SOUND_Run55_94 = 55, // sound like some ground-like surface
    SOUND_RunGround = 56,
    SOUND_RunRoad = 57,
    SOUND_RunSnow = 58,
    SOUND_Run59_98 = 59, // sound like road walk but duller
    SOUND_Run60_99 = 60, // sound like indoor stone walk with echo added
    SOUND_RunSwamp = 61,
    SOUND_RunWater = 62,
    SOUND_RunWaterIndoor = 63,
    SOUND_RunWood = 64,
    SOUND_Run65_104 = 65, // sound like wood walk but thinner
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
    SOUND_Walk55_94 = 94, // sound like some ground-like surface
    SOUND_WalkGround = 95,
    SOUND_WalkRoad = 96,
    SOUND_WalkSnow = 97,
    SOUND_Walk59_98 = 98, // sound like road walk but duller
    SOUND_Walk60_99 = 99, // sound like indoor stone walk with echo added
    SOUND_WalkSwamp = 100,
    SOUND_WalkWater = 101,
    SOUND_WalkWaterIndoor = 102,
    SOUND_WalkWood = 103,
    SOUND_Walk65_104 = 104, // sound like wood walk but thinner
    SOUND_metal_armor_strike1 = 105,
    SOUND_metal_armor_strike2 = 106,
    SOUND_metal_armor_strike3 = 107,
    SOUND_dull_armor_strike1 = 108,
    SOUND_dull_armor_strike2 = 109,
    SOUND_dull_armor_strike3 = 110,
    SOUND_bricks_down = 120,
    SOUND_bricks_up = 121,
    SOUND_damage = 122,
    SOUND_deal = 123,
    SOUND_defeat = 124,
    SOUND_querry_up = 125,
    SOUND_querry_down = 126,
    SOUND_shuffle = 127,
    SOUND_title = 128,
    SOUND_tower_up = 129,
    SOUND_typing = 130,
    SOUND_victory = 131,
    SOUND_wall_up = 132,
    SOUND_luteguitar = 133,
    SOUND_panflute = 134,
    SOUND_trumpet = 135,
    SOUND_gold01 = 200,
    SOUND_heal = 202,
    SOUND_fizzle = 203,
    SOUND_TurnPage1 = 204,
    SOUND_TurnPage2 = 205,
    SOUND_batlleen = 206,
    SOUND_batllest = 207,
    SOUND_openchest0101 = 208,
    SOUND_spellfail0201 = 209,
    SOUND_drink = 210,
    SOUND_eat = 211,
    SOUND_gong = 215,
    SOUND_hurp = 217,
    SOUND_church = 218,
    SOUND_chimes = 219,
    SOUND_splash = 220,
    SOUND_star1 = 221,
    SOUND_star2 = 222,
    SOUND_star4 = 224,
    SOUND_eradicate = 225,
    SOUND_eleccircle = 226,
    SOUND_encounter = 227,
    SOUND_openbook = 230,
    SOUND_closebook = 231,
    SOUND_teleport = 232,
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

enum MusicID {
    MUSIC_Invalid = 0,
    MUSIC_MainMenu = 14,
    MUSIC_Credits = 15
};

struct AudioSamplePoolEntry {
    AudioSamplePoolEntry(PAudioSample samplePtr_, SoundID id_, int pid_):samplePtr(samplePtr_), id(id_), pid(pid_) {}

    PAudioSample samplePtr;
    SoundID id;
    int pid;
};

class AudioSamplePool {
 public:
    explicit AudioSamplePool(bool looping):_looping(looping) {}

    bool playNew(PAudioSample sample, PAudioDataSource source, bool positional = false);
    bool playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundID id, bool positional = false);
    bool playUniquePid(PAudioSample sample, PAudioDataSource source, int pid, bool positional = false);
    void pause();
    void resume();
    void stop();
    void stopSoundId(SoundID soundId);
    void stopPid(int pid);
    void update();
    void setVolume(float value);
    bool hasPlaying();
 private:
    std::list<AudioSamplePoolEntry> _samplePool;
    bool _looping;
};


class AudioPlayer {
 protected:
    typedef struct SoundHeader {
        size_t uFileOffset;
        size_t uCompressedSize;
        size_t uDecompressedSize;
    } SoundHeader;

 public:
    AudioPlayer() : bPlayerReady(false), currentMusicTrack(MUSIC_Invalid), uMasterVolume(0), uVoiceVolume(0),
                    _voiceSoundPool(false), _regularSoundPool(false), _loopingSoundPool(true) {}
    virtual ~AudioPlayer() {}

    // Special PID values for additional sound playing semantics
    static const int SOUND_PID_EXCLUSIVE = PID_INVALID;
    static const int SOUND_PID_NON_RESETABLE = -2;
    static const int SOUND_PID_WALKING = -3;
    static const int SOUND_PID_MUSIC_VOLUME = -4;
    static const int SOUND_PID_VOICE_VOLUME = -5;

    void Initialize();

    void LoadAudioSnd();
    bool FindSound(const std::string &pName, struct AudioPlayer::SoundHeader *header);
    Blob LoadSound(const std::string &pSoundName);
    Blob LoadSound(int uSoundID);

    void SetMasterVolume(int level);
    void SetVoiceVolume(int level);
    void SetMusicVolume(int level);

    void MusicPlayTrack(MusicID eTrack);
    void MusicStart();
    void MusicStop();
    void MusicPause();
    void MusicResume();

    void UpdateSounds();
    void pauseAllSounds();
    void pauseLooping();
    void resumeSounds();
    void stopSounds();
    void stopVoiceSounds();
    void stopWalkingSounds();
    void soundDrain();

    /**
     * Play sound.
     *
     * @param eSoundID                  ID of sound.
     * @param pid                       PID of sound originator or:
     *                                  * 0 for generic ui sound, plays independently of others
     *                                  * PID_INVALID for exclusive sound - sound with the same ID will be stopped and played from start
     *                                  * SOUND_PID_NON_RESETABLE for non resetable sound - if sound still played, this call to playSound shall be ignored
     *                                  * SOUND_PID_WALKING for walking sounds, previous one will be stopped and new one started
     *                                  * SOUND_PID_MUSIC_VOLUME same as for PID_INVALID, but sound played with music volume level
     *                                  * SOUND_PID_VOICE_VOLUME same as for PID_INVALID, but sound played with voice volume level
     * @param uNumRepeats               unused but presumably must be number of repeats before stopping
     * @param x                         unused but presumably must be x coord of sound, additionally -1 seems to indicate that these coords must be ignored
     * @param y                         unused but presumably must be y coord of sound
     * @param sound_data_id             ???, unused
     */
    void playSound(SoundID eSoundID, int pid, unsigned int uNumRepeats = 0, int x = -1, int y = 0, int sound_data_id = 0);

    /**
     * Play sound of spell casting or spell sprite impact.
     *
     * @param spell                     Spell ID of spell. Indexes into `SpellSoundIds`.
     * @param pid                       PID of sound originator. See playSound description.
     * @param is_impact                 Indicates sound of spell impact, if true sound ID
     *                                  will be SpellSoundIds[spell] + 1.
     */
    void playSpellSound(SPELL_TYPE spell, unsigned int pid, bool is_impact = false);

    /**
     * Play generic UI sound.
     * Generic sounds are played in non-exclusive mode - it meand that each call to this function
     * will play sound even if sound with the same ID has not finished playing.
     *
     * @param id                        ID of sound.
     */
    void playUISound(SoundID id) {
        playSound(id, 0);
    }

    /**
     * Play sound in exclusive mode.
     * It means that if sound with the same ID has not finished playing it's playing be stopped
     * and then restarted from beginning.
     *
     * @param id                        ID of sound.
     */
    void playExclusiveSound(SoundID id) {
        playSound(id, SOUND_PID_EXCLUSIVE);
    }

    /**
     * Play sound in non-resetable mode.
     * It means that if sound with the same ID has not finished playing, this call is effectively ignored.
     * New playing of such sound can only start when previous one has finished playing.
     *
     * @param id                        ID of sound.
     */
    void playNonResetableSound(SoundID id) {
        playSound(id, SOUND_PID_NON_RESETABLE);
    }

    /**
     * Play sound of party walking.
     * Semantics generally is the same as for exclusive sounds but with additional
     * tracking to stop walking sound when needed.
     *
     * @param id                        ID of sound.
     */
    void playWalkSound(SoundID id) {
        // All walk sounds originally used PID 804 which is PID(OBJECT_Player, 100)
        playSound(id, SOUND_PID_WALKING);
    }

 protected:
    bool bPlayerReady;
    MusicID currentMusicTrack;
    float uMasterVolume;
    float uMusicVolume;
    float uVoiceVolume;
    PAudioTrack pCurrentMusicTrack;
    FileInputStream fAudioSnd;
    std::map<std::string, SoundHeader> mSoundHeaders;

    AudioSamplePool _voiceSoundPool;
    AudioSamplePool _regularSoundPool;
    AudioSamplePool _loopingSoundPool;
    PAudioSample _currentWalkingSample;
};

struct SoundDesc;

class SoundList {
 public:
    inline SoundList() {}

    void Initialize();
    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
};

extern int sLastTrackLengthMS;
extern AudioPlayer *pAudioPlayer;
extern SoundList *pSoundList;

extern std::array<float, 10> pSoundVolumeLevels;

void PlayLevelMusic();
