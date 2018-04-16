#include "Media/Audio/AudioPlayer.h"

#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "Engine/MM7.h"
#include "Engine/Party.h"
#include "Engine/ZlibWrapper.h"

#include "Engine/Objects/Actor.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "Media/Audio/OpenALSoundProvider.h"

int sLastTrackLengthMS;
AudioPlayer *pAudioPlayer;
SoundList *pSoundList;

std::array<PartySpells, 4> stru_A750F8;
std::array<PartySpells, 4> AA1058_PartyQuickSpellSound;

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

class SoundInfo {
 public:
    bool Is3D() { return ((uFlags & SOUND_FLAG_3D) == SOUND_FLAG_3D); }

 public:
    std::string sName;
    SOUND_TYPE eType;
    uint32_t uSoundID;
    uint32_t uFlags;
    PMemBuffer buffer;
    PAudioSample sample;
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

void SoundList::Initialize() {}

void SoundList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    static_assert(sizeof(SoundDesc_mm6) == 112, "Wrong type size");
    static_assert(sizeof(SoundDesc) == 120, "Wrong type size");

    size_t num_mm6_sounds = data_mm6 ? *(uint32_t *)data_mm6 : 0;
    size_t num_mm7_sounds = data_mm7 ? *(uint32_t *)data_mm7 : 0;
    size_t num_mm8_sounds = data_mm8 ? *(uint32_t *)data_mm8 : 0;

    unsigned int sNumSounds = num_mm6_sounds + num_mm7_sounds + num_mm8_sounds;
    assert(sNumSounds);
    assert(!num_mm8_sounds);

    SoundDesc *sounds = (SoundDesc *)((char *)data_mm7 + 4);
    for (size_t i = 0; i < num_mm7_sounds; i++) {
        SoundInfo si;
        si.sName = (char *)sounds[i].pSoundName;
        si.uSoundID = sounds[i].uSoundID;
        si.eType = (SOUND_TYPE)sounds[i].eType;
        si.uFlags = sounds[i].uFlags;
        mapSounds[si.uSoundID] = si;
    }

    SoundDesc_mm6 *sounds_mm6 = (SoundDesc_mm6 *)((char *)data_mm6 + 4);
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

    if (!engine_config->NoSound() && bPlayerReady && engine_config->music_level > 0) {
        if (pCurrentMusicTrack) {
            pCurrentMusicTrack->Stop();
        }
        currentMusicTrack = -1;

        String file_path = StringPrintf("Music\\%d.mp3", eTrack);
        file_path = MakeDataPath(file_path.c_str());

        if (!FileExists(file_path.c_str())) {
            logger->Warning(L"Music\\%d.mp3 not found", eTrack);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(file_path);
        if (pCurrentMusicTrack) {
            currentMusicTrack = eTrack;
            pCurrentMusicTrack->SetVolume(
                pSoundVolumeLevels[engine_config->music_level]);
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

    pCurrentMusicTrack->Resume();
}

void AudioPlayer::SetMusicVolume(int vol) {
    if (!pCurrentMusicTrack) {
        return;
    }

    vol = max(0, vol);
    vol = min(9, vol);
    pCurrentMusicTrack->SetVolume(pSoundVolumeLevels[vol] * 64.f);
}

float AudioPlayer::MusicGetVolume() {
    if (!pCurrentMusicTrack) {
        return 0.f;
    }

    return pCurrentMusicTrack->GetVolume();
}

void AudioPlayer::SetMasterVolume(int level) {
    level = max(0, level);
    level = min(9, level);
    uMasterVolume = (unsigned int)(128.0f * pSoundVolumeLevels[level]);
}

void AudioPlayer::StopAll(int sample_id) {
    if (!bPlayerReady) {
        return;
    }
}

void AudioPlayer::PlaySound(SoundID eSoundID, int pid, unsigned int uNumRepeats,
                            int source_x, int source_y, int sound_data_id) {
    if (!bPlayerReady || engine_config->sound_level < 1 ||
        (eSoundID == SOUND_Invalid)) {
        return;
    }

    if (mapSounds.find(eSoundID) == mapSounds.end()) {
        logger->Warning(L"SoundID = %u not found", eSoundID);
        return;
    }

    SoundInfo &si = mapSounds[eSoundID];

    if (!si.sample) {
        PMemBuffer buffer = LoadSound(si.sName);
        if (!buffer) {
            logger->Warning(L"Failed to load sound \"%S\"", si.sName.c_str());
            return;
        }

        si.sample = CreateAudioSample(buffer);
        if (!si.sample) {
            return;
        }
    }

    if (!si.sample) {
        return;
    }

    if (pid == 0) {  // generic sound like from UI
        si.sample->Play();
        return;
    } else if (pid == -1) {  // exclusive sounds - can override
        si.sample->Stop();
        si.sample->Play();
        return;
    } else if (pid < 0) {  // exclusive sounds - no override (close chest)
        si.sample->Play();
    } else {
        ObjectType object_type = PID_TYPE(pid);
        int object_id = PID_ID(pid);
        switch (object_type) {
            case OBJECT_BLVDoor: {
                assert(uCurrentlyLoadedLevelType == LEVEL_Indoor);
                assert(object_id < pIndoor->uNumDoors);

                float x = pIndoor->pDoors[object_id].pXOffsets[0] -
                          pParty->vPosition.x;
                float y = pIndoor->pDoors[object_id].pYOffsets[0] -
                          pParty->vPosition.y;
                float z = pIndoor->pDoors[object_id].pZOffsets[0] -
                          pParty->vPosition.z;

                si.sample->SetPosition(x / 50.f, y / 50.f, z / 50.f, 500.f);
                si.sample->Play();

                return;
            }
            case OBJECT_Player: {
                si.sample->Play();
                return;
            }
            case OBJECT_Actor: {
                assert(object_id < uNumActors);

                float x = pActors[object_id].vPosition.x - pParty->vPosition.x;
                float y = pActors[object_id].vPosition.y - pParty->vPosition.y;
                float z = pActors[object_id].vPosition.z - pParty->vPosition.z;

                si.sample->SetPosition(x / 50.f, y / 50.f, z / 50.f, 500.f);
                si.sample->Play();
                return;
            }
            case OBJECT_Decoration: {
                assert(object_id < uNumLevelDecorations);

                float x = pLevelDecorations[object_id].vPosition.x -
                          pParty->vPosition.x;
                float y = pLevelDecorations[object_id].vPosition.y -
                          pParty->vPosition.y;
                float z = pLevelDecorations[object_id].vPosition.z -
                          pParty->vPosition.z;

                si.sample->SetPosition(x / 50.f, y / 50.f, z / 50.f, 500.f);
                si.sample->Play();
                return;
            }
            case OBJECT_Item: {
                assert(object_id < uNumSpriteObjects);

                float x =
                    pSpriteObjects[object_id].vPosition.x - pParty->vPosition.x;
                float y =
                    pSpriteObjects[object_id].vPosition.y - pParty->vPosition.y;
                float z =
                    pSpriteObjects[object_id].vPosition.z - pParty->vPosition.z;

                si.sample->SetPosition(x / 50.f, y / 50.f, z / 50.f, 500.f);
                si.sample->Play();
                return;
            }
            case OBJECT_BModel: {
                si.sample->Play();
                return;
            }

            default:
                assert(false);
        }
    }
}

void AudioPlayer::MessWithChannels() { pAudioPlayer->StopChannels(-1, -1); }

void AudioPlayer::UpdateSounds() {
    float pitch = 2. * pi_double * (double)pParty->sRotationX / 2048.;
    float yaw = 2. * pi_double * (double)pParty->sRotationY / 2048.;
    provider->SetOrientation(yaw, pitch);
}

void AudioPlayer::StopChannels(int uStartChannel, int uEndChannel) {}

#pragma pack(push, 1)
struct SoundHeader {
    char pSoundName[40];
    unsigned int uFileOffset;
    unsigned int uCompressedSize;
    unsigned int uDecompressedSize;
};
#pragma pack(pop)

// for audio////////////////////////////////////
FILE *hAudioSnd;
unsigned int uNumSoundHeaders;
struct SoundHeader *pSoundHeaders;

void AudioPlayer::LoadAudioSnd() {
    hAudioSnd = fopen(MakeDataPath("Sounds\\Audio.snd").c_str(), "rb");
    if (!hAudioSnd) {
        logger->Warning(L"Can't open file: %s", L"Sounds\\Audio.snd");
        return;
    }

    fread(&uNumSoundHeaders, 1, 4, hAudioSnd);
    pSoundHeaders = nullptr;
    pSoundHeaders = (SoundHeader *)malloc(52 * uNumSoundHeaders + 2);
    fread(pSoundHeaders, 1, sizeof(SoundHeader) * uNumSoundHeaders, hAudioSnd);
}

void AudioPlayer::Initialize() {
    currentMusicTrack = 0;
    uMasterVolume = 127;

    pAudioPlayer->SetMasterVolume(engine_config->sound_level);
    if (bPlayerReady) {
        SetMusicVolume(engine_config->music_level);
    }
    LoadAudioSnd();

    bPlayerReady = true;
}

void AudioPlayer::Release() {  // Освободить
    if (this->bPlayerReady) {
        free(pSoundHeaders);
    }
}

void PlayLevelMusic() {
    unsigned int map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        pAudioPlayer->MusicPlayTrack(
            (MusicID)pMapStats->pInfos[map_id].uRedbookTrackID);
    }
}

struct SoundHeader *AudioPlayer::FindSound(const std::string &pName) {
    std::string filename = pName;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    for (unsigned int i = 0; i < uNumSoundHeaders; ++i) {
        std::string soundname = pSoundHeaders[i].pSoundName;
        std::transform(soundname.begin(), soundname.end(), soundname.begin(),
                       ::tolower);
        if (soundname == filename) {
            return &pSoundHeaders[i];
        }
    }
    return nullptr;
}

PMemBuffer AudioPlayer::LoadSound(const std::string &pSoundName) {
    SoundHeader *header = FindSound(pSoundName);
    if (header == nullptr) {
        return nullptr;
    }

    PMemBuffer buffer = AllocMemBuffer(header->uDecompressedSize);

    fseek(hAudioSnd, header->uFileOffset, SEEK_SET);
    int NumberOfBytesRead = 0;
    if (header->uCompressedSize >= header->uDecompressedSize) {
        header->uCompressedSize = header->uDecompressedSize;
        if (header->uDecompressedSize) {
            NumberOfBytesRead = fread((void *)buffer->get_data(), 1,
                                      header->uDecompressedSize, hAudioSnd);
        } else {
            logger->Warning(L"Can't load sound file!");
        }
    } else {
        PMemBuffer compressed = AllocMemBuffer(header->uCompressedSize);
        NumberOfBytesRead = fread((void *)compressed->get_data(), 1,
                                  header->uCompressedSize, hAudioSnd);
        zlib::Uncompress((void *)buffer->get_data(), &header->uDecompressedSize,
                         (void *)compressed->get_data(),
                         header->uCompressedSize);
    }

    return buffer;
}

std::array<__int16, 101> word_4EE088_sound_ids = {
    {00000, 10000, 10010, 10020, 10030, 10040, 10050, 10060, 10070, 10080,
     10090, 10100, 11000, 11010, 11020, 11030, 11040, 11050, 11060, 11070,
     11080, 11090, 11100, 12000, 12010, 12020, 12030, 12040, 12050, 12060,
     12070, 12080, 12090, 12100, 13000, 13010, 13020, 13030, 13040, 13050,
     13060, 13070, 13080, 13090, 13100, 14000, 14010, 14020, 14030, 14040,
     14050, 14060, 14070, 14080, 14090, 14100, 15000, 15010, 15020, 15030,
     15040, 15050, 15060, 15070, 15080, 15090, 15100, 16000, 16010, 16020,
     16030, 16040, 16050, 16060, 16070, 16080, 16090, 16100, 17000, 17010,
     17020, 17030, 17040, 17050, 17060, 17070, 17080, 17090, 17100, 18000,
     18010, 18020, 18030, 18040, 18050, 18060, 18070, 18080, 18090, 18100,
     1}};

void AudioPlayer::PlaySpellSound(unsigned int spell, unsigned int pid) {
    PlaySound((SoundID)word_4EE088_sound_ids[spell], pid, 0, -1, 0, 0);
}

int PartySpells::AddPartySpellSound(int uSoundID, int a6) {
    static_assert(sizeof(PartySpells) == 45016, "Wrong type size");

    int v3 = 0;
    int result = word_4EE088_sound_ids[uSoundID];

    //  int a2a = word_4EE088_sound_ids[uSoundID];
    if (word_4EE088_sound_ids[uSoundID]) {
        for (int v9 = 0; v9 < 2; ++v9) {
            //      unsigned int v7 = a2a++;
            int v8;
            //      result = pSoundList->LoadSound(v7, (char *)this + v3, 44744
            //      - v3, &v8, a6);
            if (!result) break;
            a6 += 4;
            result = v8 + 256;
            this->pSoundsOffsets[v9] = v3;
            v3 += result;
            this->pSoundsSizes[v9] = v8 + 256;
        }
    }
    return result;
}
