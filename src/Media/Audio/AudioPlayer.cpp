#include "AudioPlayer.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <thread>
#include <memory>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/MapInfo.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/EngineCallObserver.h"

#include "GUI/GUIWindow.h"

#include "Media/AudioBufferDataSource.h"

#include "Library/Logger/Logger.h"

#include "SoundList.h"
#include "OpenALTrack16.h"
#include "OpenALSample16.h"
#include "OpenALAudioDataSource.h"
#include "OpenALSoundProvider.h"

std::unique_ptr<AudioPlayer> pAudioPlayer;

static constexpr std::array<float, 10> pSoundVolumeLevels = {{
    0.0000000f, 0.1099999f, 0.2199999f, 0.3300000f, 0.4399999f,
    0.5500000f, 0.6600000f, 0.7699999f, 0.8799999f, 0.9700000f
}};

constexpr Pid FAKE_HOUSE_DOOR_PID = Pid::fromPacked(-7);
constexpr Pid FAKE_HOUSE_SPEECH_PID = Pid::fromPacked(-6);

extern OpenALSoundProvider *provider;

AudioPlayer::~AudioPlayer() = default;

void AudioPlayer::MusicPlayTrack(MusicId eTrack) {
    if (currentMusicTrack == eTrack) {
        return;
    }

    if (!engine->config->debug.NoSound.value() && bPlayerReady) {
        if (pCurrentMusicTrack) {
            pCurrentMusicTrack->Stop();
        }
        currentMusicTrack = MUSIC_INVALID;

        std::string file_path = fmt::format("music/{}.mp3", std::to_underlying(eTrack));
        if (!dfs->exists(file_path)) {
            logger->warning("AudioPlayer: {} not found", file_path);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(dfs->read(file_path));
        if (pCurrentMusicTrack) {
            currentMusicTrack = eTrack;

            pCurrentMusicTrack->SetVolume(uMusicVolume);
            pCurrentMusicTrack->Play();
            if (uMusicVolume == 0.0) {
                pCurrentMusicTrack->Pause();
            }
        }
    }
}

void AudioPlayer::MusicStart() {}

void AudioPlayer::MusicStop() {
    if (!pCurrentMusicTrack) {
        return;
    }

    pCurrentMusicTrack->Stop();
    pCurrentMusicTrack = nullptr;
    currentMusicTrack = MUSIC_INVALID;
}

void AudioPlayer::MusicPause() {
    if (!pCurrentMusicTrack) {
        return;
    }

    pCurrentMusicTrack->Pause();
}

void AudioPlayer::MusicResume() {
    if (!pCurrentMusicTrack) {
        return;
    }

    if (!pCurrentMusicTrack->Resume()) {
        MusicId playedMusicTrack = currentMusicTrack;
        if (currentMusicTrack != MUSIC_INVALID) {
            MusicStop();
            MusicPlayTrack(playedMusicTrack);
        }
    }
}

void AudioPlayer::SetMusicVolume(int level) {
    level = std::clamp(level, 0, 9);
    uMusicVolume = pSoundVolumeLevels[level];

    if (!pCurrentMusicTrack) {
        return;
    }

    pCurrentMusicTrack->SetVolume(uMusicVolume);
    if (level == 0) {
        MusicPause();
    } else {
        MusicResume();
    }
}

void AudioPlayer::SetMasterVolume(int level) {
    level = std::clamp(level, 0, 9);
    uMasterVolume = pSoundVolumeLevels[level];

    _regularSoundPool.setVolume(uMasterVolume);
    _loopingSoundPool.setVolume(uMasterVolume);
    if (_currentWalkingSample) {
        _currentWalkingSample->SetVolume(uMasterVolume);
    }
}

void AudioPlayer::SetVoiceVolume(int level) {
    level = std::clamp(level, 0, 9);
    uVoiceVolume = pSoundVolumeLevels[level];

    _voiceSoundPool.setVolume(uVoiceVolume);
}

void AudioPlayer::stopSounds() {
    if (!bPlayerReady) {
        return;
    }

    _voiceSoundPool.stop();
    _regularSoundPool.stop();
    _loopingSoundPool.stop();
    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::stopVoiceSounds() {
    if (!bPlayerReady) {
        return;
    }

    _voiceSoundPool.stop();
}

void AudioPlayer::stopWalkingSounds() {
    if (!bPlayerReady) {
        return;
    }

    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::resumeSounds() {
    _voiceSoundPool.resume();
    _regularSoundPool.resume();
    _loopingSoundPool.resume();
    if (_currentWalkingSample) {
        _currentWalkingSample->Resume();
    }
}

void AudioPlayer::playSound(SoundId eSoundID, SoundPlaybackMode mode, Pid pid) {
    if (!bPlayerReady)
        return;

    //logger->Info("AudioPlayer: trying to load sound id {}", eSoundID);

    // TODO(pskelton): do we need to reinstate this optimisation? dropped to allow better sound tracing
    if (/*engine->config->settings.SoundLevel.value() < 1 ||*/ eSoundID == SOUND_Invalid) {
        return;
    }

    SoundInfo *si = pSoundList->soundInfo(eSoundID);
    if (!si) {
        logger->warning("AudioPlayer: sound id {} not found", std::to_underlying(eSoundID));
        return;
    }

    //logger->Info("AudioPlayer: sound id {} found as '{}'", eSoundID, si.sName);

    if (!loadSoundDataSource(si)) return;

    PAudioSample sample = CreateAudioSample();

    SoundPlaybackResult result = SOUND_PLAYBACK_INVALID;
    sample->SetVolume(uMasterVolume);

    if (mode == SOUND_MODE_UI) {
        result = _regularSoundPool.playNew(sample, si->dataSource);
    } else if (mode == SOUND_MODE_EXCLUSIVE) {
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si->dataSource, eSoundID);
    } else if (mode == SOUND_MODE_NON_RESETTABLE) {
        result = _regularSoundPool.playUniqueSoundId(sample, si->dataSource, eSoundID);
    } else if (mode == SOUND_MODE_WALKING) {
        if (_currentWalkingSample) {
            _currentWalkingSample->Stop();
        }
        _currentWalkingSample = sample;
        _currentWalkingSample->Open(si->dataSource);
        _currentWalkingSample->Play();
    } else if (mode == SOUND_MODE_MUSIC) {
        sample->SetVolume(uMusicVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si->dataSource, eSoundID);
    } else if (mode == SOUND_MODE_SPEECH) {
        sample->SetVolume(uVoiceVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si->dataSource, eSoundID);
    } else if (mode == SOUND_MODE_HOUSE_DOOR || mode == SOUND_MODE_HOUSE_SPEECH) {
        pid = mode == SOUND_MODE_HOUSE_DOOR ? FAKE_HOUSE_DOOR_PID : FAKE_HOUSE_SPEECH_PID;
        _regularSoundPool.stopPid(pid);
        _regularSoundPool.playUniquePid(sample, si->dataSource, pid);
    } else {
        assert(pid);

        ObjectType object_type = pid.type();
        int object_id = pid.id();
        switch (object_type) {
            case OBJECT_Door: {
                assert(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
                assert((int)object_id < pIndoor->pDoors.size());

                sample->SetPosition(pIndoor->pDoors[object_id].pXOffsets[0],
                                    pIndoor->pDoors[object_id].pYOffsets[0],
                                    pIndoor->pDoors[object_id].pZOffsets[0], MAX_SOUND_DIST);

                result = _regularSoundPool.playUniquePid(sample, si->dataSource, pid, true);

                break;
            }

            case OBJECT_Character: {
                sample->SetVolume(uVoiceVolume);
                result = _voiceSoundPool.playUniquePid(sample, si->dataSource, pid);

                break;
            }

            case OBJECT_Actor: {
                assert(object_id < pActors.size());

                sample->SetPosition(pActors[object_id].pos.x,
                                    pActors[object_id].pos.y,
                                    pActors[object_id].pos.z, MAX_SOUND_DIST);

                // TODO(pskelton): Vanilla sounds like it does unique id but as exclusives
                // Actors play unique sounds between them. Avoids issues where in a real time mob you are hit with a cacophony of overlapping attack noises.
                result = _regularSoundPool.playUniqueSoundId(sample, si->dataSource, eSoundID, true);

                break;
            }

            case OBJECT_Decoration: {
                assert(object_id < pLevelDecorations.size());

                sample->SetPosition(pLevelDecorations[object_id].vPosition.x,
                                    pLevelDecorations[object_id].vPosition.y,
                                    pLevelDecorations[object_id].vPosition.z, MAX_SOUND_DIST);

                result = _loopingSoundPool.playNew(sample, si->dataSource, true);

                break;
            }

            case OBJECT_Item: {
                assert(object_id < pSpriteObjects.size());

                sample->SetPosition(pSpriteObjects[object_id].vPosition.x,
                                    pSpriteObjects[object_id].vPosition.y,
                                    pSpriteObjects[object_id].vPosition.z, MAX_SOUND_DIST);

                result = _regularSoundPool.playUniquePid(sample, si->dataSource, pid, true);
                break;
            }

            case OBJECT_Face: {
                result = _regularSoundPool.playUniquePid(sample, si->dataSource, pid);

                break;
            }

            default: {
                result = _regularSoundPool.playNew(sample, si->dataSource);
                logger->warning("Unexpected object type from Pid in playSound");
                break;
            }
        }
    }

    if (result == SOUND_PLAYBACK_FAILED || result == SOUND_PLAYBACK_SUCCEEDED) {
        // Only log sounds that actually play or tried to play
        if (engine->callObserver)
            engine->callObserver->notify(CALL_PLAY_SOUND, eSoundID);
    }

    switch (result) {
        case SOUND_PLAYBACK_FAILED:
            if (si->sName.empty()) {
                logger->warning("AudioPlayer: failed to play audio {} with name '{}'", std::to_underlying(eSoundID), si->sName);
            } else {
                logger->warning("AudioPlayer: failed to play audio {}", std::to_underlying(eSoundID));
            }
            break;
        case SOUND_PLAYBACK_SKIPPED:
            if (si->sName.empty()) {
                logger->trace("AudioPlayer: skipped playing sound {}", std::to_underlying(eSoundID));
            } else {
                logger->trace("AudioPlayer: skipped playing sound {} with name '{}'", std::to_underlying(eSoundID), si->sName);
            }
            break;
        case SOUND_PLAYBACK_SUCCEEDED:
            if (si->sName.empty()) {
                logger->trace("AudioPlayer: playing sound {}", std::to_underlying(eSoundID));
            } else {
                logger->trace("AudioPlayer: playing sound {} with name '{}'", std::to_underlying(eSoundID), si->sName);
            }
            break;
        default:
            break;
    }
}

bool AudioPlayer::loadSoundDataSource(SoundInfo* si) {
    if (!si->dataSource) {
        Blob buffer;

        if (si->sName == "") {  // enable this for bonus sound effects
            //logger->Info("AudioPlayer: trying to load bonus sound {}", eSoundID);
            //buffer = LoadSound(int(eSoundID));
        } else {
            buffer = LoadSound(si->sName);
        }

        if (!buffer) {
            logger->warning("AudioPlayer: failed to load sound {} ({})", std::to_underlying(si->uSoundID), si->sName);
            return false;
        }

        si->dataSource = CreateAudioBufferDataSource(std::move(buffer));
        if (!si->dataSource) {
            logger->warning("AudioPlayer: failed to create sound data source {} ({})", std::to_underlying(si->uSoundID), si->sName);
            return false;
        }

        si->dataSource = PlatformDataSourceInitialize(si->dataSource);
    }
    return true;
}

void AudioPlayer::UpdateSounds() {
    float pitch = M_PI * pParty->_viewPitch / 1024.f;
    float yaw = M_PI * pParty->_viewYaw / 1024.f;

    provider->SetOrientation(yaw, pitch);
    provider->SetListenerPosition(pParty->pos.x, pParty->pos.y, pParty->pos.z);

    _voiceSoundPool.update();
    _regularSoundPool.update();
    _loopingSoundPool.update();

    if (current_screen_type != SCREEN_GAME) {
        stopWalkingSounds();
    }
    if (_currentWalkingSample && _currentWalkingSample->IsStopped()) {
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::pauseAllSounds() {
    _voiceSoundPool.pause();
    _regularSoundPool.pause();
    _loopingSoundPool.pause();
    if (_currentWalkingSample) {
        _currentWalkingSample->Pause();
    }
}

void AudioPlayer::pauseLooping() {
    _loopingSoundPool.pause();
}

void AudioPlayer::soundDrain() {
    while (_voiceSoundPool.hasPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        _voiceSoundPool.update();
    }
    while (_regularSoundPool.hasPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        _regularSoundPool.update();
    }
}

bool AudioPlayer::isWalkingSoundPlays() {
    if (_currentWalkingSample) {
        return !_currentWalkingSample->IsStopped();
    }
    return false;
}

float AudioPlayer::getSoundLength(SoundId eSoundID) {
    SoundInfo* si = pSoundList->soundInfo(eSoundID);
    if (!si) {
        logger->warning("AudioPlayer: sound id {} not found", std::to_underlying(eSoundID));
        return 0.0f;
    }

    if (!si->dataSource) {
        // if the sound hasnt been used before - load it
        if (!loadSoundDataSource(si)) return 0.0f;

        // then force the sample to load/play to save codec info
        PAudioSample sample = CreateAudioSample();
        sample->SetVolume(0);
        _regularSoundPool.playNew(sample, si->dataSource);
    }

     return si->dataSource->GetDuration();
}

void AudioPlayer::Initialize() {
    currentMusicTrack = MUSIC_INVALID;
    uMasterVolume = 127;

    UpdateVolumeFromConfig();
    _sndReader.open(dfs->read("sounds/audio.snd"));

    bPlayerReady = true;
}

void AudioPlayer::UpdateVolumeFromConfig() {
    SetMasterVolume(engine->config->settings.SoundLevel.value());
    SetVoiceVolume(engine->config->settings.VoiceLevel.value());
    SetMusicVolume(engine->config->settings.MusicLevel.value());
}

void PlayLevelMusic() {
    if (engine->_currentLoadedMapId != MAP_INVALID) {
        pAudioPlayer->MusicPlayTrack(pMapStats->pInfos[engine->_currentLoadedMapId].musicId);
    }
}

Blob AudioPlayer::LoadSound(std::string_view pSoundName) {
    if (!_sndReader.exists(pSoundName)) {
        logger->warning("AudioPlayer: {} can't load sound header!", pSoundName);
        return Blob();
    }

    return _sndReader.read(pSoundName);
}

void AudioPlayer::playSpellSound(SpellId spell, bool is_impact, SoundPlaybackMode mode, Pid pid) {
    if (spell != SPELL_NONE)
        playSound(static_cast<SoundId>(SpellSoundIds[spell] + is_impact), mode, pid);
}

