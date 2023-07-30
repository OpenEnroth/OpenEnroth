#pragma once

#include <map>
#include <unordered_set>
#include <string>
#include <list>

#include "Utility/Workaround/ToUnderlying.h"

#include "Utility/String.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileInputStream.h"
#include "Media/Media.h"
#include "Engine/Pid.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/ActorEnums.h"

#include "SoundEnums.h"

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
    AudioPlayer() : bPlayerReady(false), currentMusicTrack(MUSIC_Invalid), uMasterVolume(0), uMusicVolume(0), uVoiceVolume(0),
                    _voiceSoundPool(false), _regularSoundPool(false), _loopingSoundPool(true) {}
    virtual ~AudioPlayer() {}

    // Special PID values for additional sound playing semantics
    static const int SOUND_PID_EXCLUSIVE = PID_INVALID;
    static const int SOUND_PID_NON_RESETABLE = -2;
    static const int SOUND_PID_WALKING = -3;
    static const int SOUND_PID_MUSIC_VOLUME = -4;
    static const int SOUND_PID_VOICE_VOLUME = -5;
    static const int SOUND_PID_HOUSE_SPEECH = -6;
    static const int SOUND_PID_HOUSE_DOOR = -7;

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
    bool isWalkingSoundPlays();

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
     */
    void playSound(SoundID eSoundID, int pid);

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
        // All walk sounds originally used PID 804 which is PID(OBJECT_Character, 100)
        playSound(id, SOUND_PID_WALKING);
    }

    /**
     * Play sound of houses.
     * To avoid multiple sounds when entering/leaving repeatedly sounds needs to be stopped.
     *
     * @param id                        ID of sound.
     * @param isSpeech                  true if this is house greet/goodbye speech.
     *                                  false if this is entering/cloosing UI sound.
     */
    void playHouseSound(SoundID id, bool isSpeech) {
        // Speech sounds originally used PID 806 which is PID(OBJECT_Face, 100)
        // Opening/closing sounds originally used PID 814 which is PID(OBJECT_Face, 101)
        playSound(id, isSpeech ? SOUND_PID_HOUSE_SPEECH : SOUND_PID_HOUSE_DOOR);
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

class SoundList {
 public:
    inline SoundList() {}

    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
};

extern int sLastTrackLengthMS;
extern AudioPlayer *pAudioPlayer;
extern SoundList *pSoundList;

extern std::array<float, 10> pSoundVolumeLevels;

void PlayLevelMusic();
