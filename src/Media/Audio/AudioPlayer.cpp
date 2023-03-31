#include <algorithm>
#include <map>
#include <string>
#include <filesystem>
#include <utility>
#include <unordered_set>

#include "Media/Audio/AudioPlayer.h"

#include "Library/Compression/Compression.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"

#include "Media/Audio/OpenALSoundProvider.h"


int sLastTrackLengthMS;
AudioPlayer *pAudioPlayer;
SoundList *pSoundList;

std::array<float, 10> pSoundVolumeLevels = {
    {0.0000000f, 0.1099999f, 0.2199999f, 0.3300000f, 0.4399999f, 0.5500000f,
     0.6600000f, 0.7699999f, 0.8799999f, 0.9700000f}};

enum SOUND_TYPE {
    SOUND_TYPE_LEVEL = 0,
    SOUND_TYPE_SYSTEM = 1,
    SOUND_TYPE_SWAP = 2,
    SOUND_TYPE_UNKNOWN = 3,
    SOUND_TYPE_LOCK = 4,
};

enum SOUND_FLAG {
    SOUND_FLAG_LOCKED = 0x1,
    SOUND_FLAG_3D = 0x2,
};

// Max value used for volume control
// TODO(Nik-RE-dev): originally it was 2.0f, but OpenAL support gains from [0.0f, 1.0f] only
static const float maxVolumeGain = 1.0f;

class SoundInfo {
 public:
    bool Is3D() { return ((uFlags & SOUND_FLAG_3D) == SOUND_FLAG_3D); }

 public:
    std::string sName;
    SOUND_TYPE eType;
    uint32_t uSoundID;
    uint32_t uFlags;
    std::shared_ptr<Blob> buffer;
    PAudioSample sample;
    uint32_t last_pid = PID_INVALID;
};

std::map<uint32_t, SoundInfo> mapSounds;

#pragma pack(push, 1)
struct SoundDesc_mm6 {
    uint8_t pSoundName[32];
    uint32_t uSoundID;
    uint32_t eType;
    uint32_t uFlags;
    uint32_t pSoundDataID[17];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SoundDesc : public SoundDesc_mm6 {
    uint32_t p3DSoundID;
    uint32_t bDecompressed;
};
#pragma pack(pop)

// Cache to store different types of walking sounds
std::unordered_set<SoundID> walkingSoundIds;

void SoundList::Initialize() {}

void SoundList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    static_assert(sizeof(SoundDesc_mm6) == 112, "Wrong type size");
    static_assert(sizeof(SoundDesc) == 120, "Wrong type size");

    size_t num_mm6_sounds = data_mm6 ? *(uint32_t *)data_mm6.data() : 0;
    size_t num_mm7_sounds = data_mm7 ? *(uint32_t *)data_mm7.data() : 0;
    size_t num_mm8_sounds = data_mm8 ? *(uint32_t *)data_mm8.data() : 0;

    unsigned int sNumSounds = num_mm6_sounds + num_mm7_sounds + num_mm8_sounds;
    assert(sNumSounds);
    assert(!num_mm8_sounds);

    SoundDesc *sounds = (SoundDesc *)((char *)data_mm7.data() + 4);
    for (size_t i = 0; i < num_mm7_sounds; i++) {
        SoundInfo si;
        si.sName = (char *)sounds[i].pSoundName;
        si.uSoundID = sounds[i].uSoundID;
        si.eType = (SOUND_TYPE)sounds[i].eType;
        si.uFlags = sounds[i].uFlags;
        mapSounds[si.uSoundID] = si;
    }

    SoundDesc_mm6 *sounds_mm6 = (SoundDesc_mm6 *)((char *)data_mm6.data() + 4);
    for (size_t i = 0; i < num_mm6_sounds; i++) {
        SoundInfo si;
        si.sName = (char *)sounds_mm6[i].pSoundName;
        si.uSoundID = sounds_mm6[i].uSoundID;
        si.eType = (SOUND_TYPE)sounds_mm6[i].eType;
        si.uFlags = sounds_mm6[i].uFlags;
        mapSounds[si.uSoundID] = si;
    }
}

extern OpenALSoundProvider *provider;

void AudioPlayer::MusicPlayTrack(MusicID eTrack) {
    if (currentMusicTrack == eTrack) {
        return;
    }

    if (!engine->config->debug.NoSound.value() && bPlayerReady && engine->config->settings.MusicLevel.value() > 0) {
        if (pCurrentMusicTrack) {
            pCurrentMusicTrack->Stop();
        }
        currentMusicTrack = -1;

        std::string file_path = fmt::format("{}.mp3", eTrack);
        file_path = MakeDataPath("music", file_path);
        if (!std::filesystem::exists(file_path)) {
            logger->Warning("AudioPlayer: {} not found", file_path);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(file_path);
        if (pCurrentMusicTrack) {
            currentMusicTrack = eTrack;
            pCurrentMusicTrack->SetVolume(pSoundVolumeLevels[engine->config->settings.MusicLevel.value()] * maxVolumeGain);
            pCurrentMusicTrack->Play();
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
    currentMusicTrack = -1;
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
        if (currentMusicTrack > 0) {
            MusicStop();
            MusicPlayTrack((MusicID)playedMusicTrack);
        }
    }
}

void AudioPlayer::SetMusicVolume(int vol) {
    if (!pCurrentMusicTrack) {
        return;
    }

    vol = std::max(0, vol);
    vol = std::min(9, vol);
    pCurrentMusicTrack->SetVolume(pSoundVolumeLevels[vol] * maxVolumeGain);
}

float AudioPlayer::MusicGetVolume() {
    if (!pCurrentMusicTrack) {
        return 0.f;
    }

    return pCurrentMusicTrack->GetVolume();
}

void AudioPlayer::SetMasterVolume(int level) {
    level = std::max(0, level);
    level = std::min(9, level);
    uMasterVolume = (maxVolumeGain * pSoundVolumeLevels[level]);

    auto iter = mapSounds.begin();
    while (iter != mapSounds.end()) {
        SoundInfo &si = iter->second;
        if (si.sample) {
            // if not voice sample - set volume
            if (PID_TYPE(si.last_pid) != OBJECT_Player)
                si.sample->SetVolume(uMasterVolume);
        }
        ++iter;
    }
}

void AudioPlayer::SetVoiceVolume(int level) {
    level = std::max(0, level);
    level = std::min(9, level);
    uVoiceVolume = (maxVolumeGain * pSoundVolumeLevels[level]);

    auto iter = mapSounds.begin();
    while (iter != mapSounds.end()) {
        SoundInfo &si = iter->second;
        if (si.sample) {
            // if voice sample - set volume
            if (PID_TYPE(si.last_pid) == OBJECT_Player)
                si.sample->SetVolume(uVoiceVolume);
        }
        ++iter;
    }
}

void AudioPlayer::stopSounds() {
    if (!bPlayerReady) {
        return;
    }

    auto iter = mapSounds.begin();
    while (iter != mapSounds.end()) {
        SoundInfo &si = iter->second;
        if (si.sample) {
            if (si.sample->Stop() && engine->config->debug.VerboseLogging.value()) {
                logger->Info("sound stopped: {}", si.sName);
            }
        }
        ++iter;
    }
}

void AudioPlayer::stopWalkingSounds() {
    if (!bPlayerReady) {
        return;
    }

    for (SoundID id : walkingSoundIds) {
        SoundInfo &si = mapSounds[id];
        assert(si.sample);
        si.sample->Stop();
    }
}


void AudioPlayer::playSound(SoundID eSoundID, int pid, unsigned int uNumRepeats, int source_x, int source_y, int sound_data_id) {
    if (!bPlayerReady)
        return;

    //logger->Info("AudioPlayer: trying to load sound id {}", eSoundID);

    if (engine->config->settings.SoundLevel.value() < 1 || (eSoundID == SOUND_Invalid)) {
        return;
    }

    if (mapSounds.find(eSoundID) == mapSounds.end()) {
        logger->Warning("AudioPlayer: sound id {} not found", eSoundID);
        return;
    }

    SoundInfo &si = mapSounds[eSoundID];
    //logger->Info("AudioPlayer: sound id {} found as '{}'", eSoundID, si.sName);

    if (!si.sample) {
        Blob buffer;

        if (si.sName == "") {  // enable this for bonus sound effects
            //logger->Info("AudioPlayer: trying to load bonus sound {}", eSoundID);
            //buffer = LoadSound(int(eSoundID));
        } else {
            buffer = LoadSound(si.sName);
        }

        if (!buffer) {
            logger->Warning("AudioPlayer: failed to load sound {} ({})", eSoundID, si.sName);
            return;
        }

        si.sample = CreateAudioSample(std::move(buffer));
        if (!si.sample) {
            logger->Warning("AudioPlayer: failed to sample sound {} ({})", eSoundID, si.sName);
            return;
        }
    }

    si.sample->SetVolume(uMasterVolume);

    if (pid == 0) {  // generic sound like from UI
        si.sample->Play();
    } else if (pid == PID_INVALID) { // exclusive sounds - can override
        si.sample->Stop();
        si.sample->Play();
    } else if (pid == -1) { // all instances must be changed to PID_INVALID
        assert(false && "AudioPlayer::playSound - pid == -1 is encountered.");
        si.sample->Stop();
        si.sample->Play();
    } else if (pid == SOUND_PID_NON_RESETABLE) {  // exclusive sounds - no override (close chest)
        si.sample->Play();
    } else if (pid == SOUND_PID_WALKING) {
        si.sample->Stop();
        si.sample->Play();
        walkingSoundIds.insert(eSoundID);
    } else {
        ObjectType object_type = PID_TYPE(pid);
        unsigned int object_id = PID_ID(pid);
        switch (object_type) {
            case OBJECT_Door: {
                assert(uCurrentlyLoadedLevelType == LEVEL_Indoor);
                assert((int)object_id < pIndoor->pDoors.size());

                si.sample->SetPosition(pIndoor->pDoors[object_id].pXOffsets[0] / 50.f,
                                       pIndoor->pDoors[object_id].pYOffsets[0] / 50.f,
                                       pIndoor->pDoors[object_id].pZOffsets[0] / 50.f, 500.f);

                si.sample->Play(false, true);

                break;
            }

            case OBJECT_Player: {
                si.sample->SetVolume(uVoiceVolume);
                if (pid == SOUND_PID_PLAYER_RESETABLE) {
                    // This is special hack for voice volume control
                    si.sample->Stop();
                }
                si.sample->Play();

                break;
            }
            case OBJECT_Actor: {
                assert(object_id < pActors.size());

                si.sample->SetPosition(pActors[object_id].vPosition.x / 50.f,
                                       pActors[object_id].vPosition.y / 50.f,
                                       pActors[object_id].vPosition.z / 50.f, 500.f);

                si.sample->Play(false, true);

                break;
            }
            case OBJECT_Decoration: {
                assert(object_id < pLevelDecorations.size());

                si.sample->SetPosition((float)pLevelDecorations[object_id].vPosition.x / 50.f,
                                       (float)pLevelDecorations[object_id].vPosition.y / 50.f,
                                       (float)pLevelDecorations[object_id].vPosition.z / 50.f, 2000.f);

                si.sample->Play(true, true);

                break;
            }
            case OBJECT_Item: {
                assert(object_id < pSpriteObjects.size());

                si.sample->SetPosition(pSpriteObjects[object_id].vPosition.x / 50.f,
                                       pSpriteObjects[object_id].vPosition.y / 50.f,
                                       pSpriteObjects[object_id].vPosition.z / 50.f, 500.f);

                si.sample->Play(false, true);
               // return;
                break;
            }
            case OBJECT_Face: {
                si.sample->Play();

                break;
            }

            default: {
                // TODO(pskelton): temp fix to reduce instances of sounds not playing
                si.sample->Play();
                if (engine->config->debug.VerboseLogging.value())
                    logger->Warning("Unexpected object type from PID in playSound");
                break;
            }
        }
    }

    si.last_pid = pid;

    if (engine->config->debug.VerboseLogging.value()) {
        if (si.sName == "")
            logger->Info("AudioPlayer: playing sound {}", eSoundID);
        else
            logger->Info("AudioPlayer: playing sound {} with name '{}'", eSoundID, si.sName);
    }

    return;
}

void AudioPlayer::ResumeSounds() {
    auto iter = mapSounds.begin();
    while (iter != mapSounds.end()) {
        SoundInfo &si = iter->second;
        if (si.sample) {
            if (si.sample->Resume() && engine->config->debug.VerboseLogging.value())
                logger->Info("sound resumed: {}", si.sName);
        }
        ++iter;
    }
}

void AudioPlayer::UpdateSounds() {
    float pitch = pi * (float)pParty->_viewPitch / 1024.f;
    float yaw = pi * (float)pParty->_viewYaw / 1024.f;
    provider->SetOrientation(yaw, pitch);
    provider->SetListenerPosition(pParty->vPosition.x / 50.f,
                                  pParty->vPosition.y / 50.f,
                                  pParty->vPosition.z / 50.f);
}

void AudioPlayer::PauseSounds(int uType) {
    // pause everything
    if (uType == 2) {
        auto iter = mapSounds.begin();
        while (iter != mapSounds.end()) {
            SoundInfo &si = iter->second;
            if (si.sample) {
                if (si.sample->Pause() && engine->config->debug.VerboseLogging.value())
                    logger->Info("sound paused: {}", si.sName);
            }
            ++iter;
        }
    } else {
        // pause non exclusives
        auto iter = mapSounds.begin();
        while (iter != mapSounds.end()) {
            SoundInfo &si = iter->second;
            if (si.sample) {
                if (si.last_pid != PID_INVALID &&
                        si.last_pid != SOUND_PID_NON_RESETABLE) {
                    if (si.sample->Pause() && engine->config->debug.VerboseLogging.value())
                        logger->Info("sound paused: {}", si.sName);
                }
            }
            ++iter;
        }
    }
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

    std::string file_path = MakeDataPath("sounds", "audio.snd");
    fAudioSnd.open(MakeDataPath("sounds", "audio.snd"));

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
    currentMusicTrack = 0;
    uMasterVolume = 127;

    pAudioPlayer->SetMasterVolume(engine->config->settings.SoundLevel.value());
    pAudioPlayer->SetVoiceVolume(engine->config->settings.VoiceLevel.value());
    if (bPlayerReady) {
        SetMusicVolume(engine->config->settings.MusicLevel.value());
    }
    LoadAudioSnd();

    bPlayerReady = true;
}

void PlayLevelMusic() {
    unsigned int map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
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
            logger->Warning("Can't load sound file!");
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}


Blob AudioPlayer::LoadSound(const std::string &pSoundName) {
    SoundHeader header = { 0 };
    if (!FindSound(pSoundName, &header)) {
        logger->Warning("AudioPlayer: {} can't load sound header!", pSoundName);
        return Blob();
    }

    fAudioSnd.seek(header.uFileOffset);
    if (header.uCompressedSize >= header.uDecompressedSize) {
        header.uCompressedSize = header.uDecompressedSize;
        if (header.uDecompressedSize) {
            return Blob::read(fAudioSnd, header.uDecompressedSize);
        } else {
            logger->Warning("AudioPlayer: {} can't load sound file!", pSoundName);
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}

void AudioPlayer::playSpellSound(SPELL_TYPE spell, unsigned int pid, bool is_impact) {
    if (spell != SPELL_NONE)
        playSound(static_cast<SoundID>(SpellSoundIds[spell] + is_impact), pid, 0, -1, 0, 0);
}

