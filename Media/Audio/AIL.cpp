#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "AIL.h"




int (__stdcall *mss32_AIL_startup)() = 0;
HREDBOOK (__stdcall *mss32_AIL_redbook_open_drive)(long) = 0;
int (__stdcall *mss32_AIL_set_preference)(unsigned int, int) = 0;
int (__stdcall *mss32_AIL_waveOutOpen)(HDIGDRIVER *, HWAVEOUT *, int, WAVEFORMAT *) = 0;
int (__stdcall *mss32_AIL_get_preference)(unsigned int) = 0;
int (__stdcall *mss32_AIL_digital_configuration)(HDIGDRIVER, int *, int *, char *) = 0;
HSAMPLE (__stdcall *mss32_AIL_allocate_sample_handle)(HDIGDRIVER) = 0;
void (__stdcall *mss32_AIL_3D_provider_attribute)(HPROVIDER, char *, void *) = 0;
unsigned int (__stdcall *mss32_AIL_redbook_tracks)(HREDBOOK) = 0;
int (__stdcall *mss32_AIL_redbook_volume)(HREDBOOK) = 0;
unsigned int (__stdcall *mss32_AIL_redbook_stop)(HREDBOOK) = 0;
void (__stdcall *mss32_AIL_set_digital_master_volume)(HDIGDRIVER, float) = 0;
int (__stdcall *mss32_AIL_redbook_set_volume)(HREDBOOK, int) = 0;
void (__stdcall *mss32_AIL_waveOutClose)(HDIGDRIVER) = 0;
unsigned int (__stdcall *mss32_AIL_redbook_pause)(HREDBOOK) = 0;
void (__stdcall *mss32_AIL_redbook_track_info)(HREDBOOK, unsigned int, unsigned int *, unsigned int *) = 0;
unsigned int (__stdcall *mss32_AIL_redbook_play)(HREDBOOK, unsigned int, unsigned int) = 0;
unsigned int (__stdcall *mss32_AIL_redbook_resume)(HREDBOOK) = 0;
AIL::Sample::Status (__stdcall *mss32_AIL_sample_status)(HSAMPLE) = 0;
int (__stdcall *mss32_AIL_sample_volume)(HSAMPLE) = 0;
int (__stdcall *mss32_AIL_enumerate_3D_providers)(int *, HPROVIDER *, char **) = 0;
DWORD (__stdcall *mss32_AIL_open_3D_provider)(HPROVIDER) = 0;
int (__stdcall *mss32_AIL_end_sample)(HSAMPLE) = 0;
int (__stdcall *mss32_AIL_set_sample_volume)(HSAMPLE, long) = 0;
int (__stdcall *mss32_AIL_set_sample_pan)(HSAMPLE, long) = 0;
void (__stdcall *mss32_AIL_end_sequence)(HSEQUENCE) = 0;
void (__stdcall *mss32_AIL_pause_stream)(HSTREAM, int) = 0;
void (__stdcall *mss32_AIL_init_sample)(HSAMPLE) = 0;
int (__stdcall *mss32_AIL_set_sample_file)(HSAMPLE, const void *, int) = 0;
void (__stdcall *mss32_AIL_set_sample_loop_count)(HSAMPLE, int) = 0;
void (__stdcall *mss32_AIL_set_sample_playback_rate)(HSAMPLE, int) = 0;
void (__stdcall *mss32_AIL_sample_ms_position)(HSAMPLE, int *, int *) = 0;
int (__stdcall *mss32_AIL_start_sample)(HSAMPLE) = 0;
AILFILETYPE (__stdcall *mss32_AIL_file_type)(void *, int) = 0;
int (__stdcall *mss32_AIL_WAV_info)(void *, AILSOUNDINFO *) = 0;
int (__stdcall *mss32_AIL_decompress_ADPCM)(AILSOUNDINFO *, void *, void *) = 0;
HREDBOOK (__stdcall *mss32_AIL_redbook_open)(int) = 0;
void (__stdcall *mss32_AIL_release_sample_handle)(HSAMPLE) = 0;
void MSS32_DLL_Initialize()
{
 HMODULE pDll = LoadLibraryW(L"mss32.dll");

 mss32_AIL_startup = (int (__stdcall *)())GetProcAddress(pDll, "_AIL_startup@0");
 mss32_AIL_redbook_open_drive = (HREDBOOK (__stdcall *)(long))GetProcAddress(pDll, "_AIL_redbook_open_drive@4");
 mss32_AIL_set_preference = (int (__stdcall *)(unsigned int, int))GetProcAddress(pDll, "_AIL_set_preference@8");
 mss32_AIL_waveOutOpen = (int (__stdcall *)(HDIGDRIVER *, HWAVEOUT *, int, WAVEFORMAT *))GetProcAddress(pDll, "_AIL_waveOutOpen@16");
 mss32_AIL_get_preference = (int (__stdcall *)(unsigned int))GetProcAddress(pDll, "_AIL_get_preference@4");
 mss32_AIL_digital_configuration = (int (__stdcall *)(HDIGDRIVER, int *, int *, char *))GetProcAddress(pDll, "_AIL_digital_configuration@16");
 mss32_AIL_allocate_sample_handle = (HSAMPLE (__stdcall *)(HDIGDRIVER))GetProcAddress(pDll, "_AIL_allocate_sample_handle@4");
 mss32_AIL_3D_provider_attribute = (void (__stdcall *)(HPROVIDER,char *, void *))GetProcAddress(pDll, "_AIL_3D_provider_attribute@12");
 mss32_AIL_redbook_tracks = (unsigned int (__stdcall *)(HREDBOOK))GetProcAddress(pDll, "_AIL_redbook_tracks@4");
 mss32_AIL_redbook_volume = (int (__stdcall *)(HREDBOOK))GetProcAddress(pDll, "_AIL_redbook_volume@4");
 mss32_AIL_redbook_stop = (unsigned int (__stdcall *)(HREDBOOK))GetProcAddress(pDll, "_AIL_redbook_stop@4");
 mss32_AIL_set_digital_master_volume = (void (__stdcall *)(HDIGDRIVER, float))GetProcAddress(pDll, "_AIL_set_digital_master_volume@8");
 mss32_AIL_redbook_set_volume = (int (__stdcall *)(HREDBOOK, int))GetProcAddress(pDll, "_AIL_redbook_set_volume@8");
 mss32_AIL_waveOutClose = (void (__stdcall *)(HDIGDRIVER))GetProcAddress(pDll, "_AIL_waveOutClose@4");
 mss32_AIL_redbook_pause = (unsigned int (__stdcall *)(HREDBOOK))GetProcAddress(pDll, "_AIL_redbook_pause@4");
 mss32_AIL_redbook_track_info = (void (__stdcall *)(HREDBOOK, unsigned int, unsigned int *, unsigned int *))GetProcAddress(pDll, "_AIL_redbook_track_info@16");
 mss32_AIL_redbook_play = (unsigned int (__stdcall *)(HREDBOOK, unsigned int, unsigned int))GetProcAddress(pDll, "_AIL_redbook_play@12");
 mss32_AIL_redbook_resume = (unsigned int (__stdcall *)(HREDBOOK))GetProcAddress(pDll, "_AIL_redbook_resume@4");
 mss32_AIL_sample_status = (AIL::Sample::Status (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_sample_status@4");
 mss32_AIL_sample_volume = (int (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_sample_volume@4");
 mss32_AIL_enumerate_3D_providers = (int (__stdcall *)(int *, HPROVIDER *, char **))GetProcAddress(pDll, "_AIL_enumerate_3D_providers@12");
 mss32_AIL_open_3D_provider = (DWORD (__stdcall *)(HPROVIDER))GetProcAddress(pDll, "_AIL_open_3D_provider@4");
 mss32_AIL_end_sample = (int (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_end_sample@4");
 mss32_AIL_set_sample_volume = (int (__stdcall *)(HSAMPLE, long))GetProcAddress(pDll, "_AIL_set_sample_volume@8");
 mss32_AIL_set_sample_pan = (int (__stdcall *)(HSAMPLE, long))GetProcAddress(pDll, "_AIL_set_sample_pan@8");
 mss32_AIL_end_sequence = (void (__stdcall *)(HSEQUENCE))GetProcAddress(pDll, "_AIL_end_sequence@4");
 mss32_AIL_pause_stream = (void (__stdcall *)(HSTREAM, int))GetProcAddress(pDll, "_AIL_pause_stream@8");
 mss32_AIL_init_sample = (void (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_init_sample@4");
 mss32_AIL_set_sample_file = (int (__stdcall *)(HSAMPLE, const void *, int))GetProcAddress(pDll, "_AIL_set_sample_file@12");
 mss32_AIL_set_sample_loop_count = (void (__stdcall *)(HSAMPLE, int))GetProcAddress(pDll, "_AIL_set_sample_loop_count@8");
 mss32_AIL_set_sample_playback_rate = (void (__stdcall *)(HSAMPLE, int))GetProcAddress(pDll, "_AIL_set_sample_playback_rate@8");
 mss32_AIL_sample_ms_position = (void (__stdcall *)(HSAMPLE, int *, int *))GetProcAddress(pDll, "_AIL_sample_ms_position@12");
 mss32_AIL_start_sample = (int (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_start_sample@4");
 mss32_AIL_file_type = (AILFILETYPE (__stdcall *)(void *, int))GetProcAddress(pDll, "_AIL_file_type@8");
 mss32_AIL_WAV_info = (int (__stdcall *)(void *, AILSOUNDINFO *))GetProcAddress(pDll, "_AIL_WAV_info@8");
 mss32_AIL_decompress_ADPCM = (int (__stdcall *)(AILSOUNDINFO *, void *, void *))GetProcAddress(pDll, "_AIL_decompress_ADPCM@12");
 mss32_AIL_redbook_open = (HREDBOOK (__stdcall *)(int))GetProcAddress(pDll, "_AIL_redbook_open@4");
 mss32_AIL_release_sample_handle = (void (__stdcall *)(HSAMPLE))GetProcAddress(pDll, "_AIL_release_sample_handle@4");
}



unsigned int __stdcall AIL_redbook_play(HREDBOOK hRedbook, unsigned int uStartMS, unsigned int uEndMS)
{
 return (mss32_AIL_redbook_play)(hRedbook, uStartMS, uEndMS);
}

void __stdcall AIL_redbook_track_info(HREDBOOK hRedbook, unsigned int uTrackNum, unsigned int *pStartMS, unsigned int *pEndMS)
{
 (mss32_AIL_redbook_track_info)(hRedbook, uTrackNum, pStartMS, pEndMS);
}

unsigned int __stdcall AIL_redbook_resume(HREDBOOK hRedbook)
{
 return (mss32_AIL_redbook_resume)(hRedbook);
}

int __stdcall AIL_enumerate_3D_providers(int *a1, HPROVIDER *pOutProv, char **pOutName)
{
 return (mss32_AIL_enumerate_3D_providers)(a1, pOutProv, pOutName);
}
HREDBOOK __stdcall AIL_redbook_open(int w)
{
 return (mss32_AIL_redbook_open)(w);
}


int __stdcall AIL_sample_volume(HSAMPLE s)
{
 return (mss32_AIL_sample_volume)(s);
}

AIL::Sample::Status __stdcall AIL_sample_status(HSAMPLE a1)
{
 return (mss32_AIL_sample_status)(a1);
}

// sub_4D8304: using guessed type int __stdcall AIL_set_digital_master_volume(_DWORD, _DWORD);
void __stdcall AIL_set_digital_master_volume(HDIGDRIVER hDrv, float master_volume)
{
 (mss32_AIL_set_digital_master_volume)(hDrv, master_volume);
}

// sub_4D8370: using guessed type int __stdcall AIL_allocate_sample_handle(_DWORD);
HSAMPLE __stdcall AIL_allocate_sample_handle(HDIGDRIVER hDrv)
{
 return (mss32_AIL_allocate_sample_handle)(hDrv);
}

// sub_4D8308: using guessed type int __fastcall AIL_redbook_set_volume(_DWORD, _DWORD, _DWORD, _DWORD);
int __stdcall AIL_redbook_set_volume(HREDBOOK hRedbook, int volume)
{
 return (mss32_AIL_redbook_set_volume)(hRedbook, volume);
}

// sub_4D8324: using guessed type int __stdcall AIL_redbook_stop(_DWORD);
unsigned int __stdcall AIL_redbook_stop(HREDBOOK hRedbook)
{
 return (mss32_AIL_redbook_stop)(hRedbook);
}

// sub_4D835C: using guessed type int __stdcall AIL_startup();
int __stdcall AIL_startup()
{
 return (mss32_AIL_startup)();
}

// sub_4D8360: using guessed type int __stdcall AIL_redbook_open_drive(_DWORD);
HREDBOOK __stdcall AIL_redbook_open_drive(long drive)
{
 return (mss32_AIL_redbook_open_drive)(drive);
}

// sub_4D834C: using guessed type int __stdcall AIL_waveOutOpen(_DWORD, _DWORD, _DWORD, _DWORD);
int __stdcall AIL_waveOutOpen(_DIG_DRIVER **drv, HWAVEOUT *phWaveOut, int wDeviceID, WAVEFORMAT *pFormat)
{
 return (mss32_AIL_waveOutOpen)(drv, phWaveOut, wDeviceID, pFormat);
}


DWORD __stdcall AIL_open_3D_provider(HPROVIDER a2)
{
 return (mss32_AIL_open_3D_provider)(a2);
}

void __stdcall AIL_3D_provider_attribute(HPROVIDER lib, char *name, void *val)
{ 
 (mss32_AIL_3D_provider_attribute)(lib,name,val);
}

// sub_4D8374: using guessed type int __stdcall AIL_redbook_tracks(_DWORD);
unsigned int __stdcall AIL_redbook_tracks(HREDBOOK hRedbook)
{
 return (mss32_AIL_redbook_tracks)(hRedbook);
}

// sub_4D83B0: using guessed type int __stdcall AIL_redbook_volume(_DWORD);
int __stdcall AIL_redbook_volume(HREDBOOK hRedbook)
{
 return (mss32_AIL_redbook_volume)(hRedbook);
}

// sub_4D8348: using guessed type int __stdcall AIL_set_preference(_DWORD, _DWORD);
int __stdcall AIL_set_preference(unsigned int number, int value)
{
 return (mss32_AIL_set_preference)(number, value);
}

// sub_4D8350: using guessed type int __stdcall AIL_digital_configuration(_DWORD, _DWORD, _DWORD, _DWORD);
int __stdcall AIL_digital_configuration(HDIGDRIVER drv, int *rate, int *format, char *string)
{
 return (mss32_AIL_digital_configuration)(drv, rate, format, string);
}

// sub_4D8354: using guessed type int __stdcall AIL_get_preference(_DWORD);
int __stdcall AIL_get_preference(unsigned int number)
{
 return (mss32_AIL_get_preference)(number);
}

void __stdcall AIL_waveOutClose(HDIGDRIVER drvr)
{
 (mss32_AIL_waveOutClose)(drvr);
}

// sub_4D8320: using guessed type int __stdcall AIL_redbook_pause(_DWORD);
unsigned int __stdcall AIL_redbook_pause(HREDBOOK hRedbook)
{
 return (mss32_AIL_redbook_pause)(hRedbook);
}

int __stdcall AIL_set_3D_provider_preference(HPROVIDER a1, const char *a2, int *a3)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_allocate_3D_sample_handle(HPROVIDER)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_sample_float_distances(void *a1, long a2, long a3, long a4, long a5)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_sample_volume(void *a1, long a2)
{
 __asm int 3
 return 0;
}

void __stdcall AIL_release_sample_handle(HSAMPLE s)
{
 (mss32_AIL_release_sample_handle)(s);
}

int __stdcall AIL_release_3D_sample_handle(void *a1)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_close_3D_provider(HPROVIDER)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_redbook_close(HREDBOOK a1)
{
 __asm int 3
 return 0;
}

// sub_4D8344: using guessed type int __stdcall AIL_shutdown();
int __stdcall AIL_shutdown()
{
 __asm int 3
 return 0;
}

int __stdcall AIL_end_sample(HSAMPLE a1)
{
 return (mss32_AIL_end_sample)(a1);
}

int __stdcall AIL_end_3D_sample(void *a1)
{
 __debugbreak();
 return 0;
}


void __stdcall AIL_end_sequence(HSEQUENCE a1)
{
 (mss32_AIL_end_sequence)(a1);
}

void __stdcall AIL_pause_stream(HSTREAM a1, int onoff)
{
 (mss32_AIL_pause_stream)(a1, onoff);
}

int __stdcall AIL_set_sample_file(HSAMPLE s, const void *file_image, int block)
{
 return (mss32_AIL_set_sample_file)(s, file_image, block);
}

int __stdcall AIL_start_sample(HSAMPLE s)
{
 return (mss32_AIL_start_sample)(s);
}

void __stdcall AIL_set_sample_playback_rate(HSAMPLE s, int rate)
{
 (mss32_AIL_set_sample_playback_rate)(s, rate);
}

void __stdcall AIL_sample_ms_position(HSAMPLE s, int *pTotalMS, int *pCurrentMS)
{
 (mss32_AIL_sample_ms_position)(s, pTotalMS, pCurrentMS);
}

int __stdcall AIL_3D_sample_status(void *a1)
{
 __asm int 3
 return 0;
}

void __stdcall AIL_set_sample_loop_count(HSAMPLE s, int num)
{
 (mss32_AIL_set_sample_loop_count)(s, num);
}

int __stdcall AIL_set_sample_volume(HSAMPLE a1, long a2)
{
 return (mss32_AIL_set_sample_volume)(a1, a2);
}

int __stdcall AIL_3D_position(void *a1, int *a2, float *a3, long *a4)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_sample_file(long a1, void *a2)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_sample_loop_count(long a1, long a2)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_start_3D_sample(long a1)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_position(void *hSample, long a2, long a3, long a4)
{
 __asm int 3
 return 0;
}

int __stdcall AIL_set_3D_orientation(void *hSample, long a2, long a3, long a4, long a5, long a6, long a7)
{
 __asm int 3
 return 0;
}

void __stdcall AIL_init_sample(HSAMPLE a1)
{
 (mss32_AIL_init_sample)(a1);
}

int __stdcall AIL_set_sample_pan(HSAMPLE a1, long a2)
{
 return (mss32_AIL_set_sample_pan)(a1, a2);
}


AILFILETYPE __stdcall AIL_file_type(void *pSoundBytes, int numBytes)
{
  return (mss32_AIL_file_type)(pSoundBytes, numBytes);
}

int __stdcall AIL_WAV_info(void *pSoundBytes, AILSOUNDINFO *pInfo)
{
  return (mss32_AIL_WAV_info)(pSoundBytes, pInfo);
}

int __stdcall AIL_decompress_ADPCM(AILSOUNDINFO *pInfo, void *a2, void *a3)
{
  return (mss32_AIL_decompress_ADPCM)(pInfo, a2, a3);
}

int __stdcall AIL_mem_free_lock(void *a1)
{
 __asm int 3
 return 0;
}