#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>
#include <sstream>
#include <string>

#include "Engine/Engine.h"

#include "Engine/ZlibWrapper.h"

#include "../MediaPlayer.h"
#include "AudioPlayer.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Timer.h"
#include "Engine/OurMath.h"
#include "Engine/MapInfo.h"
#include "GUI/GUIWindow.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "Media/Video/Bink_Smacker.h"

#include "Engine/MMT.h"


PCMWAVEFORMAT pcmWaveFormat;

int uFindSound_BinSearch_ResultID; // weak
int uLastLoadedSoundID; // weak
int sLastTrackLengthMS;
std::array<Sound, 3000> pSounds;
AudioPlayer *pAudioPlayer;
SoundList *pSoundList;


std::array<stru339_spell_sound, 4> stru_A750F8;
std::array<stru339_spell_sound, 4> AA1058_PartyQuickSpellSound;



unsigned __int8 uSoundVolumeMultiplier;// = 4;
unsigned __int8 uVoicesVolumeMultiplier;// = 4;
unsigned __int8 uMusicVolimeMultiplier;// = 4;
int bWalkSound; // idb

std::array<float, 10> pSoundVolumeLevels = 
{
  0.0000000f, 0.1099999f, 0.2199999f, 0.3300000f, 0.4399999f,
 0.5500000f, 0.6600000f, 0.7699999f, 0.8799999f, 0.9700000f  
//  0.0000000f, 0.4900000f, 0.5500000f, 0.6100000f, 0.6700000f,  //for 128.0f
// 0.7000000f, 0.7600000f, 0.8200000f, 0.8800000f, 0.9700000f     //changed 0.9900000f to 0.9700000f. for some reason it only works for values below this
};




void ReleaseSoundData(void *_this);
_DIG_DRIVER *Audio_GetFirstHardwareDigitalDriver();




//----- (004A9953) --------------------------------------------------------
void SoundList::Initialize()
{
  SoundDesc *pSoundDesc; // eax@5
  void *pSoundData; // ebx@7
  unsigned int uSoundSize; // eax@7
  char *pSoundBytes; // ebx@7
  AILFILETYPE pType; // eax@7
  int v8; // eax@8
  char pSoundName[120]; // [sp+4h] [bp-A4h]@4
  AILSOUNDINFO pInfo; // [sp+7Ch] [bp-2Ch]@10
  int v12; // [sp+A0h] [bp-8h]@12

  if ( sNumSounds > 1 )
  {
    for ( uint i = 1; i < pSoundList->sNumSounds; ++i )
    {
      sprintf(pSoundName, "%s", pSL_Sounds[i].pSoundName);
      pSoundDesc = &pSoundList->pSL_Sounds[i];
	  //if ( pSL_Sounds[i].uSoundID == 75 )
	    //__debugbreak();
      if ( pSoundList->pSL_Sounds[i].eType != SOUND_DESC_SYSTEM )
        continue;
      pSoundList->pSL_Sounds[i].pSoundData[0] = ::LoadSound(pSoundName, (SoundData *)-1, pSL_Sounds[i].uSoundID); // Ritor result crash exe file
      if ( !pAudioPlayer->b3DSoundInitialized )
        continue;
      pSoundDesc = &pSoundList->pSL_Sounds[i];
      if ( !(pSoundDesc->uFlags & SOUND_DESC_SWAP) || !pSoundDesc->pSoundData[0] )
        continue;
      pSoundData = pSoundDesc->pSoundData[0];
      uSoundSize = *(int *)pSoundData;
      pSoundBytes = (char *)pSoundData + 4;
      pType = AIL_file_type(pSoundBytes, uSoundSize);
      if ( !pType )
      {
        pSoundList->pSL_Sounds[i].bDecompressed = false;
        pSoundList->UnloadSound(i, 1);
        continue;
      }
      v8 = pType - 1;
      if ( v8 )
      {
        if ( v8 == 1 )
        {
          if ( AIL_WAV_info(pSoundBytes, &pInfo) && pInfo.uChannels != 2 )
          {
            if ( !AIL_decompress_ADPCM(&pInfo, &pSoundList->pSL_Sounds[i].p3DSound, &v12) )
            {
              pSoundList->pSL_Sounds[i].p3DSound = 0;
              pSoundList->pSL_Sounds[i].bDecompressed = true;
            }
          }
          pSoundList->UnloadSound(i, 1);
          continue;
        }
        pSoundList->pSL_Sounds[i].bDecompressed = false;
        pSoundList->UnloadSound(i, 1);
        continue;
      }
      pSoundList->pSL_Sounds[i].p3DSound = pSoundList->pSL_Sounds[i].pSoundData[0];
      pSoundList->UnloadSound(i, 1);
    }
  }
  //_CrtDumpMemoryLeaks();
}

//----- (004A9A67) --------------------------------------------------------
__int16 SoundList::LoadSound(int a1, unsigned int a3)
{
  AILSOUNDINFO v24; // [sp+84h] [bp-28h]@23

  if (bNoSound || !sNumSounds)
    return 0;

  uint       uSoundIdx = 0;
  SoundDesc *pSound = nullptr;
  for (uint i = 1; i < sNumSounds; ++i)
    if (pSL_Sounds[i].uSoundID == a1)
    {
      uSoundIdx = i;
      pSound = &pSL_Sounds[i];
      break;
    }
  if (!pSound)
    return 0;

  if (pSound->uFlags & SOUND_DESC_SWAP && pSound->p3DSound ||
      ~pSound->uFlags & SOUND_DESC_SWAP && pSound->pSoundData[0])
    return uSoundIdx;

  if (!pSound->pSoundData[0])
    pSound->pSoundData[0] = ::LoadSound(pSound->pSoundName, (SoundData *)-1, pSound->uSoundID);

  if (!pSound->pSoundData[0])
    return 0;

  if (a3)
    pSound->uFlags |= SOUND_DESC_SYSTEM;

  if (!pAudioPlayer->b3DSoundInitialized)
    return uSoundIdx;

  if (~pSound->uFlags & SOUND_DESC_SWAP || !pSound->pSoundData[0])
    return uSoundIdx;


  SoundData* pSoundData = pSound->pSoundData[0];
  switch (AIL_file_type((void *)pSoundData->pData, pSoundData->uDataSize))
  {
    default:
    case AILFILETYPE_UNKNOWN:
      pSound->bDecompressed = false;
      return 0;

    case AILFILETYPE_PCM_WAV:
      pSound->p3DSound = pSound->pSoundData[0];
      return uSoundIdx;

    case AILFILETYPE_ADPCM_WAV:
      if (AIL_WAV_info((void *)pSoundData->pData, &v24) && v24.uChannels != 2)
      {
        if (!AIL_decompress_ADPCM(&v24, &pSound->p3DSound, &a1) )
        {
          pSound->p3DSound = nullptr;
          pSound->bDecompressed = true;
          UnloadSound(uSoundIdx, 0);
        }
      }
      return uSoundIdx;
  };
}

//----- (004A9BBD) --------------------------------------------------------
int SoundList::LoadSound(unsigned int a2, LPVOID lpBuffer, int uBufferSizeLeft, int *pOutSoundSize, int a6)
{
  void *v18; // ebx@19
  DWORD NumberOfBytesRead;

  if (!sNumSounds)
    return 0;

  for ( uint i = 0; i < sNumSounds; ++i )
  {
    if ( a2 == pSL_Sounds[i].uSoundID )
    {
      if ( !a6 && pSL_Sounds[i].pSoundData )
        return i;
      for ( uint j = 0; j < (signed int)pAudioPlayer->uNumSoundHeaders; ++j )
      {
        if ( !_stricmp(pAudioPlayer->pSoundHeaders[j].pSoundName, pSL_Sounds[i].pSoundName) )
        {
          if ( (signed int)pAudioPlayer->pSoundHeaders[j].uDecompressedSize > uBufferSizeLeft )
            Error("Sound %s is size %i bytes, sound buffer size is %i bytes", pSL_Sounds[i].pSoundName, pAudioPlayer->pSoundHeaders[j].uDecompressedSize, uBufferSizeLeft);
          SetFilePointer(pAudioPlayer->hAudioSnd, pAudioPlayer->pSoundHeaders[j].uFileOffset, 0, 0);
          if ( (signed int)pAudioPlayer->pSoundHeaders[j].uCompressedSize >= (signed int)pAudioPlayer->pSoundHeaders[j].uDecompressedSize )
          {
            if ( pAudioPlayer->pSoundHeaders[j].uCompressedSize == pAudioPlayer->pSoundHeaders[j].uDecompressedSize )
              ReadFile(pAudioPlayer->hAudioSnd, lpBuffer, pAudioPlayer->pSoundHeaders[j].uDecompressedSize, &NumberOfBytesRead, 0);
            else
              MessageBoxW(nullptr, L"Can't load sound file!", L"E:\\WORK\\MSDEV\\MM7\\MM7\\Code\\Sound.cpp:666", 0);
          }
          else
          {
            v18 = malloc(pAudioPlayer->pSoundHeaders[j].uCompressedSize);
            ReadFile(pAudioPlayer->hAudioSnd, v18, pAudioPlayer->pSoundHeaders[j].uCompressedSize, &NumberOfBytesRead, 0);
            zlib::MemUnzip(lpBuffer, &pAudioPlayer->pSoundHeaders[j].uDecompressedSize, v18, pAudioPlayer->pSoundHeaders[j].uCompressedSize);
            free(v18);
          }
          pSL_Sounds[i].pSoundData[a6] = (SoundData *)lpBuffer;
          *pOutSoundSize = a2;
          return i;
        }
      }
    }
  }
  return 0;
}

//----- (004A9D3E) --------------------------------------------------------
SoundDesc *SoundList::Release()
{
  SoundDesc *result; // eax@3
  //void *v5; // ecx@3

  if ( (signed int)this->sNumSounds > 0 )
  {
    for ( uint i = 0; i < (signed int)this->sNumSounds; ++i )
    {
      result = this->pSL_Sounds;
      //v5 = this->pSL_Sounds[i].pSoundData[0];
      if ( this->pSL_Sounds[i].pSoundData[0] )
      {
        ReleaseSoundData(this->pSL_Sounds[i].pSoundData[0]);
        this->pSL_Sounds[i].pSoundData[0] = nullptr;
        this->pSL_Sounds[i].uFlags &= 0xFFFFFFFE;//~0x00000001
      }
    }
  }
  return result;
}

//----- (004A9D79) --------------------------------------------------------
void SoundList::_4A9D79(int a2)
{
  for ( int i = 0; i < (signed int)this->sNumSounds; ++i )
  {
    if ( this->pSL_Sounds[i].eType != SOUND_DESC_SYSTEM && (a2 || this->pSL_Sounds[i].eType != SOUND_DESC_LOCK) )
    {
      if ( this->pSL_Sounds[i].pSoundData[0] )
      {
        ReleaseSoundData(this->pSL_Sounds[i].pSoundData[0]);
        this->pSL_Sounds[i].pSoundData[0] = nullptr;
      }
      this->pSL_Sounds[i].uFlags &= ~SOUND_DESC_SYSTEM;
    }
  }
}

//----- (004A9DCD) --------------------------------------------------------
void SoundList::UnloadSound(unsigned int uSoundID, char a3)
{
  if ( pSL_Sounds[uSoundID].eType != SOUND_DESC_SYSTEM )
  {
    if ( (pSL_Sounds[uSoundID].uFlags & SOUND_DESC_SWAP) && pSL_Sounds[uSoundID].p3DSound && a3 )
    {
      if ( pSL_Sounds[uSoundID].bDecompressed)
        AIL_mem_free_lock(pSL_Sounds[uSoundID].p3DSound);
      pSL_Sounds[uSoundID].p3DSound = 0;
      pSL_Sounds[uSoundID].uFlags &= ~SOUND_DESC_SYSTEM;
    }
    if ( pSL_Sounds[uSoundID].pSoundData[0] )
    {
      ReleaseSoundData(pSL_Sounds[uSoundID].pSoundData[0]);
      pSL_Sounds[uSoundID].pSoundData[0] = nullptr;
      pSL_Sounds[uSoundID].uFlags &= ~SOUND_DESC_SYSTEM;
    }
  }
}


//----- (004A9E3D) --------------------------------------------------------
void SoundList::ToFile()
{
  FILE *v2; // eax@1

  v2 = fopen("data\\dsounds.bin", "wb");
  if ( !v2 )
    Error("Unable to save dsounds.bin!");

  fwrite(this, 4, 1, v2);
  fwrite(this->pSL_Sounds, 0x78u, this->sNumSounds, v2);
  fclose(v2);
}

//----- (004A9E89) --------------------------------------------------------
void SoundList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8)
{
  uint num_mm6_sounds = data_mm6 ? *(int *)data_mm6 : 0,
       num_mm7_sounds = data_mm7 ? *(int *)data_mm7 : 0,
       num_mm8_sounds = data_mm8 ? *(int *)data_mm8 : 0;

  sNumSounds = num_mm6_sounds + num_mm7_sounds + num_mm8_sounds;
  assert(sNumSounds);
  assert(!num_mm8_sounds);

  pSL_Sounds = (SoundDesc *)malloc(sNumSounds * sizeof(SoundDesc));
  memcpy(pSL_Sounds, (char *)data_mm7 + 4, num_mm7_sounds * sizeof(SoundDesc));
  for (uint i = 0; i < num_mm6_sounds; ++i)
  {
    auto src = (SoundDesc_mm6 *)((char *)data_mm6 + 4) + i;
    SoundDesc* dst = &pSL_Sounds[num_mm7_sounds + i];

    memcpy(dst, src, sizeof(SoundDesc_mm6));
    dst->p3DSound = nullptr;
    dst->bDecompressed = false;
  }
}

//----- (004A9ED0) --------------------------------------------------------
int SoundList::FromFileTxt(const char *Args)
{
  __int32 v3; // edi@1
  FILE *v4; // eax@1
  unsigned int v5; // esi@3
  void *v6; // eax@9
  FILE *v7; // ST0C_4@11
  char *i; // eax@11
  char Buf; // [sp+Ch] [bp-2F0h]@3
  FrameTableTxtLine v18; // [sp+200h] [bp-FCh]@4
  FrameTableTxtLine v19; // [sp+27Ch] [bp-80h]@4
  FILE *File; // [sp+2F8h] [bp-4h]@1
  unsigned int Argsa; // [sp+304h] [bp+8h]@3

  free(this->pSL_Sounds);
  v3 = 0;
  this->pSL_Sounds = 0;
  this->sNumSounds = 0;
  v4 = fopen(Args, "r");
  File = v4;
  if ( !v4 )
    Error("SoundListClass::load - Unable to open file: %s.");

  v5 = 0;
  Argsa = 0;
  if ( fgets(&Buf, 490, v4) )
  {
    do
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v19, txt_file_frametable_parser(&Buf, &v18), sizeof(v19));
      if ( v19.uPropCount && *v19.pProperties[0] != 47 )
        ++Argsa;
    }
    while ( fgets(&Buf, 490, File) );
    v5 = Argsa;
    v3 = 0;
  }
  this->sNumSounds = v5;
  v6 = malloc(120 * v5);
  this->pSL_Sounds = (SoundDesc *)v6;
  if ( v6 == (void *)v3 )
    Error("SoundListClass::load - Out of Memory!");

  memset(v6, v3, 120 * this->sNumSounds);
  v7 = File;
  this->sNumSounds = v3;
  fseek(v7, v3, v3);
  for ( i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File) )
  {
    *strchr(&Buf, 10) = 0;
    memcpy(&v19, txt_file_frametable_parser(&Buf, &v18), sizeof(v19));
    if ( v19.uPropCount && *v19.pProperties[0] != 47 )
    {
      sprintf(this->pSL_Sounds[this->sNumSounds].pSoundName, "%s", v19.pProperties[0]);
      this->pSL_Sounds[this->sNumSounds].uSoundID = atoi(v19.pProperties[1]);
      if ( _stricmp(v19.pProperties[2], "system") )
      {
        if ( _stricmp(v19.pProperties[2], "swap") )
        {
          if ( !_stricmp(v19.pProperties[2], "lock") )
            this->pSL_Sounds[this->sNumSounds].eType = SOUND_DESC_LOCK;
          else
            this->pSL_Sounds[this->sNumSounds].eType = SOUND_DESC_LEVEL;
        }
        else
          this->pSL_Sounds[this->sNumSounds].eType = SOUND_DESC_SWAP;
      }
      else
        this->pSL_Sounds[this->sNumSounds].eType = SOUND_DESC_SYSTEM;
      if ( v19.uPropCount >= 4 && !_stricmp(v19.pProperties[3], "3D") )
        this->pSL_Sounds[this->sNumSounds].uFlags |= SOUND_DESC_SWAP;
      ++this->sNumSounds;
    }
  }
  fclose(File);
  return 1;
}
//char to wchar_t
bool char2wchar_t(char *str1, wchar_t *str2)
{
  std::wstringstream st;
  st << str1;
  return !(st >> str2).fail();
}
//----- (004AA13F) --------------------------------------------------------
void AudioPlayer::PlayMusicTrack(MusicID eTrack)
{
  if (!bNoSound && bPlayerReady && uMusicVolimeMultiplier)
  {
    if ( use_music_folder )
    {
      alSourceStop(mSourceID);

      char string[256];
      sprintf(string, "Music\\%d.mp3", eTrack);

      if (!FileExists(string))
      {
        Log::Warning(L"Music\\%d.mp3 not found", eTrack);
        return;
      }
      LPWSTR wStr = new WCHAR[255];
      char2wchar_t(string, wStr);
      PlayAudio(wStr);
      delete [] wStr;
      alSourcef(mSourceID, AL_GAIN, pSoundVolumeLevels[uMusicVolimeMultiplier]);
    }
    else if ( hAILRedbook )
    {
      AIL_redbook_stop(hAILRedbook);
      AIL_redbook_set_volume(hAILRedbook, 64.0f * pSoundVolumeLevels[uMusicVolimeMultiplier]);
      AIL_redbook_track_info(hAILRedbook, eTrack, &uCurrentMusicTrackStartMS, &uCurrentMusicTrackEndMS);
      AIL_redbook_play(hAILRedbook, uCurrentMusicTrackStartMS + 1, uCurrentMusicTrackEndMS);
      uCurrentMusicTrackLength = ((uCurrentMusicTrackEndMS - uCurrentMusicTrackStartMS) * 128) / 1000;
    }
  }
}

//----- (004AA1F3) --------------------------------------------------------
void AudioPlayer::SetMusicVolume(int vol)
{
  if (bPlayerReady)
  {
    if (hAILRedbook)
      AIL_redbook_set_volume(hAILRedbook, vol);
  }
}

//----- (004AA214) --------------------------------------------------------
void AudioPlayer::SetMasterVolume(float fVolume)
{
  if ( bPlayerReady )
  {
    uMasterVolume = fVolume;
    if ( hDigDriver )
      AIL_set_digital_master_volume(hDigDriver, fVolume);
    if ( b3DSoundInitialized )
      s3DSoundVolume = fVolume * 0.5f;
  }
}
// 4D8304: using guessed type int __stdcall AIL_set_digital_master_volume(int, int);

//----- (004AA258) --------------------------------------------------------
void AudioPlayer::_4AA258(int a2)
{
  if (!bPlayerReady)
    return;

  if ( this->b3DSoundInitialized && a2 )
  {
    for ( uint i = 0; i < this->uNum3DSamples; ++i )
    {
      if ( this->p3DSamples[i].field_4 == a2 && AIL_3D_sample_status(this->p3DSamples[i].hSample) == AIL::Sample::Playing )
        AIL_end_3D_sample(this->p3DSamples[i].hSample);
    }
  }
  if ( this->hDigDriver && a2 )
  {
    for ( uint i = 0; i < this->uMixerChannels; ++i )
    {
      if ( this->pMixerChannels[i].source_pid == a2 && AIL_sample_status(this->pMixerChannels[i].hSample) == AIL::Sample::Playing)
      {
        AIL_end_sample(this->pMixerChannels[i].hSample);
        FreeChannel(&this->pMixerChannels[i]);
      }
    }
  }
}

//----- (004AA306) --------------------------------------------------------
void AudioPlayer::PlaySound(SoundID eSoundID, signed int pid, unsigned int uNumRepeats, signed int source_x, signed int source_y, int sound_data_id, float uVolume, int sPlaybackRate)
{
  int v12; // edi@13
  signed int v13; // ecx@17
  signed int v14; // eax@20
  int v15; // eax@24
  signed int v16; // eax@25
  SpriteObject *pLayingItem; // eax@28
  signed int v18; // eax@29
  Actor *pActor1; // eax@32
  signed int v20; // ecx@32
  double v21; // st7@32
  signed int v22; // ecx@33
  AudioPlayer_3DSample *pAudioPlayer_3DSample; // esi@53
  AudioPlayer_3DSample *pAudioPlayer_3DSample1; // esi@61
  int v25; // esi@67
  double v26; // st7@68
  int v27; // ST18_4@68
  int v28; // ebx@68
  int v29; // eax@68
  AudioPlayer_3DSample *pAudioPlayer_3DSample2; // esi@69
  int v31; // ST18_4@70
  int v32; // ebx@70
  int v33; // eax@70
  int v34; // eax@70
  char v35; // zf@70
  signed int v36; // ebx@74
  AudioPlayer_3DSample *pAudioPlayer_3DSample3; // esi@79
  int v40; // eax@81
  char *v41; // edi@82
  int v42; // esi@82
  double v43; // st7@91
  SpriteObject *pLayingItem2; // eax@92
  Actor *pActor; // eax@93
  signed int v46; // ecx@93
  double v47; // st7@93
  BLVDoor *pBLVDoor; // eax@97
  double v49; // st7@104
  int v50; // ST18_4@104
  int v51; // ebx@104
  int v52; // eax@104
  float v53; // ST0C_4@106
  float v54; // ST04_4@106
  signed int v62; // esi@133
  unsigned int v86; // [sp+14h] [bp-60h]@84
  RenderVertexSoft pRenderVertexSoft; // [sp+24h] [bp-50h]@1
  int v90; // [sp+58h] [bp-1Ch]@68
  float v91; // [sp+5Ch] [bp-18h]@68
  float v93; // [sp+64h] [bp-10h]@1
  signed int varC; // [sp+68h] [bp-Ch]@68
  int v96; // [sp+70h] [bp-4h]@19
  signed int uNumRepeatsb; // [sp+84h] [bp+10h]@93
  float uNumRepeatsa; // [sp+84h] [bp+10h]@104
  float v99; // [sp+8Ch] [bp+18h]@104
  signed int v100; // [sp+90h] [bp+1Ch]@32
  int v101; // [sp+90h] [bp+1Ch]@52
  int v102; // [sp+90h] [bp+1Ch]@60
  int v103; // [sp+90h] [bp+1Ch]@68

  if ( !bPlayerReady || !uSoundVolumeMultiplier || !hDigDriver || eSoundID == SOUND_Invalid )
    return;

  float sample_volume = 10000;

  int sound_id = 0;
  for (uint i = 0; i < pSoundList->sNumSounds; ++i)
    if (pSoundList->pSL_Sounds[i].uSoundID == eSoundID)
    {
      sound_id = i;
      break;
    }

  if (!sound_id)
  {
    Log::Warning(L"SoundID = %u not found", eSoundID);
    return;
  }
  assert(sound_id < pSoundList->sNumSounds);
  if ( !sound_data_id )
  {
    if ( !pSoundList->pSL_Sounds[sound_id].pSoundData[0] )
    {
      if ( pSoundList->pSL_Sounds[sound_id].eType == SOUND_DESC_SWAP )
        pSoundList->LoadSound(eSoundID, 0);
    }
  }
  if ( !pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] )
    return;

  int start_channel = 0,
      end_channel = 0;
  v62 = start_channel;

  if (!b3DSoundInitialized || pSoundList->pSL_Sounds[sound_id].Is3D())
  {
    if (pid == 0)  // generic sound like from UI
    {
      start_channel = 10;
      end_channel = 12;
      for (uint i = start_channel; i <= end_channel; ++i)
      {
        if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
        {
          if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
            return;                          // already playing the same sound from the same source - return
          AIL_end_sample(pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
          FreeChannel(&pMixerChannels[i]);
        }
      }
      for ( uint j = start_channel; j <= end_channel; j++ )
      {
        if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
        {
          AIL_init_sample(pMixerChannels[j].hSample);
          AIL_set_sample_file(pMixerChannels[j].hSample, pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id]->pData, -1);
          if ( sample_volume == 10000 )
            sample_volume = uMasterVolume;
          if (uVolume)
            sample_volume = uVolume;
          int object_type = PID_TYPE(pid),
              object_id = PID_ID(pid);
          if (source_x != -1)//срабатывает например у форта в Хермондейле звук выстрелов пушек
          {
            //if (!source_x)
              //source_x = pParty->vPosition.x;
            //if (!source_y)
              //source_y = pParty->vPosition.y;
           if ( source_x )//Ritor1: for pedestals
           {
              AIL_set_sample_pan(pMixerChannels[j].hSample, sub_4AB66C(source_x, source_y));
              int vol = GetSoundStrengthByDistanceFromParty(source_x, source_y, pParty->vPosition.z);
              AIL_set_sample_volume(pMixerChannels[j].hSample, vol);
           }
          }
          if (uNumRepeats)
            AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
          pMixerChannels[j].uSourceTrackIdx = sound_id;
          pMixerChannels[j].source_pid = pid;
          pMixerChannels[j].uSourceTrackID = eSoundID;
          int rval = AIL_start_sample(pMixerChannels[j].hSample);
          if ( sPlaybackRate )
            AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
          if (object_type == OBJECT_Player)
            AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
          return;
        }
      }
      return;
    }
    else if (pid == -1)  // exclusive sounds - can override
    {
      /*if ( AIL_sample_status(pMixerChannels[13].hSample) == AIL::Sample::Done )
      {
          AIL_end_sample(pMixerChannels[13].hSample);
          if ( pMixerChannels[13].uSourceTrackIdx )
            FreeChannel(&pMixerChannels[13]);
      }*/
      AIL_init_sample(pMixerChannels[13].hSample);
      AIL_set_sample_file(pMixerChannels[13].hSample, pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id]->pData, -1);
      if ( sample_volume == 10000 )
        sample_volume = uMasterVolume;
      if (uVolume)
        sample_volume = uVolume;
      AIL_set_sample_volume(pMixerChannels[13].hSample, sample_volume);
      int object_type = PID_TYPE(pid),
        object_id = PID_ID(pid);
      if (uNumRepeats)
        AIL_set_sample_loop_count(pMixerChannels[13].hSample, uNumRepeats - 1);
      pMixerChannels[13].uSourceTrackIdx = sound_id;
      pMixerChannels[13].source_pid = pid;
      pMixerChannels[13].uSourceTrackID = eSoundID;
      int rval = AIL_start_sample(pMixerChannels[13].hSample);//no sound chest close 
      if ( sPlaybackRate )
        AIL_set_sample_playback_rate(pMixerChannels[13].hSample, sPlaybackRate);
      if (object_type == OBJECT_Player)
        AIL_sample_ms_position(pMixerChannels[13].hSample, &sLastTrackLengthMS, 0);
      return;
    }
    else if (pid < 0)    // exclusive sounds - no override (close chest)
    {
      start_channel = 14;
      end_channel = 14;
      for (uint i = start_channel; i <= end_channel; ++i)
      {
        if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
        {
          if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
            return;                          // already playing the same sound from the same source - return
          AIL_end_sample(pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
          FreeChannel(&pMixerChannels[i]);
        }
      }
      for ( uint j = start_channel; j <= end_channel; j++ )
      {
        if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
        {
          AIL_init_sample(pMixerChannels[j].hSample);
          char *p = (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id];
          if (sound_data_id == 0)  
			  p = p + 4;//for RIFF
          AIL_set_sample_file(pMixerChannels[j].hSample, p, -1);
          if ( sample_volume == 10000 )
            sample_volume = uMasterVolume;
          if (uVolume)
            sample_volume = uVolume;
          int object_type = PID_TYPE(pid),
              object_id = PID_ID(pid);
          if (uNumRepeats)
            AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
          pMixerChannels[j].uSourceTrackIdx = sound_id;
          pMixerChannels[j].source_pid = pid;
          pMixerChannels[j].uSourceTrackID = eSoundID;
          int rval = AIL_start_sample(pMixerChannels[j].hSample);//no sound chest close 
          if ( sPlaybackRate )
            AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
          if (object_type == OBJECT_Player)
            AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
          return;
        }
      }
      return;
    }
    else
    {
        int object_type = PID_TYPE(pid),
            object_id = PID_ID(pid);
        switch (object_type)
        {
          case OBJECT_BLVDoor:
          {
            assert(uCurrentlyLoadedLevelType == LEVEL_Indoor);
            assert(object_id < pIndoor->uNumDoors);
            if ( !pIndoor->pDoors[object_id].uDoorID )
              return;

            start_channel = 10;
            end_channel = 12;
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
              {
                if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample( pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume)
                  sample_volume = uVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                /*if (!GetSoundStrengthByDistanceFromParty(pIndoor->pDoors[object_id].pXOffsets[0], pIndoor->pDoors[object_id].pYOffsets[0], pIndoor->pDoors[object_id].pZOffsets[0]))
                {
                  AIL_end_sample(pMixerChannels[j].hSample);
                  FreeChannel(&pMixerChannels[j]);
                  return;
                } */
                AIL_set_sample_pan(pMixerChannels[j].hSample, sub_4AB66C(pIndoor->pDoors[object_id].pXOffsets[0], pIndoor->pDoors[object_id].pYOffsets[0]));
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          case OBJECT_Player:
          {
            start_channel = 10;
            end_channel = 12;
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
              {
                if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample( pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume)
                  sample_volume = uVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          case OBJECT_Actor:
          {
            start_channel = 0;
            end_channel = 3;
            assert(object_id < uNumActors);
            sample_volume = GetSoundStrengthByDistanceFromParty(pActors[object_id].vPosition.x, pActors[object_id].vPosition.y, pActors[object_id].vPosition.z);
            if (!sample_volume)
              return;
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
              {
                if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample( pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume)
                  sample_volume = uVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                if (!GetSoundStrengthByDistanceFromParty(pActors[object_id].vPosition.x, pActors[object_id].vPosition.y, pActors[object_id].vPosition.z))
                  return;
                AIL_set_sample_pan(pMixerChannels[j].hSample, sub_4AB66C(pActors[object_id].vPosition.x, pActors[object_id].vPosition.y));
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          case OBJECT_Decoration:
          {
            start_channel = 4;
            end_channel = 4;
            assert(object_id < uNumLevelDecorations);
            sample_volume = GetSoundStrengthByDistanceFromParty(pLevelDecorations[object_id].vPosition.x, pLevelDecorations[object_id].vPosition.y, pLevelDecorations[object_id].vPosition.z);
            if (sample_volume == 0.0)
              return;
			if ( pid > 0  )
			{
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )//звук фонтана и шагов не проходят проверку на повтор
              {
                if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample(pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
			}
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume == 0.0)
                  sample_volume = uMasterVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                if (!GetSoundStrengthByDistanceFromParty(pLevelDecorations[object_id].vPosition.x, pLevelDecorations[object_id].vPosition.y, pLevelDecorations[object_id].vPosition.z))
                  return;
				AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                AIL_set_sample_pan(pMixerChannels[j].hSample, sub_4AB66C(pLevelDecorations[object_id].vPosition.x, pLevelDecorations[object_id].vPosition.y));
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          case OBJECT_Item:
          {
            start_channel = 5;
            end_channel = 7;
            assert(object_id < uNumSpriteObjects);
            sample_volume = GetSoundStrengthByDistanceFromParty(pSpriteObjects[object_id].vPosition.x, pSpriteObjects[object_id].vPosition.y, pSpriteObjects[object_id].vPosition.z);
            if (!sample_volume)
              return;
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
              {
                if (pMixerChannels[i].uSourceTrackIdx == sound_id)
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample(pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume)
                  sample_volume = uVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                if (!GetSoundStrengthByDistanceFromParty(pSpriteObjects[object_id].vPosition.x, pSpriteObjects[object_id].vPosition.y, pSpriteObjects[object_id].vPosition.z) )
                  return;
                AIL_set_sample_pan(pMixerChannels[v62].hSample, sub_4AB66C(pSpriteObjects[object_id].vPosition.x, pSpriteObjects[object_id].vPosition.y));
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          case OBJECT_BModel:
          {
            start_channel = 8;
            end_channel = 9;
            for (uint i = start_channel; i <= end_channel; ++i)
            {
              if ( pMixerChannels[i].source_pid == pid && AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Playing )
              {
                if ( pMixerChannels[i].uSourceTrackIdx == sound_id )
                  return;                          // already playing the same sound from the same source - return
                AIL_end_sample(pMixerChannels[i].hSample);  // requested new sound from the same source - end & switch
                FreeChannel(&pMixerChannels[i]);
              }
            }
            for ( uint j = start_channel; j <= end_channel; j++ )
            {
              if ( AIL_sample_status(pMixerChannels[j].hSample) == AIL::Sample::Done )
              {
                AIL_init_sample(pMixerChannels[j].hSample);
                AIL_set_sample_file(pMixerChannels[j].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
                if (uVolume)
                  sample_volume = uVolume;
                AIL_set_sample_volume(pMixerChannels[j].hSample, sample_volume);
                if (uNumRepeats)
                  AIL_set_sample_loop_count(pMixerChannels[j].hSample, uNumRepeats - 1);
                pMixerChannels[j].uSourceTrackIdx = sound_id;
                pMixerChannels[j].source_pid = pid;
                pMixerChannels[j].uSourceTrackID = eSoundID;
                int rval = AIL_start_sample(pMixerChannels[j].hSample);
                if ( sPlaybackRate )
                  AIL_set_sample_playback_rate(pMixerChannels[j].hSample, sPlaybackRate);
                if (object_type == OBJECT_Player)
                  AIL_sample_ms_position(pMixerChannels[j].hSample, &sLastTrackLengthMS, 0);
                return;
              }
            }
          }
          return;

          default:
            assert(false);
        }
      }

	  __debugbreak();//Ritor1
      if (start_channel > end_channel)  // no free channel - occupy the quitest one
      {
        start_channel = -1;
        int min_volume = sample_volume;
        for (uint i = start_channel; i <= end_channel; ++i)
        {
          int volume = AIL_sample_volume(pMixerChannels[i].hSample);
          if (volume < min_volume)
          {
            min_volume = volume;
            start_channel = i;
          }
        }
        if (v62 == -1)   // no free channels at all - only channel 13 allows override (a3 == -1)
        {
          if (start_channel != 13)
            return;
          v62 = 13;
        }
        AIL_end_sample(pMixerChannels[v62].hSample);
        FreeChannel(&pMixerChannels[v62]);
      }

      if (v62 > end_channel)//10!=13
        return;

      if ( sample_volume == 10000 )
        sample_volume = uMasterVolume;

      AIL_init_sample(pMixerChannels[v62].hSample);
      AIL_set_sample_file(pMixerChannels[v62].hSample, (char *)pSoundList->pSL_Sounds[sound_id].pSoundData[sound_data_id] + 4 * (sound_data_id == 0), -1);
      if (uVolume)
        sample_volume = uVolume;
      AIL_set_sample_volume(pMixerChannels[v62].hSample, sample_volume);

      int object_type = PID_TYPE(pid),
          object_id = PID_ID(pid);
      if (source_x != -1)
      {
        if (!source_x)
          source_x = pParty->vPosition.x;
        if (!source_y)
          source_y = pParty->vPosition.y;
        AIL_set_sample_pan(pMixerChannels[v62].hSample, sub_4AB66C(source_x, source_y));
        AIL_set_sample_volume(pMixerChannels[v62].hSample, GetSoundStrengthByDistanceFromParty(source_x, source_y, pParty->vPosition.z));
      }

      if (uNumRepeats)
        AIL_set_sample_loop_count(pMixerChannels[v62].hSample, uNumRepeats - 1);
      pMixerChannels[v62].uSourceTrackIdx = sound_id;
      pMixerChannels[v62].source_pid = pid;
      pMixerChannels[v62].uSourceTrackID = eSoundID;
      int rval = AIL_start_sample(pMixerChannels[v62].hSample);
      if ( sPlaybackRate )
        AIL_set_sample_playback_rate(pMixerChannels[v62].hSample, sPlaybackRate);
      if (object_type == OBJECT_Player)
        AIL_sample_ms_position(pMixerChannels[v62].hSample, &sLastTrackLengthMS, 0);
      return; 
  }
  else
  {
  __debugbreak(); // 3d sound stuff, refactor
  v12 = 13;
  if ( pid < 0 )
  {
    v15 = pAudioPlayer->uNum3DSamples;
    if ( pid == -1 )
    {
      if ( v15 < 16 )
        v12 = v15 - 1;
      v96 = v12;
      //goto LABEL_46;
	  pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
      pRenderVertexSoft.vWorldPosition.y = (double)pParty->vPosition.y;
      v21 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
      goto LABEL_47;
    }
    if ( v15 >= 16 )
      v15 = 14;
    v12 = v15;
    //goto LABEL_45;
	v96 = v15;
    pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
    pRenderVertexSoft.vWorldPosition.y = (double)pParty->vPosition.y;
    v21 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
    goto LABEL_47;
  }
  if ( PID_TYPE(pid) == 2 )
  {
    v22 = pAudioPlayer->uNum3DSamples;
    if ( v22 < 16 )
    {
      v12 = 5 * v22 / 16;
      v96 = 7 * v22 / 16;
    }
    else
    {
      v96 = 7;
      v12 = 5;
    }
    pLayingItem = &pSpriteObjects[PID_ID(pid)];
  }
  else
  {
    if ( PID_TYPE(pid) == 3 )
    {
      v18 = pAudioPlayer->uNum3DSamples;
      v12 = 0;
      if ( v18 < 16 )
        v96 = 3 * v18 / 16;
      else
        v96 = 3;
      pActor1 = &pActors[PID_ID(pid)];
      v20 = pActor1->vPosition.y;
      pRenderVertexSoft.vWorldPosition.x = (double)pActor1->vPosition.x;
      v100 = pActor1->vPosition.z;
      pRenderVertexSoft.vWorldPosition.y = (double)v20;
      v21 = (double)v100;
      goto LABEL_47;
    }
    if ( PID_TYPE(pid) != 5 )
    {
      v13 = pAudioPlayer->uNum3DSamples;
      if ( PID_TYPE(pid) == 6 )
      {
        if ( v13 >= 16 )
        {
          v96 = 9;
          v12 = 8;
          //goto LABEL_46;
		  pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
          pRenderVertexSoft.vWorldPosition.y = (double)pParty->vPosition.y;
		  v21 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
		  goto LABEL_47;
        }
        v12 = 8 * v13 / 16;
        v14 = 9 * v13;
      }
      else
      {
        if ( v13 >= 16 )
        {
          v96 = 12;
          v12 = 10;
//LABEL_46:
          pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
          pRenderVertexSoft.vWorldPosition.y = (double)pParty->vPosition.y;
          v21 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
          goto LABEL_47;
        }
        v12 = 10 * v13 / 16;
        v14 = 12 * v13;
      }
      v15 = v14 / 16;
//LABEL_45:
      v96 = v15;
      //goto LABEL_46;
	  pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
      pRenderVertexSoft.vWorldPosition.y = (double)pParty->vPosition.y;
      v21 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
      goto LABEL_47;
    }
    v16 = pAudioPlayer->uNum3DSamples;
    if ( v16 < 16 )
    {
      v12 = v16 / 4;
      v96 = v16 / 4;
    }
    else
    {
      v12 = 4;
      v96 = 4;
    }
    pLayingItem = (SpriteObject *)&pLevelDecorations[PID_ID(pid)];
  }
  pRenderVertexSoft.vWorldPosition.x = (double)pLayingItem->vPosition.x;
  pRenderVertexSoft.vWorldPosition.y = (double)pLayingItem->vPosition.y;
  v21 = (double)pLayingItem->vPosition.z;
LABEL_47:
  pRenderVertexSoft.vWorldPosition.z = v21;
  if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
    sub_4AAEA6_transform(&pRenderVertexSoft);
  else
    pIndoorCameraD3D->ViewTransform(&pRenderVertexSoft, 1);
  if ( pid )
  {
    if ( pid != -1 )
    {
      v101 = 0;
      if ( pAudioPlayer->uNum3DSamples > 0 )
      {
        pAudioPlayer_3DSample = pAudioPlayer->p3DSamples;
        do
        {
          if ( AIL_3D_sample_status(pAudioPlayer_3DSample->hSample) == 4 
			  && pAudioPlayer_3DSample->field_4 == pid && AIL_3D_sample_status(pAudioPlayer_3DSample->hSample) == 4 )
          {
            if ( pAudioPlayer_3DSample->field_8 == sound_id )
              return;
            AIL_end_3D_sample(pAudioPlayer_3DSample->hSample);
            pAudioPlayer->_4ABF23(pAudioPlayer_3DSample);
          }
          ++v101;
          ++pAudioPlayer_3DSample;
        }
        while ( v101 < pAudioPlayer->uNum3DSamples );
      }
    }
  }
  v102 = v12;
  if ( v12 <= v96 )
  {
    pAudioPlayer_3DSample1 = &pAudioPlayer->p3DSamples[v12];
    while ( AIL_3D_sample_status(pAudioPlayer_3DSample1->hSample) != 2 )
    {
      ++v102;
      ++pAudioPlayer_3DSample1;
      if ( v102 > v96 )
        goto LABEL_67;
    }
    AIL_end_3D_sample(pAudioPlayer_3DSample1->hSample);
    if ( pAudioPlayer_3DSample1->field_8 )
      pAudioPlayer->_4ABF23(pAudioPlayer_3DSample1);
  }
LABEL_67:
  v25 = v96;
  if ( v102 == v96 + 1 )
  {
    LODWORD(v91) = -1;
    v103 = 0;
    *(float *)&varC = pRenderVertexSoft.vWorldViewPosition.y * -0.012207031;
    v93 = 0.0;
    v26 = pRenderVertexSoft.vWorldViewPosition.x * 0.012207031;
    *(float *)&uVolume = v26;
    v27 = abs((signed __int64)v26);
    v28 = abs((signed __int64)v93);
    v29 = abs((signed __int64)*(float *)&varC);
    v90 = int_get_vector_length(v29, v28, v27);
    sPlaybackRate = v12;
    if ( v12 > v25 )
      goto LABEL_192;
    pAudioPlayer_3DSample2 = &pAudioPlayer->p3DSamples[v12];
    do
    {
      AIL_3D_position(pAudioPlayer_3DSample2->hSample, &varC, &v93, (long *)&uVolume);
      v31 = abs((signed __int64)*(float *)&uVolume);
      v32 = abs((signed __int64)v93);
      v33 = abs((signed __int64)*(float *)&varC);
      v34 = int_get_vector_length(v33, v32, v31);
      v35 = v103 == v34;
      if ( v103 < v34 )
      {
        v103 = v34;
        v35 = 1;
      }
      if ( v35 && v90 < v103 )
      {
        v36 = sPlaybackRate;
        LODWORD(v91) = sPlaybackRate;
      }
      else
      {
        v36 = LODWORD(v91);
      }
      ++sPlaybackRate;
      ++pAudioPlayer_3DSample2;
    }
    while ( sPlaybackRate <= v96 );
    if ( v36 == -1 )
    {
LABEL_192:
      v36 = 13;
      if ( v12 != 13 )
        return;
    }
    //pAudioPlayer2 = pAudioPlayer;
    pAudioPlayer_3DSample3 = &pAudioPlayer->p3DSamples[v36];
    AIL_end_3D_sample(pAudioPlayer_3DSample3->hSample);
    pAudioPlayer->_4ABF23(pAudioPlayer_3DSample3);
    v102 = v36;
  }
  //v39 = v89;
  if ( pSoundList->pSL_Sounds[sound_id].p3DSound || (LOWORD(v40) = pSoundList->LoadSound(eSoundID, 0), v40) )
  {
    v41 = (char *)pAudioPlayer + 16 * v102;
    v42 = (int)(v41 + 20);
    if ( AIL_set_3D_sample_file(*((int *)v41 + 5), *(void **)((char *)&pSoundList->pSL_Sounds->p3DSound + sound_id * sizeof(SoundDesc))) )
    {
      if ( uNumRepeats )
        v86 = uNumRepeats - 1;
      else
        v86 = 1;
      AIL_set_3D_sample_loop_count(*(int *)v42, v86);
      if ( source_x == -1 )
      {
        if ( PID_TYPE(pid) == 1 )
        {
          if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
          {
			//goto LABEL_103;
            pBLVDoor = &pIndoor->pDoors[PID_ID(pid)];
            if ( !pBLVDoor->uDoorID )
             return;
            pRenderVertexSoft.vWorldPosition.x = (double)*pBLVDoor->pXOffsets;
            pRenderVertexSoft.vWorldPosition.y = (double)*pBLVDoor->pYOffsets;
            v47 = (double)*pBLVDoor->pZOffsets;
LABEL_101:
            pRenderVertexSoft.vWorldPosition.z = v47;
          //if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
          //{
            sub_4AAEA6_transform(&pRenderVertexSoft);
            //goto LABEL_104;
          //}
		  }
//LABEL_103:
		  if ( uCurrentlyLoadedLevelType != LEVEL_Indoor )
            pIndoorCameraD3D->ViewTransform(&pRenderVertexSoft, 1);
//LABEL_104:
          AIL_start_3D_sample(*(int *)v42);
          AIL_set_3D_sample_float_distances(*(int **)v42, 100.0, 20.0, 100.0, 20.0);
          AIL_set_3D_sample_volume(*(int **)v42, pAudioPlayer->s3DSoundVolume);
          v99 = pRenderVertexSoft.vWorldViewPosition.y * -0.012207031;
          v49 = pRenderVertexSoft.vWorldViewPosition.x * 0.012207031;
          uNumRepeatsa = v49;
          v50 = abs((signed __int64)v49);
          v51 = abs(0);
          v52 = abs((signed __int64)v99);
          if ( int_get_vector_length(v52, v51, v50) <= 100 )
          {
            AIL_set_3D_position((void *)*(int *)v42, LODWORD(v99), 0.0, LODWORD(uNumRepeatsa));
            v53 = -uNumRepeatsa;
            v54 = -v99;
            AIL_set_3D_orientation((void *)*(int *)v42, LODWORD(v54), 0.0, LODWORD(v53), 0.0, 1.0, 0.0);
            //pAudioPlayer3 = pAudioPlayer;
            *((int *)v41 + 6) = pid;
            *((int *)v41 + 7) = sound_id;
            *(&pAudioPlayer->bEAXSupported + 4 * (v102 + 2)) = eSoundID;
          }
          else
          {
            AIL_end_3D_sample(*(int **)v42);
            pAudioPlayer->_4ABF23((AudioPlayer_3DSample *)(v41 + 20));
          }
          return;
        }
        if ( PID_TYPE(pid) == 2 )
        {
          pLayingItem2 = &pSpriteObjects[PID_ID(pid)];
        }
        else
        {
          if ( PID_TYPE(pid) == 3 )
          {
            pActor = &pActors[PID_ID(pid)];
            v46 = pActor->vPosition.y;
            pRenderVertexSoft.vWorldPosition.x = (double)pActor->vPosition.x;
            uNumRepeatsb = pActor->vPosition.z;
            pRenderVertexSoft.vWorldPosition.y = (double)v46;
            v47 = (double)uNumRepeatsb;
            goto LABEL_101;
          }
          if ( PID_TYPE(pid) != 5 )
          {
            pRenderVertexSoft.vWorldPosition.x = (double)pParty->vPosition.x;
            v43 = (double)pParty->vPosition.y;
            pRenderVertexSoft.vWorldPosition.y = v43;
            v47 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
            goto LABEL_101;
          }
          pLayingItem2 = (SpriteObject *)&pLevelDecorations[PID_ID(pid)];
        }
        pRenderVertexSoft.vWorldPosition.x = (double)pLayingItem2->vPosition.x;
        pRenderVertexSoft.vWorldPosition.y = (double)pLayingItem2->vPosition.y;
        v47 = (double)pLayingItem2->vPosition.z;
        goto LABEL_101;
      }
      pRenderVertexSoft.vWorldPosition.x = (double)source_x;
      v43 = (double)source_y;
      pRenderVertexSoft.vWorldPosition.y = v43;
      v47 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
      goto LABEL_101;
    }
  }
  }
}

//----- (0040DEA5) --------------------------------------------------------
void  AudioPlayer::MessWithChannels()
{
  pAudioPlayer->StopChannels(-1, -1);
}


//----- (004AAFCF) --------------------------------------------------------
void AudioPlayer::UpdateSounds()
{
  int v2; // ebx@1
  int v7; // ebx@6
  int v8; // ebx@9
  int v9; // ebx@10
  int v10; // ebx@11
  double v11; // st7@13
  SpriteObject *v12; // eax@14
  Actor *v13; // eax@15
  BLVDoor *pDoor; // eax@19
  double v16; // st7@22
  double v17; // st6@22
  double v18; // st5@23
  double v19; // st4@24
  double v20; // st3@24
  double v21; // st6@28
  double v22; // st7@32
  int v23; // ST1C_4@32
  int v24; // ebx@32
  int v25; // eax@32
  float v26; // ST10_4@34
  float v27; // ST08_4@34
  signed int v53; // eax@88
  RenderVertexSoft a1; // [sp+24h] [bp-48h]@1
  float v55; // [sp+54h] [bp-18h]@22
  float v56; // [sp+58h] [bp-14h]@22
  int uNumRepeats; // [sp+5Ch] [bp-10h]@15
  float v58; // [sp+60h] [bp-Ch]@23
  int v59; // [sp+64h] [bp-8h]@4

  v2 = 0;
  if (!bPlayerReady)
    return;
  
  //if (field_2D0_time_left <= pEventTimer->uTimeElapsed)
    //field_2D0_time_left = 32;
  //else
  //{
    //field_2D0_time_left -= pEventTimer->uTimeElapsed;
    //return;
  //}
  field_2D0_time_left -= pEventTimer->uTimeElapsed;
  if ( field_2D0_time_left <= 0 )
  {
  field_2D0_time_left = 32;
  if ( b3DSoundInitialized )//for 3D sound
  {
    __debugbreak(); // refactor refactor
    v2 = 0;
    for ( v59 = 0; v59 < pAudioPlayer->uNum3DSamples; ++v59 )
    {
      v7 = PID_TYPE(this->p3DSamples[v59].field_4);
      if ( AIL_3D_sample_status(this->p3DSamples[v59].hSample) == AIL::Sample::Done )
      {
        AIL_end_3D_sample(this->p3DSamples[v59].hSample);
        pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
      }
      if ( AIL_3D_sample_status(this->p3DSamples[v59].hSample) != AIL::Sample::Playing )
        continue;
      v8 = v7 - 1;//
      if ( v8 )//> 1
      {
        v9 = v8 - 1;//
        if ( v9 )//> 2
        {
          v10 = v9 - 1;//
          if ( !v10 )//3
          {
            v13 = &pActors[PID_ID(this->p3DSamples[v59].field_4)];
            //uNumRepeats = v13->vPosition.x;
            //v14 = v13->vPosition.y;
            a1.vWorldPosition.x = (double)v13->vPosition.x;
            //uNumRepeats = v13->vPosition.z;
            a1.vWorldPosition.y = (double)v13->vPosition.y;
            //v11 = (double)uNumRepeats;
            a1.vWorldPosition.z = v13->vPosition.z;
            if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
            {
              v16 = pIndoorCameraD3D->fRotationXCosine;
              v17 = pIndoorCameraD3D->fRotationXSine;
              v55 = pIndoorCameraD3D->fRotationYCosine;
              v56 = pIndoorCameraD3D->fRotationYSine;
              if (pIndoorCameraD3D->sRotationX)
              {
                v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
                *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
                v18 = a1.vWorldPosition.z - (double)pParty->vPosition.z;
                //if ( pRenderer->pRenderD3D )
                {
                  v19 = *(float *)&uNumRepeats * v56 + v58 * v55;
                  v20 = v58 * v56 - *(float *)&uNumRepeats * v55;
                }
                //else
               // {
                 // v19 = v58 * v55 - *(float *)&uNumRepeats * v56;
                 // v20 = v58 * v56 + *(float *)&uNumRepeats * v55;
                //}
                a1.vWorldViewPosition.x = v19 * v16 - v18 * v17;
                a1.vWorldViewPosition.y = v20;
                a1.vWorldViewPosition.z = v19 * v17 + v18 * v16;
              }
              else
              {
                v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
                *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
                //if ( pRenderer->pRenderD3D )
                {
                  a1.vWorldViewPosition.x = *(float *)&uNumRepeats * v56 + v58 * v55;
                  v21 = v58 * v56 - *(float *)&uNumRepeats * v55;
                }
                //else
                //{
                //  a1.vWorldViewPosition.x = v58 * v55 - *(float *)&uNumRepeats * v56;
                //  v21 = v58 * v56 + *(float *)&uNumRepeats * v55;
                //}
                a1.vWorldViewPosition.y = v21;
                a1.vWorldViewPosition.z = a1.vWorldPosition.z - (double)pParty->vPosition.z;
              }
            }
            else
              pIndoorCameraD3D->ViewTransform(&a1, 1);
            v58 = a1.vWorldViewPosition.y * -0.012207031;
            v22 = a1.vWorldViewPosition.x * 0.012207031;
            *(float *)&uNumRepeats = v22;
            v23 = abs((signed __int64)v22);
            v24 = abs(0);
            v25 = abs((signed __int64)v58);
            if ( int_get_vector_length(v25, v24, v23) <= 100 )
            {
              AIL_set_3D_position(this->p3DSamples[v59].hSample, LODWORD(v58), 0.0, uNumRepeats);
              v26 = -*(float *)&uNumRepeats;
              v27 = -v58;
              AIL_set_3D_orientation(this->p3DSamples[v59].hSample, LODWORD(v27), 0.0, LODWORD(v26), 0.0, 1.0, 0.0);
            }
            else
            {
              AIL_end_3D_sample(this->p3DSamples[v59].hSample);
              pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
            }
            continue;
          }
          if ( v10 != 2 )//4
          {
            a1.vWorldPosition.x = (double)pParty->vPosition.x;
            a1.vWorldPosition.y = (double)pParty->vPosition.y;
            v11 = (double)pParty->sEyelevel + (double)pParty->vPosition.z;
            a1.vWorldPosition.z = v11;
            if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
            {
              v16 = pIndoorCameraD3D->fRotationXCosine;
              v17 = pIndoorCameraD3D->fRotationXSine;
              v55 = pIndoorCameraD3D->fRotationYCosine;
              v56 = pIndoorCameraD3D->fRotationYSine;
              if (pIndoorCameraD3D->sRotationX)
              {
                v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
                *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
                v18 = a1.vWorldPosition.z - (double)pParty->vPosition.z;
                //if ( pRenderer->pRenderD3D )
                {
                  v19 = *(float *)&uNumRepeats * v56 + v58 * v55;
                  v20 = v58 * v56 - *(float *)&uNumRepeats * v55;
                }
                //else
                //{
                 // v19 = v58 * v55 - *(float *)&uNumRepeats * v56;
                 // v20 = v58 * v56 + *(float *)&uNumRepeats * v55;
                //}
                a1.vWorldViewPosition.x = v19 * v16 - v18 * v17;
                a1.vWorldViewPosition.y = v20;
                a1.vWorldViewPosition.z = v19 * v17 + v18 * v16;
              }
              else
              {
                v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
                *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
                //if ( pRenderer->pRenderD3D )
                {
                  a1.vWorldViewPosition.x = *(float *)&uNumRepeats * v56 + v58 * v55;
                  v21 = v58 * v56 - *(float *)&uNumRepeats * v55;
                }
                //else
                //{
                 // a1.vWorldViewPosition.x = v58 * v55 - *(float *)&uNumRepeats * v56;
                 // v21 = v58 * v56 + *(float *)&uNumRepeats * v55;
                //}
                a1.vWorldViewPosition.y = v21;
                a1.vWorldViewPosition.z = a1.vWorldPosition.z - (double)pParty->vPosition.z;
              }
            }
            else
              pIndoorCameraD3D->ViewTransform(&a1, 1);
            v58 = a1.vWorldViewPosition.y * -0.012207031;
            v22 = a1.vWorldViewPosition.x * 0.012207031;
            *(float *)&uNumRepeats = v22;
            v23 = abs((signed __int64)v22);
            v24 = abs(0);
            v25 = abs((signed __int64)v58);
            if ( int_get_vector_length(v25, v24, v23) <= 100 )
            {
              AIL_set_3D_position(this->p3DSamples[v59].hSample, LODWORD(v58), 0.0, uNumRepeats);
              v26 = -*(float *)&uNumRepeats;
              v27 = -v58;
              AIL_set_3D_orientation(this->p3DSamples[v59].hSample, LODWORD(v27), 0.0, LODWORD(v26), 0.0, 1.0, 0.0);
            }
            else
            {
              AIL_end_3D_sample(this->p3DSamples[v59].hSample);
              pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
            }
            continue;
          }//5
          v12 = (SpriteObject *)&pLevelDecorations[PID_ID(this->p3DSamples[v59].field_4)];
        }
        else//2
          v12 = &pSpriteObjects[PID_ID(this->p3DSamples[v59].field_4)];
        a1.vWorldPosition.x = (double)v12->vPosition.x;
        a1.vWorldPosition.y = (double)v12->vPosition.y;
        v11 = (double)v12->vPosition.z;
//LABEL_21:
        a1.vWorldPosition.z = v11;
        if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
        {
          v16 = pIndoorCameraD3D->fRotationXCosine;
          v17 = pIndoorCameraD3D->fRotationXSine;
          v55 = pIndoorCameraD3D->fRotationYCosine;
          v56 = pIndoorCameraD3D->fRotationYSine;
          if (pIndoorCameraD3D->sRotationX)
          {
            v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
            *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
            v18 = a1.vWorldPosition.z - (double)pParty->vPosition.z;
            //if ( pRenderer->pRenderD3D )
            {
              v19 = *(float *)&uNumRepeats * v56 + v58 * v55;
              v20 = v58 * v56 - *(float *)&uNumRepeats * v55;
            }
            //else
           // {
            //  v19 = v58 * v55 - *(float *)&uNumRepeats * v56;
           //   v20 = v58 * v56 + *(float *)&uNumRepeats * v55;
            //}
            a1.vWorldViewPosition.x = v19 * v16 - v18 * v17;
            a1.vWorldViewPosition.y = v20;
            a1.vWorldViewPosition.z = v19 * v17 + v18 * v16;
          }
          else
          {
            v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
            *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
            //if ( pRenderer->pRenderD3D )
            {
              a1.vWorldViewPosition.x = *(float *)&uNumRepeats * v56 + v58 * v55;
              v21 = v58 * v56 - *(float *)&uNumRepeats * v55;
            }
            //else
            //{
              //a1.vWorldViewPosition.x = v58 * v55 - *(float *)&uNumRepeats * v56;
              //v21 = v58 * v56 + *(float *)&uNumRepeats * v55;
            //}
            a1.vWorldViewPosition.y = v21;
            a1.vWorldViewPosition.z = a1.vWorldPosition.z - (double)pParty->vPosition.z;
          }
        }
        else
          pIndoorCameraD3D->ViewTransform(&a1, 1);
        v58 = a1.vWorldViewPosition.y * -0.012207031;
        v22 = a1.vWorldViewPosition.x * 0.012207031;
        *(float *)&uNumRepeats = v22;
        v23 = abs((signed __int64)v22);
        v24 = abs(0);
        v25 = abs((signed __int64)v58);
        if ( int_get_vector_length(v25, v24, v23) <= 100 )
        {
          AIL_set_3D_position(this->p3DSamples[v59].hSample, LODWORD(v58), 0.0, uNumRepeats);
          v26 = -*(float *)&uNumRepeats;
          v27 = -v58;
          AIL_set_3D_orientation(this->p3DSamples[v59].hSample, LODWORD(v27), 0.0, LODWORD(v26), 0.0, 1.0, 0.0);
        }
        else
        {
          AIL_end_3D_sample(this->p3DSamples[v59].hSample);
          pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
        }
        continue;
      }
      if ( uCurrentlyLoadedLevelType != LEVEL_Indoor )//==1
      {
        pIndoorCameraD3D->ViewTransform(&a1, 1);
        v58 = a1.vWorldViewPosition.y * -0.012207031;
        v22 = a1.vWorldViewPosition.x * 0.012207031;
        *(float *)&uNumRepeats = v22;
        v23 = abs((signed __int64)v22);
        v24 = abs(0);
        v25 = abs((signed __int64)v58);
        if ( int_get_vector_length(v25, v24, v23) <= 100 )
        {
          AIL_set_3D_position(this->p3DSamples[v59].hSample, LODWORD(v58), 0.0, uNumRepeats);
          v26 = -*(float *)&uNumRepeats;
          v27 = -v58;
          AIL_set_3D_orientation(this->p3DSamples[v59].hSample, LODWORD(v27), 0.0, LODWORD(v26), 0.0, 1.0, 0.0);
        }
        else
        {
          AIL_end_3D_sample(this->p3DSamples[v59].hSample);
          pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
        }
        continue;
      }
      pDoor = &pIndoor->pDoors[PID_ID(this->p3DSamples[v59].field_4)];
      if ( pDoor->uDoorID )
      {
        uNumRepeats = *pDoor->pXOffsets;
        a1.vWorldPosition.x = (double)uNumRepeats;
        uNumRepeats = *pDoor->pYOffsets;
        a1.vWorldPosition.y = (double)uNumRepeats;
        uNumRepeats = *pDoor->pZOffsets;
        v11 = (double)uNumRepeats;
        a1.vWorldPosition.z = v11;
        if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
        {
          v16 = pIndoorCameraD3D->fRotationXCosine;
          v17 = pIndoorCameraD3D->fRotationXSine;
          v55 = pIndoorCameraD3D->fRotationYCosine;
          v56 = pIndoorCameraD3D->fRotationYSine;
          if (pIndoorCameraD3D->sRotationX)
          {
            v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
            *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
            v18 = a1.vWorldPosition.z - (double)pParty->vPosition.z;
            //if ( pRenderer->pRenderD3D )
            {
              v19 = *(float *)&uNumRepeats * v56 + v58 * v55;
              v20 = v58 * v56 - *(float *)&uNumRepeats * v55;
            }
            //else
            //{
             // v19 = v58 * v55 - *(float *)&uNumRepeats * v56;
             // v20 = v58 * v56 + *(float *)&uNumRepeats * v55;
            //}
            a1.vWorldViewPosition.x = v19 * v16 - v18 * v17;
            a1.vWorldViewPosition.y = v20;
            a1.vWorldViewPosition.z = v19 * v17 + v18 * v16;
          }
          else
          {
            v58 = a1.vWorldPosition.x - (double)pParty->vPosition.x;
            *(float *)&uNumRepeats = a1.vWorldPosition.y - (double)pParty->vPosition.y;
            //if ( pRenderer->pRenderD3D )
            {
              a1.vWorldViewPosition.x = *(float *)&uNumRepeats * v56 + v58 * v55;
              v21 = v58 * v56 - *(float *)&uNumRepeats * v55;
            }
            //else
            //{
            //  a1.vWorldViewPosition.x = v58 * v55 - *(float *)&uNumRepeats * v56;
            //  v21 = v58 * v56 + *(float *)&uNumRepeats * v55;
            //}
            a1.vWorldViewPosition.y = v21;
            a1.vWorldViewPosition.z = a1.vWorldPosition.z - (double)pParty->vPosition.z;
          }
        }
        else
          pIndoorCameraD3D->ViewTransform(&a1, 1);
        v58 = a1.vWorldViewPosition.y * -0.012207031;
        v22 = a1.vWorldViewPosition.x * 0.012207031;
        *(float *)&uNumRepeats = v22;
        v23 = abs((signed __int64)v22);
        v24 = abs(0);
        v25 = abs((signed __int64)v58);
        if ( int_get_vector_length(v25, v24, v23) <= 100 )
        {
          AIL_set_3D_position(this->p3DSamples[v59].hSample, LODWORD(v58), 0.0, uNumRepeats);
          v26 = -*(float *)&uNumRepeats;
          v27 = -v58;
          AIL_set_3D_orientation(this->p3DSamples[v59].hSample, LODWORD(v27), 0.0, LODWORD(v26), 0.0, 1.0, 0.0);
        }
        else
        {
          AIL_end_3D_sample(this->p3DSamples[v59].hSample);
          pAudioPlayer->_4ABF23(&this->p3DSamples[v59]);
        }
      }
    }
    //}
  }

//LABEL_37:
  for (int i = 0; i < uMixerChannels; ++i)
  {
    if (AIL_sample_status(pMixerChannels[i].hSample) == AIL::Sample::Done)
    {
      AIL_end_sample(pMixerChannels[i].hSample);
      FreeChannel(&pMixerChannels[i]);
    }
  }

  for (int i = 0; i < uMixerChannels; ++i)
  {
    if (pMixerChannels[i].source_pid <= 0)
      continue;

    int source_type = PID_TYPE(pMixerChannels[i].source_pid),
        source_id = PID_ID(pMixerChannels[i].source_pid);
//    int source_x,
//       int source_y,
//        source_z;

    switch (source_type)
    {
      case 0:
      case OBJECT_Player:
      case OBJECT_BModel:
        continue;

      case OBJECT_BLVDoor:
      {
        assert(uCurrentlyLoadedLevelType == LEVEL_Indoor);

        assert(source_id < pIndoor->uNumDoors);
        if (!pIndoor->pDoors[source_id].uDoorID)
          continue;

        //source_x = pIndoor->pDoors[source_id].pXOffsets[0];
        //source_y = pIndoor->pDoors[source_id].pYOffsets[0];
        //source_z = pIndoor->pDoors[source_id].pZOffsets[0];
        int sound_strength = GetSoundStrengthByDistanceFromParty(pIndoor->pDoors[source_id].pXOffsets[0],
                                                                 pIndoor->pDoors[source_id].pYOffsets[0],
                                                                 pIndoor->pDoors[source_id].pZOffsets[0]);
        if ( sound_strength )
        {
          AIL_set_sample_volume(pMixerChannels[i].hSample, sound_strength);
          AIL_set_sample_pan(pMixerChannels[i].hSample, sub_4AB66C(pIndoor->pDoors[source_id].pXOffsets[0],
                                                                 pIndoor->pDoors[source_id].pYOffsets[0]));
        }
        else
        {
          AIL_end_sample(pMixerChannels[i].hSample);
          FreeChannel(&pMixerChannels[i]);
        }
      }
      continue;

      case OBJECT_Item:
      {
        //assert(source_id < uNumSpriteObjects); // Ritor1:в ида до и после перехода одинаково

        //source_x = pSpriteObjects[source_id].vPosition.x;
        //source_y = pSpriteObjects[source_id].vPosition.y;
        //source_z = pSpriteObjects[source_id].vPosition.z;
        int sound_strength = GetSoundStrengthByDistanceFromParty(pSpriteObjects[source_id].vPosition.x,
                                                                 pSpriteObjects[source_id].vPosition.y,
                                                                 pSpriteObjects[source_id].vPosition.z);
        if ( sound_strength )
        {
          AIL_set_sample_volume(pMixerChannels[i].hSample, sound_strength);
          AIL_set_sample_pan(pMixerChannels[i].hSample, sub_4AB66C(pSpriteObjects[source_id].vPosition.x,
                                                                   pSpriteObjects[source_id].vPosition.y));
        }
        else
        {
          AIL_end_sample(pMixerChannels[i].hSample);
          FreeChannel(&pMixerChannels[i]);
        }
      }
      continue;

      case OBJECT_Decoration:
      {
        assert(source_id < uNumLevelDecorations);

        //source_x = pLevelDecorations[source_id].vPosition.x;
        //source_y = pLevelDecorations[source_id].vPosition.y;
        //source_z = pLevelDecorations[source_id].vPosition.z;
        int sound_strength = GetSoundStrengthByDistanceFromParty(pLevelDecorations[source_id].vPosition.x,
                                                                 pLevelDecorations[source_id].vPosition.y,
                                                                 pLevelDecorations[source_id].vPosition.z);
        if ( sound_strength )
        {
          //AIL_set_sample_volume(pMixerChannels[i].hSample, sound_strength);
          AIL_set_sample_pan(pMixerChannels[i].hSample, sub_4AB66C(pLevelDecorations[source_id].vPosition.x,
                                                                   pLevelDecorations[source_id].vPosition.y));
        }
        else
        {
          AIL_end_sample(pMixerChannels[i].hSample);
          FreeChannel(&pMixerChannels[i]);
        }
      }
      continue;

      case OBJECT_Actor:
      {
        assert(source_id < uNumActors);

        //source_x = pActors[source_id].vPosition.x;
        //source_y = pActors[source_id].vPosition.y;
        //source_z = pActors[source_id].vPosition.z;
        int sound_strength = GetSoundStrengthByDistanceFromParty(pActors[source_id].vPosition.x,
                                                                 pActors[source_id].vPosition.y,
                                                                 pActors[source_id].vPosition.z);
        if ( sound_strength )
        {
          AIL_set_sample_volume(pMixerChannels[i].hSample, sound_strength);
          AIL_set_sample_pan(pMixerChannels[i].hSample, sub_4AB66C(pActors[source_id].vPosition.x,
                                                                   pActors[source_id].vPosition.y));
        }
        else
        {
          AIL_end_sample(pMixerChannels[i].hSample);
          FreeChannel(&pMixerChannels[i]);
        }
      }
      continue;

      default:
        assert(false);
        continue;
    }

    /*if (int sound_strength = GetSoundStrengthByDistanceFromParty(source_x, source_y, source_z))
    {
      AIL_set_sample_volume(pMixerChannels[i].hSample, sound_strength);
      AIL_set_sample_pan(pMixerChannels[i].hSample, sub_4AB66C(source_x, source_y));
    }
    else
    {
      AIL_end_sample(pMixerChannels[i].hSample);
      FreeChannel(&pMixerChannels[i]);
    } */
  }

  if (current_screen_type != SCREEN_GAME) //отключение звука декораций при переключении окна игры
  {
    if (AIL_sample_status(pMixerChannels[4].hSample) == AIL::Sample::Playing)
      AIL_end_sample(pMixerChannels[4].hSample);
    return;
  }
  if (!_6807E0_num_decorations_with_sounds_6807B8)
    return;

  v55 = 0;
      //v59 = 0;
  for (uint i = 0; i < _6807E0_num_decorations_with_sounds_6807B8; ++i)
  {
    LODWORD(v56) = 1;
        //v43 = _6807B8_level_decorations_ids[v59];
        //v44 = &pLevelDecorations[_6807B8_level_decorations_ids[v59]];
        //v45 = abs(v44->vPosition.z - pParty->vPosition.z);
        //v46 = abs(v44->vPosition.y - pParty->vPosition.y);
        //v47 = abs(v44->vPosition.x - pParty->vPosition.x);
    LevelDecoration* decor = &pLevelDecorations[_6807B8_level_decorations_ids[i]];
    if (int_get_vector_length(abs(decor->vPosition.x - pParty->vPosition.x),
                              abs(decor->vPosition.y - pParty->vPosition.y),
                              abs(decor->vPosition.z - pParty->vPosition.z)) > 8192)
      continue;

    DecorationDesc* decor_desc = &pDecorationList->pDecorations[decor->uDecorationDescID];
      //v48 = &pDecorationList->pDecorations[decor->uDecorationDescID];
      //v49 = v48->uFlags;
      uNumRepeats = (~(unsigned __int8)decor_desc->uFlags & DECORATION_DESC_SLOW_LOOP) >> 6;
 
    if (decor_desc->SoundOnDawn() || decor_desc->SoundOnDusk())
    {
        //v50 = decor->field_1A;
        v55 = 0.0;
        uNumRepeats = 2;
        if (decor->field_1A)
        {
          //v51 = decor->field_1A - 32;
          decor->field_1A = decor->field_1A - 32;
          if ( decor->field_1A < 0 )
            decor->field_1A = 0;
        }
    }

      //v52 = v48->uFlags;
    if (!decor_desc->SoundOnDawn())
    {
      if (!decor_desc->SoundOnDusk())
      {
        if ( v55 == 0.0 )
        {
          if ( v56 != 0.0 )
          {
            v53 = 8 * _6807B8_level_decorations_ids[i];
            LOBYTE(v53) = v53 | OBJECT_Decoration;
            PlaySound((SoundID)decor_desc->uSoundID, v53, uNumRepeats, -1, 0, 0, 0, 0);//sound of Boat and water(звуки корабля, плескания воды)
          }
          continue;
        }
        if ( !decor->field_1A )
          decor->field_1A = (rand() % 15 + 1) << 7;
        if ( v56 != 0.0 )
        {
          v53 = 8 * _6807B8_level_decorations_ids[i];
          LOBYTE(v53) = v53 | OBJECT_Decoration;
          PlaySound((SoundID)decor_desc->uSoundID, v53, uNumRepeats, -1, 0, 0, 0, 0);
        }
        continue;
      }
      if ( v55 != 0.0 )
      {
        if ( !decor->field_1A )
          decor->field_1A = (rand() % 15 + 1) << 7;
        if ( v56 != 0.0 )
        {
          v53 = 8 * _6807B8_level_decorations_ids[i];
          LOBYTE(v53) = v53 | OBJECT_Decoration;
          PlaySound((SoundID)decor_desc->uSoundID, v53, uNumRepeats, -1, 0, 0, 0, 0);
        }
        continue;
      }
    }
    v56 = 0.0;

    if (pParty->uCurrentHour >= 5 && pParty->uCurrentHour < 6 ||
        pParty->uCurrentHour >= 20 && pParty->uCurrentHour < 21)
    {
        if ( !decor->field_1A && rand() % 100 < 100 )
          LODWORD(v56) = 1;
        LODWORD(v55) = 1;
    }
    if ( v55 == 0.0 )
    {
      if ( v56 != 0.0 )
      {
        v53 = 8 * _6807B8_level_decorations_ids[i];
        LOBYTE(v53) = v53 | OBJECT_Decoration;
        PlaySound((SoundID)decor_desc->uSoundID, v53, uNumRepeats, -1, 0, 0, 0, 0);
      }
      continue;
    }
    if ( !decor->field_1A )
      decor->field_1A = (rand() % 15 + 1) << 7;
    if ( v56 != 0.0 )
    {
      v53 = 8 * _6807B8_level_decorations_ids[i];
      LOBYTE(v53) = v53 | OBJECT_Decoration;
      PlaySound((SoundID)decor_desc->uSoundID, v53, uNumRepeats, -1, 0, 0, 0, 0);
    }
    continue;
  }
  }
}

//----- (004AB66C) --------------------------------------------------------
int sub_4AB66C(int a1, int a2)
{
  signed int v2; // eax@1

  v2 = stru_5C6E00->uDoublePiMask & (stru_5C6E00->Atan2(a1 - pParty->vPosition.x, a2 - pParty->vPosition.y)
                                  - stru_5C6E00->uIntegerHalfPi - pParty->sRotationY);
  if ( v2 > (signed int)stru_5C6E00->uIntegerPi )
    v2 = 2 * stru_5C6E00->uIntegerPi - v2;
  v2 =(v2 >> 3) - (v2 >> 10) ;
  return v2;
}
// 4AB66C: using guessed type int sub_4AB66C(int, int);

//----- (004AB6B1) --------------------------------------------------------
int GetSoundStrengthByDistanceFromParty(int x, int y, int z)
{
  int dir_x; // ST08_4@1
  int dir_y; // esi@1
  int dir_z; // eax@1
  int length; // [sp+10h] [bp+8h]@1

  dir_z = abs(z - pParty->vPosition.z);
  dir_y = abs(y - pParty->vPosition.y);
  dir_x = abs(x - pParty->vPosition.x);
  length = int_get_vector_length(dir_x, dir_y, dir_z);
  if ( length <= 0x2000 )
    return 114 - (unsigned __int64)(signed __int64)((double)length * 0.0001220703125 * 100.0);
  else
    return 0;
}

//----- (004AB71F) --------------------------------------------------------
void AudioPlayer::StopChannels(int uStartChannel, int uEndChannel)
{
  if ( bPlayerReady )
  {
    if ( b3DSoundInitialized )
    {
      for ( uint i = 0; i < uNum3DSamples; ++i )
      {
        if ( (uStartChannel == -1 || i < uStartChannel || i > uEndChannel)
            && p3DSamples[i].field_8 && pSoundList->pSL_Sounds[p3DSamples[i].field_8].eType != SOUND_DESC_SYSTEM)
        {
          AIL_end_3D_sample(p3DSamples[i].hSample);
          _4ABF23(&p3DSamples[i]);
          p3DSamples[i].field_4 = 0;
        }
      }
    }
    if ( hDigDriver )
    {
      for ( int i = 0; i < uMixerChannels; ++i )
      {
        if ( (uStartChannel == -1 || i < uStartChannel || i > uEndChannel)
          && pSoundList->pSL_Sounds[pMixerChannels[i].uSourceTrackIdx].eType != SOUND_DESC_SYSTEM)//все, кроме системных звуков, отключаются
        {
          AIL_end_sample(pMixerChannels[i].hSample);
          FreeChannel(&pMixerChannels[i]);
          pMixerChannels[i].source_pid = 0;
        }
      }
    }
    if (hSequence)
      AIL_end_sequence(hSequence);
    if (hStream)
      AIL_pause_stream(hStream, 1);
  }
}

//----- (004AB818) --------------------------------------------------------
void AudioPlayer::LoadAudioSnd()
{
  DWORD NumberOfBytesRead; // [sp+Ch] [bp-4h]@3

  hAudioSnd = CreateFileA("Sounds\\Audio.snd", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, 0);
  if (hAudioSnd == INVALID_HANDLE_VALUE)
  {
    Log::Warning(L"Can't open file: %s", L"Sounds\\Audio.snd");
    return;
  }

  ReadFile(hAudioSnd, &uNumSoundHeaders, 4, &NumberOfBytesRead, 0);
  pSoundHeaders = nullptr;
  pSoundHeaders = (SoundHeader *)malloc(52 * uNumSoundHeaders + 2);
  ReadFile(hAudioSnd, pSoundHeaders, 52 * uNumSoundHeaders, &NumberOfBytesRead, 0);
}

//----- (004AB8CE) --------------------------------------------------------
void AudioPlayer::Initialize()
{
  int v3; // ebx@1
  _PROVIDER *v6; // eax@9
  int v12; // [sp+Ch] [bp-Ch]@9
  char *Str1; // [sp+10h] [bp-8h]@6
  int v14; // [sp+14h] [bp-4h]@5

  //WriteWindowsRegistryString( "3DSoundProvider", "Aureal A3D Interactive(TM)");//запись в реестр для 3D звука(Microsoft DirectSound3D with Creative Labs EAX(TM))

  v3 = 0;
  //this->hWindow = hWnd;
  this->hAILRedbook = 0;
  this->hDigDriver = 0;
  this->dword_0002AC = 0;
  this->hSequence = 0;
  this->uMasterVolume = 127;
  this->dword_0002C8 = 64;
  this->dword_0002CC = 2;

  MSS32_DLL_Initialize();
  BINKW32_DLL_Initialize();
  SMACKW32_DLL_Initialize();
  
  AIL_startup();
  if (bCanLoadFromCD)
    hAILRedbook = AIL_redbook_open_drive(cMM7GameCDDriveLetter/*cGameCDDriveLetter*/);
  //else
  //  hAILRedbook = AIL_redbook_open(0);
  //v4 = Audio_GetFirstHardwareDigitalDriver();

  hDigDriver = Audio_GetFirstHardwareDigitalDriver();
  if ( hDigDriver )
    SmackSoundUseMSS(hDigDriver);
  if ( ReadWindowsRegistryInt("Disable3DSound", 0) != 1 && true)//pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT )
  {
    v14 = 0;
    bEAXSupported = 0;
    b3DSoundInitialized = 0;
    ReadWindowsRegistryString("3DSoundProvider", p3DSoundProvider, 128, "NONE");
    CheckA3DSupport(true);
    HPROVIDER prov;
    while ( AIL_enumerate_3D_providers(&v14, &prov, &Str1) )
    {
      if ( !strcmp(Str1, p3DSoundProvider) )
      {
        if ( AIL_open_3D_provider(prov) )
        {
          bEAXSupported = 0;
          b3DSoundInitialized = 0;
          h3DSoundProvider = 0;
        }
        else
        {
          v6 = prov;
          //v7 = prov;
          b3DSoundInitialized = 1;
          h3DSoundProvider = v6;
          uNum3DSamples = 4;
          AIL_3D_provider_attribute(prov, "EAX environment selection", &v12);
          if ( v12 != -1 )
            bEAXSupported = 1;
        }
        pAudioPlayer->_4AC0A2();
        break;
      }
    }
  }
  for ( v3; v3 < uMixerChannels; ++v3 )
  {
    pMixerChannels[v3].hSample = AIL_allocate_sample_handle(hDigDriver);
    if ( !pMixerChannels[v3].hSample )
      break;
  }
  uMixerChannels = v3;
  if ( bPlayerReady )
    StopChannels(-1, -1);
  //v10 = hAILRedbook;
  bPlayerReady = true;
  if ( hAILRedbook )
  {
    AIL_redbook_stop(hAILRedbook);
    uNumRedbookTracks = AIL_redbook_tracks(hAILRedbook);
  }
  pAudioPlayer->sRedbookVolume = AIL_redbook_volume(hAILRedbook);
  pAudioPlayer->SetMasterVolume(pSoundVolumeLevels[uSoundVolumeMultiplier] * 128.0f);
  if ( bPlayerReady )
  {
    if ( use_music_folder )
      alSourcef (mSourceID, AL_GAIN, pSoundVolumeLevels[uMusicVolimeMultiplier]);
	else if ( hAILRedbook )
	  AIL_redbook_set_volume(hAILRedbook, (unsigned __int64)(pSoundVolumeLevels[uMusicVolimeMultiplier] * 64.0f) >> 32);
  }
  LoadAudioSnd();
}

//----- (004ABAF7) --------------------------------------------------------
_DIG_DRIVER *Audio_GetFirstHardwareDigitalDriver(void)
{
  int v0; // ecx@1
  size_t v2; // eax@4
  signed int v3; // kr14_4@9
  int v5; // [sp+10h] [bp-Ch]@2
  unsigned int pNum_devices; // [sp+14h] [bp-8h]@1
  _DIG_DRIVER *hDrv; // [sp+18h] [bp-4h]@3
  
  static int sample_Rate = 22050;
  static int bitsPerSample = 16;
  static int channels = 2;

  AIL_set_preference(15, 0);
  AIL_set_preference(33, 1);
  v0 = sample_Rate;
  pAudioPlayer->pDeviceNames[0][0] = 0;
  pAudioPlayer->uNumDevices = 0;
  pNum_devices = 0;

  if ( sample_Rate < 11025 )
    return 0;
  v5 = 0;
  while ( 1 )
  {
    while ( 1 )
    {
      pcmWaveFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
      pcmWaveFormat.wf.nChannels = channels;                                   // Channels: 1 = mono, 2 = stereo
      pcmWaveFormat.wf.nSamplesPerSec = v0;                                    //частота оцифровки
      pcmWaveFormat.wf.nBlockAlign = channels * bitsPerSample / 8;             //количество данных в блоке
      pcmWaveFormat.wBitsPerSample = bitsPerSample;
      pcmWaveFormat.wf.nAvgBytesPerSec = pcmWaveFormat.wf.nSamplesPerSec * pcmWaveFormat.wf.nBlockAlign;
      if ( !AIL_waveOutOpen(&hDrv, 0, -1, &pcmWaveFormat.wf) )
      {
        strcpy(pAudioPlayer->pDeviceNames[v5], "Device: ");
        v2 = strlen(pAudioPlayer->pDeviceNames[v5]);
        AIL_digital_configuration(hDrv, (int *)pAudioPlayer->pFrequency + v5, (int *)pAudioPlayer->array_000C30 + v5, (char *)pAudioPlayer->pDeviceNames + v2 + v5 * 32);
        ++pNum_devices;
        v5++;
        pAudioPlayer->uNumDevices = pNum_devices;
        if ( AIL_get_preference(15) )
          return hDrv;
        if ( !strstr(pAudioPlayer->pDeviceNames[v5 - 1], "Emulated") )
          return hDrv;
        AIL_waveOutClose(hDrv);
        AIL_set_preference(15, 1);
        break;
	  }
      if ( !AIL_get_preference(15) )
      {
        AIL_set_preference(15, 1);
        if ( sample_Rate < 11025 )
          return 0;
	    break;
      }
      //v3 = sample_Rate;
      v0 = sample_Rate / 2;
      sample_Rate /= 2;
      if ( sample_Rate / 2 < 11025 )
      {
        if ( bitsPerSample == 8 )
       {
          v0 = 22050;
          bitsPerSample = 8;
          sample_Rate = 22050;
        }
        if ( v0 < 11025 )
          return 0;
		break;
      }
    }
  }
}

//----- (004ABC9B) --------------------------------------------------------
void AudioPlayer::CheckA3DSupport(bool query)
{
  DWORD cbData; // [sp+8h] [bp-Ch]@1
  HKEY hKey; // [sp+10h] [bp-4h]@1
  hKey = 0;
  cbData = 4;
  if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Aureal\\A3D", 0, KEY_READ|KEY_WOW64_32KEY, &hKey))
  {
    int Aureal3D_SplashAudio = 0;
    if (query)
      RegQueryValueExA(hKey, "SplashAudio", 0, 0, (LPBYTE)&Aureal3D_SplashAudio, &cbData);
    RegSetValueExA(hKey, "SplashAudio", 0, 4, (const BYTE *)&Aureal3D_SplashAudio, 4);

    int Aureal3D_SplashScreen = 0;
    if (query)
      RegQueryValueExA(hKey, "SplashScreen", 0, 0, (LPBYTE)&Aureal3D_SplashScreen, &cbData);
    RegSetValueExA(hKey, "SplashScreen", 0, 4, (const BYTE *)&Aureal3D_SplashScreen, 4);
    RegCloseKey(hKey);
  }
}


//----- (004ABD5B) --------------------------------------------------------
void AudioPlayer::Release() //Освободить
{
  MixerChannel *pMixerChannel; // ebx@3
//  char v4; // dl@5
  AudioPlayer_3DSample *p3DSample; // edi@7
  void *v9; // ecx@15

  if ( this->bPlayerReady )
  {
	free(pSoundHeaders);
    CloseHandle(pMediaPlayer->hMagicVid);
    CloseHandle(pMediaPlayer->hMightVid);
    pAudioPlayer->StopChannels(-1, -1);
    if ( pAudioPlayer->uMixerChannels > 0 )
    {
      pMixerChannel = pAudioPlayer->pMixerChannels;
      for ( uint i = 0; i < pAudioPlayer->uMixerChannels; ++i )
      {
        AIL_release_sample_handle(pMixerChannel->hSample);
        ++pMixerChannel;
      }
    }
    if ( ReadWindowsRegistryInt("Disable3DSound", 0) != 1 )
    {
      CheckA3DSupport(false);
      if ( pAudioPlayer->uNum3DSamples > 0 )
      {
        p3DSample = pAudioPlayer->p3DSamples;
        for ( uint i = 0; i < pAudioPlayer->uNum3DSamples; ++i )
        {
          if ( p3DSample->hSample )
          {
            AIL_release_3D_sample_handle(p3DSample->hSample);
            p3DSample->hSample = 0;
          }
          ++p3DSample;
        }
      }
      if ( pAudioPlayer->h3DSoundProvider )
      {
        AIL_close_3D_provider(pAudioPlayer->h3DSoundProvider);
        pAudioPlayer->h3DSoundProvider = 0;
      }
    }
    if ( pAudioPlayer->hAILRedbook )
    {
      AIL_redbook_stop(pAudioPlayer->hAILRedbook);
      AIL_redbook_set_volume((HREDBOOK)&pAudioPlayer->hAILRedbook, pAudioPlayer->sRedbookVolume);
      AIL_redbook_close(pAudioPlayer->hAILRedbook);
    }
    AIL_shutdown();
    pSoundList->Release();
    v9 = *(void **)&pAudioPlayer->field_C78[0];
    if ( v9 )
      ReleaseSoundData(v9);
    CloseHandle(pAudioPlayer->hAudioSnd);
  }
}

//----- (004ABE55) --------------------------------------------------------
void AudioPlayer::FreeChannel(MixerChannel *pChannel)
{
  int num_same_sound_on_channels; // eax@8
  int v10; // ecx@12
  int v12; // eax@13
  int v14[16]; // [sp+Ch] [bp-48h]@8
  int num_playing_channels; // [sp+4Ch] [bp-8h]@5

  if (!pSoundList->pSL_Sounds)
    return;

  //v4 = &pSoundList->pSounds[pChannel->uSourceTrackIdx];
  if ( pSoundList->pSL_Sounds[pChannel->uSourceTrackIdx].eType == SOUND_DESC_SWAP)
  {
    if ( pSoundList->pSL_Sounds[pChannel->uSourceTrackIdx].pSoundData[0] && 
       !(pSoundList->pSL_Sounds[pChannel->uSourceTrackIdx].uFlags & SOUND_DESC_SYSTEM) )
    {
      num_playing_channels = 0;
      num_same_sound_on_channels = 0;
      if ( this->uMixerChannels <= 0 )
        goto LABEL_16;
      for ( uint i = 0; i < uMixerChannels; i++ )
      {
        if ( pChannel->uSourceTrackID == pMixerChannels[i].uSourceTrackID )
        {
          v14[num_same_sound_on_channels++] = i;
          if ( AIL_sample_status((HSAMPLE)pMixerChannels[i].hSample) == AIL::Sample::Playing)
            ++num_playing_channels;
        }
      }
      if ( !num_playing_channels )
      {
LABEL_16:
        pSoundList->UnloadSound(pChannel->uSourceTrackIdx, 1);
        for ( v10 = 0; v10 < num_same_sound_on_channels; v10++ )
        {
          v12 = 16 * (v14[v10] + 47);
          pMixerChannels[v14[v10]].uSourceTrackID = 0;
          *(unsigned int *)((char *)&bEAXSupported + v12) = 0;
        }
      }
    }
  }
}

//----- (004ABF23) --------------------------------------------------------
void AudioPlayer::_4ABF23(AudioPlayer_3DSample *a2)
{
  int v2; // ebx@1
  unsigned __int8 v5; // zf@5
  unsigned __int8 v6; // sf@5
  char *v7; // edi@6
  int v8; // eax@8
  int v10; // ecx@12
  int v11; // eax@13
  int v13[16]; // [sp+Ch] [bp-48h]@8
  int v14; // [sp+4Ch] [bp-8h]@5
  int v15; // [sp+50h] [bp-4h]@5

  if ( pSoundList->pSL_Sounds )
  {
    //v4 = &pSoundList->pSounds[a2->field_8];
    if ( pSoundList->pSL_Sounds[a2->field_8].eType == SOUND_DESC_SWAP)
    {
      if ( pSoundList->pSL_Sounds[a2->field_8].p3DSound && !(pSoundList->pSL_Sounds[a2->field_8].uFlags & SOUND_DESC_SYSTEM) )
      {
        v5 = this->uNum3DSamples == 0;
        v6 = this->uNum3DSamples < 0;
        v14 = 0;
        v15 = 0;
        if ( v6 | v5 )
          goto LABEL_16;
        v7 = (char *)this->p3DSamples;
        __debugbreak();//Ritor1
        for ( v2 = 0; v2 < uNum3DSamples; ++v2 )
        {
          if ( a2->field_C == *((int *)v7 + 3) )
          {
            v8 = v15;
            //v9 = this->p3DSamples[v2];
            ++v15;
            v13[v8] = v2;
            if ( AIL_3D_sample_status(&this->p3DSamples[v2]) == 4 )
              ++v14;
          }
          v7 += 16;
        }
        if ( !v14 )
        {
LABEL_16:
          pSoundList->UnloadSound(a2->field_8, 1);
          for ( v10 = 0; v10 < v15; v10++ )
          {
            v11 = v13[v10];
            *(&bEAXSupported + 4 * (v11 + 2)) = 0;
            p3DSamples[v11].field_8 = 0;
          }
        }
      }
    }
  }
}

//----- (004ABFDB) --------------------------------------------------------
void PlayLevelMusic()
{
  unsigned int map_id; // eax@1

  map_id = pMapStats->GetMapInfo(pCurrentMapName);
  if ( map_id )
    pAudioPlayer->PlayMusicTrack((MusicID)pMapStats->pInfos[map_id].uRedbookTrackID);
}

//----- (004AC004) --------------------------------------------------------
void AudioPlayer::SetEAXPreferences()
{
  float v4; // [sp+4h] [bp-4h]@2

  if ( this->bEAXSupported )
  {
    v4 = 0.0;
    AIL_set_3D_provider_preference(this->h3DSoundProvider, "EAX effect volume", (int *)&v4);
    v4 = 1.0;
    AIL_set_3D_provider_preference(this->h3DSoundProvider, "EAX damping", (int *)&v4);
  }
}
// 4D82DC: using guessed type int __stdcall AIL_set_3D_provider_preference(int, int, int);

//----- (004AC041) --------------------------------------------------------
void AudioPlayer::SetMapEAX()
{
  unsigned int pMapID; // eax@1
  int v3; // [sp+4h] [bp-4h]@3

  pMapID = pMapStats->GetMapInfo(pCurrentMapName);
  if ( this->b3DSoundInitialized && this->bEAXSupported )
  {
    v3 = pMapStats->pInfos[pMapID].uEAXEnv;
    if ( (unsigned int)v3 >= 0x1A )
    {
      SetEAXPreferences();
      this->field_214 = -1;
    }
    else
    {
      AIL_set_3D_provider_preference(this->h3DSoundProvider, "EAX environment selection", &v3);
      this->field_214 = v3;
    }
  }
}
// 4D82DC: using guessed type int __stdcall AIL_set_3D_provider_preference(int, int, int);

//----- (004AC0A2) --------------------------------------------------------
int AudioPlayer::_4AC0A2()
{
  unsigned int map_id; // eax@1
  void *v9; // eax@8
  int v12; // [sp+1Ch] [bp-8h]@1

  if ( this->b3DSoundInitialized )
  {
    //v5 = &this->uNum3DSamples;
    AIL_3D_provider_attribute(this->h3DSoundProvider, "Maximum supported samples", &this->uNum3DSamples);
    if ( this->uNum3DSamples > 32 )
      this->uNum3DSamples = 32;
    //v6 = this->uNum3DSamples;
    if ( !this->uNum3DSamples )
    {
      this->b3DSoundInitialized = 0;
      return -1;
    }
    //v13 = 0;
    //if ( this->uNum3DSamples > 0 )
    //{
      //v8 = this->p3DSamples;
      //while ( 1 )
      for ( int i = 0; i < this->uNum3DSamples; ++i )
      {
        v9 = (void *)AIL_allocate_3D_sample_handle(this->h3DSoundProvider);
        this->p3DSamples[i].hSample = v9;
        if ( !v9 )
          this->uNum3DSamples = i;
        AIL_set_3D_sample_float_distances(v9, 4096.0, 256.0, 4096.0, 256.0);
        AIL_set_3D_sample_volume(this->p3DSamples[i].hSample, this->s3DSoundVolume);
        //++v8;
      }
    //}
    if ( this->bEAXSupported )
    {
      //v10 = v4;
      //v11 = pMapStats->pInfos[map_id].uEAXEnv;
      map_id = pMapStats->GetMapInfo(pCurrentMapName);
      v12 = pMapStats->pInfos[map_id].uEAXEnv;
      if ( pMapStats->pInfos[map_id].uEAXEnv >= 0x1A )
      {
        pAudioPlayer->SetEAXPreferences();
        this->field_214 = -1;
      }
      else
      {
        AIL_set_3D_provider_preference(this->h3DSoundProvider, "EAX environment selection", &v12);
        this->field_214 = v12;
      }
    }
  }
  return 1;
}

//----- (004A96BE) --------------------------------------------------------
void ReleaseSoundData(void *_this)
{
  //for ( uint i = 0; (void *)&pSounds[i].pSoundData < (void *)&pSounds[2999].pSoundData; i++ )
  for ( uint i = 0; i < 2999; i++ )
  {
    if ( pSounds[i].pSoundData == _this )
    {
      free(_this);
      memset(&pSounds[i], 0, sizeof(pSounds[i]));
    }
  }

}

//----- (004A96FF) --------------------------------------------------------
struct SoundHeader *FindSound_BinSearch(unsigned int uStart, unsigned int uEnd, const char *pName)
{
/*  SoundHeader *result; // eax@2
  SoundHeader *pSound;
  signed int v6; // ebx@11

  while ( 1 )
  {
    v6 = uEnd - uStart;
    pSound = &pAudioPlayer->pSoundHeaders[v6 / 2 + uStart];
    if ( !pSound )
      return false;
    result = (SoundHeader *)_stricmp(pName, pSound->pSoundName);
    if ( !_stricmp(pName, pSound->pSoundName) )
      uFindSound_BinSearch_ResultID = v6 / 2 + uStart;
    if ( uStart == uEnd )
    {
      uFindSound_BinSearch_ResultID = -1;
      return result;
    }
    if ( (signed int)result < 0 )
      break;

    if ( v6 <= 4 )
    {
      if ( (signed int)uStart < (signed int)uEnd )
      {
        for ( uint i = uStart; i < (signed int)uEnd; ++i )
        {
          if ( !_stricmp(pName, pAudioPlayer->pSoundHeaders[i].pSoundName) )
          {
            uFindSound_BinSearch_ResultID = i;
            return &pAudioPlayer->pSoundHeaders[i];
          }
        }
      }
      uFindSound_BinSearch_ResultID = -1;
      return false;
    }

    uStart += v6 / 2;
LABEL_10:
	;
  }
  if ( v6 > 4 )
  {
    uEnd = v6 / 2 + uStart;
    goto LABEL_10;
  }
  if ( (signed int)uStart >= (signed int)uEnd )
  {
    uFindSound_BinSearch_ResultID = -1;
    return false;
  }*/
  for ( uint i = uStart; i < (signed int)uEnd; ++i )
  {
    if ( !_stricmp(pName, pAudioPlayer->pSoundHeaders[i].pSoundName) )
    {
      uFindSound_BinSearch_ResultID = i;
      return &pAudioPlayer->pSoundHeaders[i];
    }
  }
  uFindSound_BinSearch_ResultID = -1;
  return false;
}
// F1B4C8: using guessed type int uFindSound_BinSearch_ResultID;

//----- (004A97C6) --------------------------------------------------------
SoundData *LoadSound(const char *pSoundName, SoundData *pOutBuff, unsigned int uID)
{
  DWORD NumberOfBytesRead; // [sp+14h] [bp-8h]@8

  for (uint i = 0; i < 3000; ++i)
  {
    if (pSounds[i].uID == uID)
      return pSounds[i].pSoundData;
  }
  FindSound_BinSearch(0, pAudioPlayer->uNumSoundHeaders, pSoundName);
  if ( uFindSound_BinSearch_ResultID == -1 )
    return 0;
  if ( pOutBuff == (SoundData *)-1 )
    pOutBuff = (SoundData *)malloc(pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize + 4);
  SetFilePointer(pAudioPlayer->hAudioSnd, pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uFileOffset, 0, 0);
  if ( (signed int)pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uCompressedSize >= (signed int)pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize )
  {
    pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uCompressedSize = pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize;
    if ( pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize )
      ReadFile(pAudioPlayer->hAudioSnd, pOutBuff->pData, pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize, &NumberOfBytesRead, 0);// Ritor1: pSounds[20]
    else
      MessageBoxW(nullptr, L"Can't load sound file!", L"E:\\WORK\\MSDEV\\MM7\\MM7\\Code\\Sound.cpp:448", 0);
  }
  else
  {
    uID = (unsigned int)malloc(pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uCompressedSize);
    ReadFile(pAudioPlayer->hAudioSnd, (LPVOID)uID, pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uCompressedSize, &NumberOfBytesRead, 0);
    zlib::MemUnzip(pOutBuff->pData, &pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize, (const void *)uID, pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uCompressedSize);
    free((void *)uID);
  }
  if ( pOutBuff )
  {
    pOutBuff->uDataSize = pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize;

    if ( uID == SOUND_StartMainChoice02 )//Ritor1: code included (BUG)
    {
      char *p = (char *)pOutBuff->pData;
      p = p+7;
      memcpy(&pOutBuff->pData, p, NumberOfBytesRead - 7);
	  pOutBuff->uDataSize = NumberOfBytesRead - 7;
    }
    uLastLoadedSoundID = 0;
    if ( pSounds[0].pSoundData )
    {
      for ( uint i = 0; pSounds[i].pSoundData; i++ )
        ++uLastLoadedSoundID;
    }
    strcpy((char *)pSounds[uLastLoadedSoundID].SoundName, pSoundName);
    pSoundList->uTotalLoadedSoundSize += pAudioPlayer->pSoundHeaders[uFindSound_BinSearch_ResultID].uDecompressedSize;
    pSounds[uLastLoadedSoundID].pSoundData = pOutBuff;
    return pOutBuff;
  }
  else
    return 0;
}
