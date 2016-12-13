#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Bink_Smacker.h"







int (__stdcall *smackw32_SmackSoundUseMSS)(HDIGDRIVER) = 0;
unsigned int (__stdcall *smackw32_SmackUseMMX)(unsigned int) = 0;
HSMACK (__stdcall *smackw32_SmackOpen)(HANDLE, unsigned int, unsigned int) = 0;
HSMACKBLIT (__stdcall *smackw32_SmackBlitOpen)(unsigned int) = 0;
void (__stdcall *smackw32_SmackToBuffer)(HSMACK, unsigned int, unsigned int, unsigned int, unsigned int, void *, unsigned int) = 0;
void (__stdcall *smackw32_SmackBlitSetPalette)(HSMACKBLIT, void *, unsigned int) = 0;
unsigned int (__stdcall *smackw32_SmackDoFrame)(HSMACK) = 0;
unsigned int (__stdcall *smackw32_SmackToBufferRect)(HSMACK, unsigned int) = 0;
void (__stdcall *smackw32_SmackBlit)(HSMACKBLIT, void *, unsigned int, unsigned int, unsigned int, void *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) = 0;
void (__stdcall *smackw32_SmackNextFrame)(HSMACK) = 0;
unsigned int (__stdcall *smackw32_SmackWait)(HSMACK) = 0;
unsigned int (__stdcall *smackw32_SmackSoundOnOff)(HSMACK, unsigned int) = 0;
void (__stdcall *smackw32_SmackClose)(HSMACK) = 0;
void (__stdcall *smackw32_SmackBufferClose)(HSMACKBUF) = 0;
void (__stdcall *smackw32_SmackBlitClose)(HSMACKBLIT) = 0;
int  (__stdcall *smackw32_SmackBlitClear)(HSMACKBLIT, unsigned short *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int) = 0;
int  (__stdcall *smackw32_SmackGoto)(_SMACK *, long) = 0;
int  (__stdcall *smackw32_SmackBufferOpen)(HWND a1, long a2, long a3, long a4, long a5, long a6) = nullptr;
void(__stdcall *smackw32_SmackBufferNewPalette)(HSMACKBUF, void *, unsigned int) = nullptr;
void(__stdcall *smackw32_SmackColorRemapWithTrans)(_SMACK *, void *, unsigned int, unsigned int, unsigned int) = nullptr;
void SMACKW32_DLL_Initialize()
{
  HMODULE pDll = LoadLibraryW(L"SmackW32.dll");
  
  #define LOAD(x) smackw32_##x = (decltype(smackw32_##x))GetProcAddress(pDll, #x)
  {
    smackw32_SmackSoundUseMSS = (int (__stdcall *)(HDIGDRIVER))GetProcAddress(pDll, "_SmackSoundUseMSS@4");
    smackw32_SmackUseMMX = (unsigned int (__stdcall *)(unsigned int))GetProcAddress(pDll, "_SmackUseMMX@4");
    smackw32_SmackOpen = (HSMACK (__stdcall *)(HANDLE, unsigned int, unsigned int))GetProcAddress(pDll, "_SmackOpen@12");
    smackw32_SmackBlitOpen = (HSMACKBLIT (__stdcall *)(unsigned int))GetProcAddress(pDll, "_SmackBlitOpen@4");
    smackw32_SmackToBuffer = (void (__stdcall *)(HSMACK, unsigned int, unsigned int, unsigned int, unsigned int, void *, unsigned int))GetProcAddress(pDll, "_SmackToBuffer@28");
    smackw32_SmackBlitSetPalette = (void (__stdcall *)(HSMACKBLIT, void *, unsigned int))GetProcAddress(pDll, "_SmackBlitSetPalette@12");
    smackw32_SmackDoFrame = (unsigned int (__stdcall *)(HSMACK))GetProcAddress(pDll, "_SmackDoFrame@4");
    smackw32_SmackToBufferRect = (unsigned int (__stdcall *)(HSMACK, unsigned int))GetProcAddress(pDll, "_SmackToBufferRect@8");
    smackw32_SmackBlit = (void (__stdcall *)(HSMACKBLIT, void *, unsigned int, unsigned int, unsigned int, void *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))GetProcAddress(pDll, "_SmackBlit@44");
    smackw32_SmackNextFrame = (void (__stdcall *)(HSMACK))GetProcAddress(pDll, "_SmackNextFrame@4");
    smackw32_SmackWait = (unsigned int (__stdcall *)(HSMACK))GetProcAddress(pDll, "_SmackWait@4");
    smackw32_SmackSoundOnOff = (unsigned int (__stdcall *)(HSMACK, unsigned int))GetProcAddress(pDll, "_SmackSoundOnOff@8");
    smackw32_SmackClose = (void (__stdcall *)(HSMACK))GetProcAddress(pDll, "_SmackClose@4");
    smackw32_SmackBufferClose = (void (__stdcall *)(HSMACKBUF))GetProcAddress(pDll, "_SmackBufferClose@4");
    smackw32_SmackBlitClose = (void (__stdcall *)(HSMACKBLIT))GetProcAddress(pDll, "_SmackBlitClose@4");
    smackw32_SmackBlitClear = (int (__stdcall *)(HSMACKBLIT, unsigned short *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int))GetProcAddress(pDll, "_SmackBlitClear@32");
    smackw32_SmackGoto = (int  (__stdcall *)(_SMACK *, long))GetProcAddress(pDll, "_SmackGoto@8");
    smackw32_SmackBufferOpen = (int  (__stdcall *)(HWND, long, long, long, long, long))GetProcAddress(pDll, "_SmackBufferOpen@24");
	smackw32_SmackBufferNewPalette = (void(__stdcall *)(HSMACKBUF, void *, unsigned int))GetProcAddress(pDll, "_SmackBufferNewPalette@12");
	smackw32_SmackColorRemapWithTrans = (void(__stdcall *)(_SMACK *, void *, unsigned int, unsigned int, unsigned int))GetProcAddress(pDll, "_SmackColorRemapWithTrans@20");
    //LOAD(SmackBufferNewPalette);
    //LOAD(SmackColorRemapWithTrans);
  }
}




void __stdcall SmackColorRemapWithTrans(_SMACK *a1, void *a2, unsigned int a3, unsigned int a4, unsigned int a5)
{
  return (smackw32_SmackColorRemapWithTrans)(a1, a2, a3, a4, a5);
}

void __stdcall SmackBlitClose(HSMACKBLIT hBlit)
{
 (smackw32_SmackBlitClose)(hBlit);
}

void __stdcall SmackBufferClose(HSMACKBUF hBuf)
{
 (smackw32_SmackBufferClose)(hBuf);
}

void __stdcall SmackClose(HSMACK hSmack)
{
 (smackw32_SmackClose)(hSmack);
}

unsigned int __stdcall SmackSoundOnOff(HSMACK hSmack, unsigned int bOn)
{
 return (smackw32_SmackSoundOnOff)(hSmack, bOn);
}

unsigned int __stdcall SmackWait(HSMACK hSmack)
{
 return (smackw32_SmackWait)(hSmack);
}

void __stdcall SmackNextFrame(HSMACK hSmack)
{
 (smackw32_SmackNextFrame)(hSmack);
}

void __stdcall SmackBlit(HSMACKBLIT hBlit, void *pDest, unsigned int uDestPitch, unsigned int uDestX, unsigned int uDestY, void *pSrc, unsigned int uSrcPitch, unsigned int uSrcX, unsigned int uSrcY, unsigned int uSrcZ, unsigned int uSrcW)
{
 (smackw32_SmackBlit)(hBlit, pDest, uDestPitch, uDestX, uDestY, pSrc, uSrcPitch, uSrcX, uSrcY, uSrcZ, uSrcW);
}

unsigned int __stdcall SmackToBufferRect(HSMACK hSmack, unsigned int uSmackSurface)
{
 return (smackw32_SmackToBufferRect)(hSmack, uSmackSurface);
}

unsigned int __stdcall SmackDoFrame(HSMACK hSmack)
{
 return (smackw32_SmackDoFrame)(hSmack);
}

void __stdcall SmackBlitSetPalette(HSMACKBLIT hBlit, void *pPalette, unsigned int uPalType)
{
 (smackw32_SmackBlitSetPalette)(hBlit, pPalette, uPalType);
}

int __stdcall SmackSoundUseMSS(HDIGDRIVER hDrv)
{
 return (smackw32_SmackSoundUseMSS)(hDrv);
}

unsigned int __stdcall SmackUseMMX(unsigned int flag)
{
 return (smackw32_SmackUseMMX)(flag);
}

HSMACK __stdcall SmackOpen(HANDLE hSourceFile, unsigned int uFlags, unsigned int uExtraBuffers)
{
 return (smackw32_SmackOpen)(hSourceFile, uFlags, uExtraBuffers);
}

HSMACKBLIT __stdcall SmackBlitOpen(unsigned int uSurfaceFormat)
{
 return (smackw32_SmackBlitOpen)(uSurfaceFormat);
}

void __stdcall SmackToBuffer(HSMACK hSmack, unsigned int uX, unsigned int uY, unsigned int uPitch, unsigned int uHeight, void *pBuffer, unsigned int uFlags)
{
 (smackw32_SmackToBuffer)(hSmack, uX, uY, uPitch, uHeight, pBuffer, uFlags);
}

int __stdcall SmackBlitClear(HSMACKBLIT a1, unsigned short *pFrameData, unsigned int uTargetSurfacePitch, unsigned int uOutX, unsigned int uOutY, unsigned int uOutZ, unsigned int uOutW, int a8)
{
 return (smackw32_SmackBlitClear)(a1, pFrameData, uTargetSurfacePitch, uOutX, uOutY, uOutZ, uOutW, a8);
}

int __stdcall SmackGoto(_SMACK *a1, long a2)
{
 return (smackw32_SmackGoto)(a1, a2);
}


int __stdcall SmackBufferOpen(HWND a1, long a2, long a3, long a4, long a5, long a6)
{
  return (smackw32_SmackBufferOpen)(a1, a2, a3, a4, a5, a6);
}

int __stdcall SmackVolumePan(_SMACK *a3, long a4, long a5, long a6)
{
 //__asm int 3
 return 0;
}


// sub_4D83D0: using guessed type int __stdcall SmackBufferNewPalette(_DWORD, _DWORD, _DWORD);
void __stdcall SmackBufferNewPalette(HSMACKBUF a1, void *a2, unsigned int a3)
{
 (smackw32_SmackBufferNewPalette)(a1, a2, a3);
}































int (__stdcall *binkw32_BinkDDSurfaceType)(struct IDirectDrawSurface *) = 0;
int (__stdcall *binkw32_BinkSetSoundSystem)(void *, HDIGDRIVER) = 0;
int (__stdcall *binkw32_BinkOpenMiles)(int) = 0;
HBINK (__stdcall *binkw32_BinkOpen)(void *, unsigned int) = 0;
int (__stdcall *binkw32_BinkWait)(HBINK) = 0;
int (__stdcall *binkw32_BinkDoFrame)(HBINK) = 0;
int (__stdcall *binkw32_BinkNextFrame)(HBINK) = 0;
int (__stdcall *binkw32_BinkGetRects)(HBINK, unsigned int) = 0;
int (__stdcall *binkw32_BinkCopyToBuffer)(HBINK, void *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) = 0;
int (__stdcall *binkw32_BinkPause)(HBINK, int) = 0;
int (__stdcall *binkw32_BinkClose)(HBINK) = 0;
int (__stdcall *binkw32_BinkBufferSetOffset)(void *, int, int) = 0;
int (__stdcall *binkw32_BinkBufferSetScale)(void *, unsigned int, unsigned int) = 0;
void BINKW32_DLL_Initialize()
{
 HMODULE pDll = LoadLibraryW(L"BinkW32.dll");

 binkw32_BinkDDSurfaceType = (int (__stdcall *)(struct IDirectDrawSurface *))GetProcAddress(pDll, "_BinkDDSurfaceType@4");
 binkw32_BinkSetSoundSystem = (int (__stdcall *)(void *, HDIGDRIVER))GetProcAddress(pDll, "_BinkSetSoundSystem@8");
 binkw32_BinkOpenMiles = (int (__stdcall *)(int))GetProcAddress(pDll, "_BinkOpenMiles@4");
 binkw32_BinkOpen = (HBINK (__stdcall *)(void *, unsigned int))GetProcAddress(pDll, "_BinkOpen@8");
 binkw32_BinkWait = (int (__stdcall *)(HBINK))GetProcAddress(pDll, "_BinkWait@4");
 binkw32_BinkBufferSetOffset = (int (__stdcall *)(void *, int, int))GetProcAddress(pDll, "_BinkBufferSetOffset@12");
 binkw32_BinkBufferSetScale = (int (__stdcall *)(void *, unsigned int, unsigned int))GetProcAddress(pDll, "_BinkBufferSetScale@12");
 binkw32_BinkDoFrame = (int (__stdcall *)(HBINK))GetProcAddress(pDll, "_BinkDoFrame@4");
 binkw32_BinkNextFrame = (int (__stdcall *)(HBINK))GetProcAddress(pDll, "_BinkNextFrame@4");
 binkw32_BinkGetRects = (int (__stdcall *)(HBINK, unsigned int))GetProcAddress(pDll, "_BinkGetRects@8");
 binkw32_BinkCopyToBuffer = (int (__stdcall *)(HBINK, void *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))GetProcAddress(pDll, "_BinkCopyToBuffer@28");
 binkw32_BinkPause = (int (__stdcall *)(HBINK, int))GetProcAddress(pDll, "_BinkPause@8");
 binkw32_BinkClose = (int (__stdcall *)(HBINK))GetProcAddress(pDll, "_BinkClose@4");
}


int __stdcall BinkPause(HBINK hBink, int bPause)
{
 return (binkw32_BinkPause)(hBink, bPause);
}

int __stdcall BinkClose(HBINK hBink)
{
 return (binkw32_BinkClose)(hBink);
}

int __stdcall BinkGetRects(HBINK hBink, unsigned int uFlags)
{
 return (binkw32_BinkGetRects)(hBink, uFlags);
}

int __stdcall BinkCopyToBuffer(HBINK hBink, void *pBuffer, unsigned int lPitch, unsigned int uNumScanlines, unsigned int uX, unsigned int uY, unsigned int uFlags)
{
 return (binkw32_BinkCopyToBuffer)(hBink, pBuffer, lPitch, uNumScanlines, uX, uY, uFlags);
}

int __stdcall BinkDoFrame(HBINK hBink)
{
 return (binkw32_BinkDoFrame)(hBink);
}

int __stdcall BinkNextFrame(HBINK hBink)
{
 return (binkw32_BinkNextFrame)(hBink);
}

HBINK __stdcall BinkOpen(void *hFileHandle, unsigned int uFlags)
{
 return (binkw32_BinkOpen)(hFileHandle, uFlags);
}

int __stdcall BinkOpenMiles(int unk)
{
 return (binkw32_BinkOpenMiles)(unk);
}

int __stdcall BinkWait(HBINK hBink)
{
 return (binkw32_BinkWait)(hBink);
}



int __stdcall BinkBufferSetOffset(void *pStruct, int b, int c)
{
 return (binkw32_BinkBufferSetOffset)(pStruct, b, c);
}

int __stdcall BinkBufferSetScale(void *pStruct, unsigned int uWidth, unsigned int uHeight)
{
 return (binkw32_BinkBufferSetScale)(pStruct, uWidth, uHeight);
}

int __stdcall BinkDDSurfaceType(struct IDirectDrawSurface *pDDS)
{
 return (binkw32_BinkDDSurfaceType)(pDDS);
}

int __stdcall BinkSetSoundSystem(void *pSoundSystem, HDIGDRIVER hDrv)
{
 return (binkw32_BinkSetSoundSystem)(pSoundSystem, hDrv);
}






int __stdcall BinkGoto(_BINK *a1, long a2, long a3)
{
 __asm int 3
 return 0;
}




















