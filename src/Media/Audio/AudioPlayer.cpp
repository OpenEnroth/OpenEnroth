#include "AudioPlayer.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <map>
#include <string>
#include <utility>
#include <memory>

#include "Engine/GameType.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Resources/ResourceManager.h"
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

// MM8's dsounds.bin shifts MM7's UI sound ids 200-232 down by 67 (204 TurnPage1 is 137, 232 teleport is 165)
// and uses 205 for the quest jingle. Ids 48 and the per-monster "bored" slot have no MM8 sound.
static SoundId mm8TranslatedSoundId(SoundId id) {
    if (gameType() != GAME_TYPE_MM8)
        return id;

    switch (id) {
    case SOUND_quest:              return static_cast<SoundId>(205);
    case SOUND_EndTurnBasedMode:   return static_cast<SoundId>(139);
    case SOUND_StartTurnBasedMode: return static_cast<SoundId>(140);
    case SOUND_openchest0101:      return static_cast<SoundId>(141);
    case SOUND_spellfail0201:      return static_cast<SoundId>(142);
    case SOUND_openbook:           return static_cast<SoundId>(163);
    case SOUND_closebook:          return static_cast<SoundId>(164);
    case SOUND_teleport:           return static_cast<SoundId>(165);
    default:                       return id;
    }
}

AudioPlayer::~AudioPlayer() = default;

void AudioPlayer::MusicPlayTrack(MusicId eTrack) {
    if (currentMusicTrack == eTrack)
        return;

    // Music (music/*.mp3) doesn't depend on sounds/audio.snd, so it must not be gated
    // on bPlayerReady - that flag only tracks SFX sample data readiness.
    // NoSound and the dfs->exists() check below already cover the
    // disabled/missing-file cases correctly.
    if (!engine->config->debug.NoSound.value()) {
        if (pCurrentMusicTrack)
            pCurrentMusicTrack->Stop();
        currentMusicTrack = MUSIC_INVALID;

        std::string file_path = fmt::format("music/{}.mp3", std::to_underlying(eTrack));
        if (!dfs->exists(file_path)) {
            MM_WARNING("AudioPlayer: {} not found", file_path);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(dfs->read(file_path));
        if (pCurrentMusicTrack) {
            currentMusicTrack = eTrack;

            pCurrentMusicTrack->SetVolume(uMusicVolume);
            pCurrentMusicTrack->Play();
            if (uMusicVolume == 0.0)
                pCurrentMusicTrack->Pause();
        }
    }
}

void AudioPlayer::MusicStart() {}

void AudioPlayer::MusicStop() {
    if (!pCurrentMusicTrack)
        return;

    pCurrentMusicTrack->Stop();
    pCurrentMusicTrack = nullptr;
    currentMusicTrack = MUSIC_INVALID;
}

void AudioPlayer::MusicPause() {
    if (!pCurrentMusicTrack)
        return;

    pCurrentMusicTrack->Pause();
}

void AudioPlayer::MusicResume() {
    if (!pCurrentMusicTrack)
        return;

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

    if (!pCurrentMusicTrack)
        return;

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
    if (_currentWalkingSample)
        _currentWalkingSample->SetVolume(uMasterVolume);
}

void AudioPlayer::SetVoiceVolume(int level) {
    level = std::clamp(level, 0, 9);
    uVoiceVolume = pSoundVolumeLevels[level];

    _voiceSoundPool.setVolume(uVoiceVolume);
}

void AudioPlayer::stopSounds() {
    if (!bPlayerReady)
        return;

    _voiceSoundPool.stop();
    _regularSoundPool.stop();
    _loopingSoundPool.stop();
    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::stopVoiceSounds() {
    if (!bPlayerReady)
        return;

    _voiceSoundPool.stop();
}

void AudioPlayer::stopWalkingSounds() {
    if (!bPlayerReady)
        return;

    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::resumeSounds() {
    _voiceSoundPool.resume();
    _regularSoundPool.resume();
    _loopingSoundPool.resume();
    if (_currentWalkingSample)
        _currentWalkingSample->Resume();
}

void AudioPlayer::playSound(SoundId eSoundID, SoundPlaybackMode mode, Pid pid) {
    if (!bPlayerReady)
        return;

    //logger->Info("AudioPlayer: trying to load sound id {}", eSoundID);

    // TODO(pskelton): do we need to reinstate this optimisation? dropped to allow better sound tracing
    if (/*engine->config->settings.SoundLevel.value() < 1 ||*/ eSoundID == SOUND_Invalid)
        return;

    eSoundID = mm8TranslatedSoundId(eSoundID);

    SoundInfo *si = pSoundList->soundInfo(eSoundID);
    if (!si) {
        MM_WARNING("AudioPlayer: sound id {} not found", std::to_underlying(eSoundID));
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
        if (_currentWalkingSample)
            _currentWalkingSample->Stop();
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
        result = _regularSoundPool.playUniquePid(sample, si->dataSource, pid);
    } else {
        assert(pid);

        ObjectType object_type = pid.type();
        int object_id = pid.id();
        switch (object_type) {
            case OBJECT_Door: {
                assert(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
                assert((int)object_id < pIndoor->doors.size());

                sample->SetPosition(pIndoor->doors[object_id].offsets[0].x,
                                    pIndoor->doors[object_id].offsets[0].y,
                                    pIndoor->doors[object_id].offsets[0].z, MAX_SOUND_DIST);

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

            case OBJECT_Sprite: {
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
                MM_WARNING("Unexpected object type from Pid in playSound");
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
            if (si->name.empty()) {
                MM_WARNING("AudioPlayer: failed to play audio {}", std::to_underlying(eSoundID));
            } else {
                MM_WARNING("AudioPlayer: failed to play audio {} with name '{}'", std::to_underlying(eSoundID), si->name);
            }
            break;
        case SOUND_PLAYBACK_SKIPPED:
            if (si->name.empty()) {
                MM_TRACE("AudioPlayer: skipped playing sound {}", std::to_underlying(eSoundID));
            } else {
                MM_TRACE("AudioPlayer: skipped playing sound {} with name '{}'", std::to_underlying(eSoundID), si->name);
            }
            break;
        case SOUND_PLAYBACK_SUCCEEDED:
            if (si->name.empty()) {
                MM_TRACE("AudioPlayer: playing sound {}", std::to_underlying(eSoundID));
            } else {
                MM_TRACE("AudioPlayer: playing sound {} with name '{}'", std::to_underlying(eSoundID), si->name);
            }
            break;
        default:
            break;
    }
}

bool AudioPlayer::loadSoundDataSource(SoundInfo* si) {
    if (!si->dataSource) {
        Blob buffer;

        if (si->name == "") {  // enable this for bonus sound effects
            //logger->Info("AudioPlayer: trying to load bonus sound {}", eSoundID);
            //buffer = LoadSound(int(eSoundID));
        } else {
            buffer = LoadSound(si->name);
        }

        if (!buffer) {
            MM_WARNING("AudioPlayer: failed to load sound {} ({})", std::to_underlying(si->soundId), si->name);
            return false;
        }

        si->dataSource = CreateAudioBufferDataSource(std::move(buffer));
        if (!si->dataSource) {
            MM_WARNING("AudioPlayer: failed to create sound data source {} ({})", std::to_underlying(si->soundId), si->name);
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

    if (current_screen_type != SCREEN_GAME)
        stopWalkingSounds();
    if (_currentWalkingSample && _currentWalkingSample->IsStopped())
        _currentWalkingSample = nullptr;
}

void AudioPlayer::pauseAllSounds() {
    _voiceSoundPool.pause();
    _regularSoundPool.pause();
    _loopingSoundPool.pause();
    if (_currentWalkingSample)
        _currentWalkingSample->Pause();
}

void AudioPlayer::pauseLooping() {
    _loopingSoundPool.pause();
}

void AudioPlayer::soundDrain() {
    // A paused or broken source never reports that it stopped, so the wait is bounded.
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    for (AudioSamplePool *pool : {&_voiceSoundPool, &_regularSoundPool}) {
        while (pool->hasPlaying() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            pool->update();
        }
    }
}

bool AudioPlayer::isWalkingSoundPlays() {
    if (_currentWalkingSample)
        return !_currentWalkingSample->IsStopped();
    return false;
}

float AudioPlayer::getSoundLength(SoundId eSoundID) {
    if (!bPlayerReady)
        return 0.0f;

    eSoundID = mm8TranslatedSoundId(eSoundID);

    SoundInfo* si = pSoundList->soundInfo(eSoundID);
    if (!si) {
        MM_WARNING("AudioPlayer: sound id {} not found", std::to_underlying(eSoundID));
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

    // MM8 installs don't ship `sounds/audio.snd` at all (see PathResolver.cpp's
    // `mm8ValidateList`) - unlike MM7, its sound samples are plain .wav entries inside
    // `data/EnglishD.lod` (see LoadSound() below), which `ResourceManager` opens.
    // Ready as soon as that LOD is available, no `_sndReader` involved.
    if (gameType() == GAME_TYPE_MM8) {
        bPlayerReady = engine->resources()->isEnglishDOpen();
        return;
    }

    if (!dfs->exists("sounds/audio.snd"))
        return;
    _sndReader.open(dfs->read("sounds/audio.snd"));

    bPlayerReady = true;
}

void AudioPlayer::UpdateVolumeFromConfig() {
    SetMasterVolume(engine->config->settings.SoundLevel.value());
    SetVoiceVolume(engine->config->settings.VoiceLevel.value());
    SetMusicVolume(engine->config->settings.MusicLevel.value());
}

void PlayLevelMusic() {
    if (engine->_currentLoadedMapId != MAP_INVALID)
        pAudioPlayer->MusicPlayTrack(pMapTable->pInfos[engine->_currentLoadedMapId].musicId);
}

Blob AudioPlayer::LoadSound(std::string_view pSoundName) {
    // MM8 keeps its sound samples as plain .wav entries in `data/EnglishD.lod` instead of MM7's
    // `sounds/audio.snd` container, in the same
    // LOD-compressed-entry format used there for images/text. LOD entry names are truncated to
    // 16 bytes in the container itself,
    // so a long name loses its ".wav" suffix there too - truncate our lookup the same way.
    if (gameType() == GAME_TYPE_MM8) {
        std::string wavName = fmt::format("{}.wav", pSoundName);
        if (wavName.size() > 16)
            wavName.resize(16);

        if (!engine->resources()->hasEnglishD(wavName)) {
            MM_WARNING("AudioPlayer: {} can't load sound header!", pSoundName);
            return Blob();
        }

        return engine->resources()->englishDData(wavName);
    }

    if (!_sndReader.exists(pSoundName)) {
        MM_WARNING("AudioPlayer: {} can't load sound header!", pSoundName);
        return Blob();
    }

    return _sndReader.read(pSoundName);
}

void AudioPlayer::playSpellSound(SpellId spell, bool is_impact, SoundPlaybackMode mode, Pid pid) {
    if (spell == SPELL_NONE)
        return;
    SoundId soundId = static_cast<SoundId>(SpellSoundIds[spell] + is_impact);
    // Some MM8 spells have a cast sound and no impact sound: SPELL_EARTH_BLADES has 13050 and nothing
    // at 13051. The original is silent there, so ask the table.
    if (gameType() == GAME_TYPE_MM8 && is_impact && !pSoundList->soundInfo(soundId))
        return;
    playSound(soundId, mode, pid);
}

