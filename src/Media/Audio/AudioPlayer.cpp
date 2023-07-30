#include "AudioPlayer.h"

#include <algorithm>
#include <map>
#include <string>
#include <filesystem>
#include <utility>
#include <vector>
#include <thread>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Party.h"
#include "Engine/Snapshots/EntitySnapshots.h"
#include "Engine/Engine.h"
#include "Engine/MapInfo.h"

#include "Media/Audio/OpenALSoundProvider.h"

#include "GUI/GUIWindow.h"

#include "Library/Snapshots/SnapshotSerialization.h"
#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"

#include "Utility/DataPath.h"

#include "SoundInfo.h"

int sLastTrackLengthMS;
AudioPlayer *pAudioPlayer;
SoundList *pSoundList;

std::array<float, 10> pSoundVolumeLevels = {
    {0.0000000f, 0.1099999f, 0.2199999f, 0.3300000f, 0.4399999f, 0.5500000f,
     0.6600000f, 0.7699999f, 0.8799999f, 0.9700000f}};

std::map<uint32_t, SoundInfo> mapSounds;

void SoundList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    std::vector<SoundInfo> sounds;

    if (data_mm6)
        deserialize(data_mm6, &sounds, tags::append, tags::via<SoundInfo_MM6>);
    if (data_mm7)
        deserialize(data_mm7, &sounds, tags::append, tags::via<SoundInfo_MM7>);
    if (data_mm8)
        deserialize(data_mm8, &sounds, tags::append, tags::via<SoundInfo_MM7>);

    assert(!sounds.empty());

    for (const SoundInfo &sound : sounds)
        mapSounds[sound.uSoundID] = sound;
}

extern OpenALSoundProvider *provider;

void AudioPlayer::MusicPlayTrack(MusicID eTrack) {
    if (currentMusicTrack == eTrack) {
        return;
    }

    if (!engine->config->debug.NoSound.value() && bPlayerReady) {
        if (pCurrentMusicTrack) {
            pCurrentMusicTrack->Stop();
        }
        currentMusicTrack = MUSIC_Invalid;

        std::string file_path = fmt::format("{}.mp3", eTrack);
        file_path = makeDataPath("music", file_path);
        if (!std::filesystem::exists(file_path)) {
            logger->warning("AudioPlayer: {} not found", file_path);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(file_path);
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
    currentMusicTrack = MUSIC_Invalid;
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
        int playedMusicTrack = currentMusicTrack;
        if (currentMusicTrack != MUSIC_Invalid) {
            MusicStop();
            MusicPlayTrack((MusicID)playedMusicTrack);
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

void AudioPlayer::playSound(SoundID eSoundID, int pid) {
    if (!bPlayerReady)
        return;

    //logger->Info("AudioPlayer: trying to load sound id {}", eSoundID);

    if (engine->config->settings.SoundLevel.value() < 1 || (eSoundID == SOUND_Invalid)) {
        return;
    }

    if (mapSounds.find(eSoundID) == mapSounds.end()) {
        logger->warning("AudioPlayer: sound id {} not found", eSoundID);
        return;
    }

    SoundInfo &si = mapSounds[eSoundID];
    //logger->Info("AudioPlayer: sound id {} found as '{}'", eSoundID, si.sName);

    if (!si.dataSource) {
        Blob buffer;

        if (si.sName == "") {  // enable this for bonus sound effects
            //logger->Info("AudioPlayer: trying to load bonus sound {}", eSoundID);
            //buffer = LoadSound(int(eSoundID));
        } else {
            buffer = LoadSound(si.sName);
        }

        if (!buffer) {
            logger->warning("AudioPlayer: failed to load sound {} ({})", eSoundID, si.sName);
            return;
        }

        si.dataSource = CreateAudioBufferDataSource(std::move(buffer));
        if (!si.dataSource) {
            logger->warning("AudioPlayer: failed to create sound data source {} ({})", eSoundID, si.sName);
            return;
        }

        si.dataSource = PlatformDataSourceInitialize(si.dataSource);
    }

    PAudioSample sample = CreateAudioSample();

    bool result = true;
    sample->SetVolume(uMasterVolume);

    if (pid == 0) {  // generic sound like from UI
        result = _regularSoundPool.playNew(sample, si.dataSource);
    } else if (pid == PID_INVALID) { // exclusive sounds - can override
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si.dataSource, eSoundID);
    } else if (pid == -1) { // all instances must be changed to PID_INVALID
        assert(false && "AudioPlayer::playSound - pid == -1 is encountered.");
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si.dataSource, eSoundID);
    } else if (pid == SOUND_PID_NON_RESETABLE) {  // exclusive sounds - no override (close chest)
        result = _regularSoundPool.playUniqueSoundId(sample, si.dataSource, eSoundID);
    } else if (pid == SOUND_PID_WALKING) {
        if (_currentWalkingSample) {
            _currentWalkingSample->Stop();
        }
        _currentWalkingSample = sample;
        _currentWalkingSample->Open(si.dataSource);
        _currentWalkingSample->Play();
    } else if (pid == SOUND_PID_MUSIC_VOLUME) {
        sample->SetVolume(uMusicVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si.dataSource, eSoundID);
    } else if (pid == SOUND_PID_VOICE_VOLUME) {
        sample->SetVolume(uVoiceVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, si.dataSource, eSoundID);
    } else if (pid == SOUND_PID_HOUSE_SPEECH || pid == SOUND_PID_HOUSE_DOOR) {
        _regularSoundPool.stopPid(pid);
        _regularSoundPool.playUniquePid(sample, si.dataSource, pid);
    } else {
        ObjectType object_type = PID_TYPE(pid);
        unsigned int object_id = PID_ID(pid);
        switch (object_type) {
            case OBJECT_Door: {
                assert(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
                assert((int)object_id < pIndoor->pDoors.size());

                sample->SetPosition(pIndoor->pDoors[object_id].pXOffsets[0],
                                    pIndoor->pDoors[object_id].pYOffsets[0],
                                    pIndoor->pDoors[object_id].pZOffsets[0], MAX_SOUND_DIST);

                result = _regularSoundPool.playUniquePid(sample, si.dataSource, pid, true);

                break;
            }

            case OBJECT_Character: {
                sample->SetVolume(uVoiceVolume);
                result = _voiceSoundPool.playUniquePid(sample, si.dataSource, pid);

                break;
            }

            case OBJECT_Actor: {
                assert(object_id < pActors.size());

                sample->SetPosition(pActors[object_id].pos.x,
                                    pActors[object_id].pos.y,
                                    pActors[object_id].pos.z, MAX_SOUND_DIST);

                result = _regularSoundPool.playUniquePid(sample, si.dataSource, pid, true);

                break;
            }

            case OBJECT_Decoration: {
                assert(object_id < pLevelDecorations.size());

                sample->SetPosition(pLevelDecorations[object_id].vPosition.x,
                                    pLevelDecorations[object_id].vPosition.y,
                                    pLevelDecorations[object_id].vPosition.z, MAX_SOUND_DIST);

                result = _loopingSoundPool.playNew(sample, si.dataSource, true);

                break;
            }

            case OBJECT_Item: {
                assert(object_id < pSpriteObjects.size());

                sample->SetPosition(pSpriteObjects[object_id].vPosition.x,
                                    pSpriteObjects[object_id].vPosition.y,
                                    pSpriteObjects[object_id].vPosition.z, MAX_SOUND_DIST);

                result = _regularSoundPool.playUniquePid(sample, si.dataSource, pid, true);
                break;
            }

            case OBJECT_Face: {
                result = _regularSoundPool.playUniquePid(sample, si.dataSource, pid);

                break;
            }

            default: {
                result = _regularSoundPool.playNew(sample, si.dataSource);
                logger->warning("Unexpected object type from PID in playSound");
                break;
            }
        }
    }

    if (!result) {
        if (si.sName.empty()) {
            logger->warning("AudioPlayer: failed to play audio {} with name '{}'", eSoundID, si.sName);
        } else {
            logger->warning("AudioPlayer: failed to play audio {}", eSoundID);
        }
    } else {
        if (si.sName.empty()) {
            logger->verbose("AudioPlayer: playing sound {}", eSoundID);
        } else {
            logger->verbose("AudioPlayer: playing sound {} with name '{}'", eSoundID, si.sName);
        }
    }
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

bool AudioSamplePool::playNew(PAudioSample sample, PAudioDataSource source, bool positional) {
    update();
    if (!sample->Open(source)) {
        return false;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, PID_INVALID));
    return true;
}

bool AudioSamplePool::playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundID id, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.id == id) {
            return true;
        }
    }
    if (!sample->Open(source)) {
        return false;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, id, PID_INVALID));
    return true;
}

bool AudioSamplePool::playUniquePid(PAudioSample sample, PAudioDataSource source, int pid, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.pid == pid) {
            return true;
        }
    }
    if (!sample->Open(source)) {
        return false;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, pid));
    return true;
}

void AudioSamplePool::pause() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Pause();
    }
}

void AudioSamplePool::resume() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Resume();
    }
}

void AudioSamplePool::stop() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Stop();
    }
    _samplePool.clear();
}

void AudioSamplePool::stopSoundId(SoundID soundId) {
    assert(soundId != SOUND_Invalid);

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).id == soundId) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::stopPid(int pid) {
    assert(pid != PID_INVALID);

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).pid == pid) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::update() {
    auto it = _samplePool.begin();
    std::erase_if(_samplePool, [](const AudioSamplePoolEntry& entry) { return entry.samplePtr->IsStopped(); });
}

void AudioSamplePool::setVolume(float value) {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->SetVolume(value);
    }
}

bool AudioSamplePool::hasPlaying() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (!entry.samplePtr->IsStopped()) {
            return true;
        }
    }
    return false;
}

#pragma pack(push, 1)
struct SoundHeader_mm7 {
    char pSoundName[40];
    uint32_t uFileOffset;
    uint32_t uCompressedSize;
    uint32_t uDecompressedSize;
};
#pragma pack(pop)

void AudioPlayer::LoadAudioSnd() {
    static_assert(sizeof(SoundHeader_mm7) == 52, "Wrong type size");

    std::string file_path = makeDataPath("sounds", "audio.snd");
    fAudioSnd.open(makeDataPath("sounds", "audio.snd"));

    uint32_t uNumSoundHeaders {};
    fAudioSnd.readOrFail(&uNumSoundHeaders, sizeof(uNumSoundHeaders));
    for (uint32_t i = 0; i < uNumSoundHeaders; i++) {
        SoundHeader_mm7 header_mm7;
        fAudioSnd.readOrFail(&header_mm7, sizeof(header_mm7));
        SoundHeader header;
        header.uFileOffset = header_mm7.uFileOffset;
        header.uCompressedSize = header_mm7.uCompressedSize;
        header.uDecompressedSize = header_mm7.uDecompressedSize;
        mSoundHeaders[toLower(header_mm7.pSoundName)] = header;
    }
}

void AudioPlayer::Initialize() {
    currentMusicTrack = MUSIC_Invalid;
    uMasterVolume = 127;

    SetMasterVolume(engine->config->settings.SoundLevel.value());
    SetVoiceVolume(engine->config->settings.VoiceLevel.value());
    SetMusicVolume(engine->config->settings.MusicLevel.value());

    LoadAudioSnd();

    bPlayerReady = true;
}

void PlayLevelMusic() {
    MAP_TYPE map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id != MAP_INVALID) {
        pAudioPlayer->MusicPlayTrack((MusicID)pMapStats->pInfos[map_id].uRedbookTrackID);
    }
}


bool AudioPlayer::FindSound(const std::string &pName, AudioPlayer::SoundHeader *header) {
    if (header == nullptr) {
        return false;
    }

    std::map<std::string, SoundHeader>::iterator it = mSoundHeaders.find(toLower(pName));
    if (it == mSoundHeaders.end()) {
        return false;
    }

    *header = it->second;

    return true;
}


Blob AudioPlayer::LoadSound(int uSoundID) {  // bit of a kludge (load sound by ID index) - plays some interesting files
    SoundHeader header = { 0 };

    if (uSoundID < 0 || uSoundID > mSoundHeaders.size())
        return {};

    // iterate through to get sound by int ID
    std::map<std::string, SoundHeader>::iterator it = mSoundHeaders.begin();
    std::advance(it, uSoundID);

    if (it == mSoundHeaders.end())
        return {};

    header = it->second;

    fAudioSnd.seek(header.uFileOffset);
    if (header.uCompressedSize >= header.uDecompressedSize) {
        header.uCompressedSize = header.uDecompressedSize;
        if (header.uDecompressedSize) {
            return Blob::read(fAudioSnd, header.uDecompressedSize);
        } else {
            logger->warning("Can't load sound file!");
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}


Blob AudioPlayer::LoadSound(const std::string &pSoundName) {
    SoundHeader header = { 0 };
    if (!FindSound(pSoundName, &header)) {
        logger->warning("AudioPlayer: {} can't load sound header!", pSoundName);
        return Blob();
    }

    fAudioSnd.seek(header.uFileOffset);
    if (header.uCompressedSize >= header.uDecompressedSize) {
        header.uCompressedSize = header.uDecompressedSize;
        if (header.uDecompressedSize) {
            return Blob::read(fAudioSnd, header.uDecompressedSize);
        } else {
            logger->warning("AudioPlayer: {} can't load sound file!", pSoundName);
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}

void AudioPlayer::playSpellSound(SPELL_TYPE spell, unsigned int pid, bool is_impact) {
    if (spell != SPELL_NONE)
        playSound(static_cast<SoundID>(SpellSoundIds[spell] + is_impact), pid);
}

