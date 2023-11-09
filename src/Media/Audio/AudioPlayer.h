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
#include "Engine/Pid.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/ActorEnums.h"

#include "SoundEnums.h"

struct AudioSamplePoolEntry {
    AudioSamplePoolEntry(PAudioSample samplePtr, SoundID id, Pid pid) : samplePtr(samplePtr), id(id), pid(pid) {}

    PAudioSample samplePtr;
    SoundID id;
    Pid pid;
};

class AudioSamplePool {
 public:
    explicit AudioSamplePool(bool looping):_looping(looping) {}

    bool playNew(PAudioSample sample, PAudioDataSource source, bool positional = false);
    bool playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundID id, bool positional = false);
    bool playUniquePid(PAudioSample sample, PAudioDataSource source, Pid pid, bool positional = false);
    void pause();
    void resume();
    void stop();
    void stopSoundId(SoundID soundId);
    void stopPid(Pid pid);
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
    virtual ~AudioPlayer();

    void Initialize();
    void UpdateVolumeFromConfig();

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
     * @param mode                      Playback mode.
     * @param pid                       `Pid` of sound source.
     */
    void playSound(SoundID eSoundID, SoundPlaybackMode mode, Pid pid = Pid());

    /**
     * Play sound of spell casting or spell sprite impact.
     *
     * @param spell                     Spell ID of spell. Indexes into `SpellSoundIds`.
     * @param is_impact                 Indicates sound of spell impact, if true sound ID
     *                                  will be SpellSoundIds[spell] + 1.
     * @param mode                      Playback mode.
     * @param pid                       `Pid` of sound originator. See `playSound` description.
     */
    void playSpellSound(SpellId spell, bool is_impact, SoundPlaybackMode mode, Pid pid = Pid());

    /**
     * Play generic UI sound.
     * Generic sounds are played in non-exclusive mode - it meand that each call to this function
     * will play sound even if sound with the same ID has not finished playing.
     *
     * @param id                        ID of sound.
     */
    void playUISound(SoundID id) {
        playSound(id, SOUND_MODE_UI);
    }

    /**
     * Play sound in exclusive mode.
     * It means that if sound with the same ID has not finished playing it's playing be stopped
     * and then restarted from beginning.
     *
     * @param id                        ID of sound.
     */
    void playExclusiveSound(SoundID id) {
        playSound(id, SOUND_MODE_EXCLUSIVE);
    }

    /**
     * Play sound in non-resetable mode.
     * It means that if sound with the same ID has not finished playing, this call is effectively ignored.
     * New playing of such sound can only start when previous one has finished playing.
     *
     * @param id                        ID of sound.
     */
    void playNonResetableSound(SoundID id) {
        playSound(id, SOUND_MODE_NON_RESETTABLE);
    }

    /**
     * Play sound of party walking.
     * Semantics generally is the same as for exclusive sounds but with additional
     * tracking to stop walking sound when needed.
     *
     * @param id                        ID of sound.
     */
    void playWalkSound(SoundID id) {
        // All walk sounds originally used Pid 804 which is Pid(OBJECT_Character, 100)
        playSound(id, SOUND_MODE_WALKING);
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
        // Speech sounds originally used Pid 806 which is Pid(OBJECT_Face, 100)
        // Opening/closing sounds originally used Pid 814 which is Pid(OBJECT_Face, 101)
        playSound(id, isSpeech ? SOUND_MODE_HOUSE_SPEECH : SOUND_MODE_HOUSE_DOOR);
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

extern std::unique_ptr<AudioPlayer> pAudioPlayer;
extern SoundList *pSoundList;

void PlayLevelMusic();
