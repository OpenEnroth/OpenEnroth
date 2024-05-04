#pragma once

#include <map>
#include <string>
#include <memory>
#include <list>

#include "Engine/Pid.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/ActorEnums.h"

#include "Media/AudioTrack.h"

#include "Library/Snd/SndReader.h"

#include "Utility/String/Transformations.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileInputStream.h"

#include "SoundEnums.h"
#include "AudioSamplePool.h"
#include "SoundInfo.h"

class AudioPlayer {
 public:
    AudioPlayer() = default;
    virtual ~AudioPlayer();

    void Initialize();
    void UpdateVolumeFromConfig();

    Blob LoadSound(std::string_view pSoundName);

    void SetMasterVolume(int level);
    void SetVoiceVolume(int level);
    void SetMusicVolume(int level);

    void MusicPlayTrack(MusicId eTrack);
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
     * Returns length of sound in seconds.
     *
     * @param eSoundID                  ID of sound.
     */
    float getSoundLength(SoundId eSoundID);

    /**
     * Play sound.
     *
     * @param eSoundID                  ID of sound.
     * @param mode                      Playback mode.
     * @param pid                       `Pid` of sound source.
     */
    void playSound(SoundId eSoundID, SoundPlaybackMode mode, Pid pid = Pid());

    /**
     * Checks that data source has been populated and if not loads it into SoundInfo. Returns true on success
     *
     * @param si                        SoundInfo to be loaded
     */
    bool loadSoundDataSource(SoundInfo* si);

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
    void playUISound(SoundId id) {
        playSound(id, SOUND_MODE_UI);
    }

    /**
     * Play sound in exclusive mode.
     * It means that if sound with the same ID has not finished playing it's playing be stopped
     * and then restarted from beginning.
     *
     * @param id                        ID of sound.
     */
    void playExclusiveSound(SoundId id) {
        playSound(id, SOUND_MODE_EXCLUSIVE);
    }

    /**
     * Play sound in non-resetable mode.
     * It means that if sound with the same ID has not finished playing, this call is effectively ignored.
     * New playing of such sound can only start when previous one has finished playing.
     *
     * @param id                        ID of sound.
     */
    void playNonResetableSound(SoundId id) {
        playSound(id, SOUND_MODE_NON_RESETTABLE);
    }

    /**
     * Play sound of party walking.
     * Semantics generally is the same as for exclusive sounds but with additional
     * tracking to stop walking sound when needed.
     *
     * @param id                        ID of sound.
     */
    void playWalkSound(SoundId id) {
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
    void playHouseSound(SoundId id, bool isSpeech) {
        // Speech sounds originally used Pid 806 which is Pid(OBJECT_Face, 100)
        // Opening/closing sounds originally used Pid 814 which is Pid(OBJECT_Face, 101)
        playSound(id, isSpeech ? SOUND_MODE_HOUSE_SPEECH : SOUND_MODE_HOUSE_DOOR);
    }

 protected:
    bool bPlayerReady = false;
    MusicId currentMusicTrack = MUSIC_INVALID;
    float uMasterVolume = 0;
    float uMusicVolume = 0;
    float uVoiceVolume = 0;
    PAudioTrack pCurrentMusicTrack;

    AudioSamplePool _voiceSoundPool = AudioSamplePool(false);
    AudioSamplePool _regularSoundPool = AudioSamplePool(false);
    AudioSamplePool _loopingSoundPool = AudioSamplePool(true);
    PAudioSample _currentWalkingSample;
    SndReader _sndReader;
};

extern std::unique_ptr<AudioPlayer> pAudioPlayer;

void PlayLevelMusic();
