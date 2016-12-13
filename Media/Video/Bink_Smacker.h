#pragma once
#include "Media/Audio/AIL.h"




void BINKW32_DLL_Initialize();


int __stdcall BinkDDSurfaceType(struct IDirectDrawSurface *);
int __stdcall BinkSetSoundSystem(void *pSoundSystem, HDIGDRIVER hDrv);
int __stdcall BinkOpenMiles(int);
HBINK __stdcall BinkOpen(void *hFileHandle, unsigned int uFlags);
int __stdcall BinkWait(HBINK);
int __stdcall BinkDoFrame(HBINK);
int __stdcall BinkNextFrame(HBINK);
int __stdcall BinkGetRects(HBINK hBink, unsigned int uFlags);
int __stdcall BinkCopyToBuffer(HBINK hBink, void *pBuffer, unsigned int lPitch, unsigned int uNumLines, unsigned int uX, unsigned int uY, unsigned int uFlags);
int __stdcall BinkPause(HBINK, int bPause);
int __stdcall BinkClose(HBINK);
int __stdcall BinkGoto(HBINK, long a2, long a3);

int __stdcall BinkBufferSetOffset(void *, int, int);
int __stdcall BinkBufferSetScale(void *, unsigned int uWidth, unsigned int uHeight);

  void SMACKW32_DLL_Initialize();

struct _SMACK
{
  unsigned int Version;           // SMK2 only right now
  unsigned int Width;             // Width (1 based, 640 for example)
  unsigned int Height;            // Height (1 based, 480 for example)
  unsigned int Frames;            // Number of frames (1 based, 100 = 100 frames)
  unsigned int MSPerFrame;        // Frame Rate
  unsigned int SmackerType;       // bit 0 set=ring frame
  unsigned int LargestInTrack[7]; // Largest single size for each track
  unsigned int tablesize;         // Size of the init tables
  unsigned int codesize;          // Compression info   
  unsigned int absize;            // ditto
  unsigned int detailsize;        // ditto
  unsigned int typesize;          // ditto
  unsigned int TrackType[7];      // high byte=0x80-Comp,0x40-PCM data,0x20-16 bit,0x10-stereo
  unsigned int extra;             // extra value (should be zero)
  unsigned int NewPalette;        // set to one if the palette changed
  unsigned char Palette[772];      // palette data
  unsigned int PalType;           // type of palette
  unsigned int FrameNum;          // 0374 Frame Number to be displayed
  unsigned int FrameSize;         // The current frame's size in bytes
  unsigned int SndSize;           // The current frame sound tracks' size in bytes
  int LastRectx;                  // 0380 Rect set in from SmackToBufferRect (X coord)
  int LastRecty;                  // Rect set in from SmackToBufferRect (Y coord)
  int LastRectw;                  // Rect set in from SmackToBufferRect (Width)
  int LastRecth;                  // 038C Rect set in from SmackToBufferRect (Height)
  unsigned int OpenFlags;         // flags used on open
  unsigned int LeftOfs;           // Left Offset used in SmackTo
  unsigned int TopOfs;            // Top Offset used in SmackTo
  unsigned int LargestFrameSize;  // Largest frame size
  unsigned int Highest1SecRate;   // Highest 1 sec data rate
  unsigned int Highest1SecFrame;  // Highest 1 sec data rate starting frame
  unsigned int ReadError;         // Set to non-zero if a read error has ocurred
  unsigned int addr32;            // translated address for 16 bit interface
};



struct _SMACKBLIT
{
  unsigned int    Flags;
  unsigned char  *Palette;
  unsigned int    PalType;
  unsigned short *SmoothTable;
  unsigned short *Conv8to16Table;
  unsigned int    whichmode;
  unsigned int    palindex;
  unsigned int    t16index;
  unsigned int    smoothindex;
  unsigned int    smoothtype;
  unsigned int    firstpalette;
};

struct _SMACKBUF
{
        unsigned int Reversed;             // 1 if the buffer is upside down
        unsigned int SurfaceType;          // SMACKSURFACExxxx defines
        unsigned int BlitType;             // SMACKxxxxBLIT defines
        unsigned int FullScreen;           // 1 if full-screen
        unsigned int Width;
        unsigned int Height;
        unsigned int Pitch;
        unsigned int Zoomed;
        unsigned int ZWidth;
        unsigned int ZHeight;
        unsigned int DispColors;           // colors on the screen
        unsigned int MaxPalColors;         // total possible colors in palette (usually 256)
        unsigned int PalColorsInUse;       // Used colors in palette (usually 236)
        unsigned int StartPalColor;        // first usable color index (usually 10)
        unsigned int EndPalColor;          // last usable color index (usually 246)
        RGBQUAD Palette[256];
        unsigned int PalType;
        unsigned int forceredraw;  // force a complete redraw on next blit (for >8bit)
        unsigned int didapalette;  // force an invalidate on the next palette change

        void * Buffer;
        void * DIBRestore;
        unsigned int OurBitmap;
        unsigned int OrigBitmap;
        unsigned int OurPalette;
        unsigned int WinGDC;
        unsigned int FullFocused;
        unsigned int ParentHwnd;
        unsigned int OldParWndProc;
        unsigned int OldDispWndProc;
        unsigned int DispHwnd;
        unsigned int WinGBufHandle;
        void * lpDD;
        void * lpDDSP;
        unsigned int DDSurfaceType;
        struct _SMACKBLIT DDblit;
        int ddSoftwarecur;
        int didaddblit;
        int lastwasdd;
        RECT ddscreen;
        int manyblits;
        int * blitrects;
        int * rectsptr;
        int maxrects;
        int numrects;
        HDC lastdc;
};
#define BINKFRAMERATE            0x00001000L // Override fr (call BinkFrameRate first)
#define BINKPRELOADALL            0x00002000L // Preload the entire animation
#define BINKSNDTRACK            0x00004000L // Set the track number to play
#define BINKOLDFRAMEFORMAT        0x00008000L // using the old Bink frame format (internal use only)
#define BINKRBINVERT            0x00010000L // use reversed R and B planes (internal use only)
#define BINKGRAYSCALE            0x00020000L // Force Bink to use grayscale
#define BINKNOMMX                0x00040000L // Don't use MMX
#define BINKNOSKIP                0x00080000L // Don't skip frames if falling behind
#define BINKALPHA                0x00100000L // Decompress alpha plane (if present)
#define BINKNOFILLIOBUF            0x00200000L // Fill the IO buffer in SmackOpen
#define BINKSIMULATE            0x00400000L // Simulate the speed (call BinkSim first)
#define BINKFILEHANDLE            0x00800000L // Use when passing in a file handle
#define BINKIOSIZE                0x01000000L // Set an io size (call BinkIOSize first)
#define BINKIOPROCESSOR            0x02000000L // Set an io processor (call BinkIO first)
#define BINKFROMMEMORY            0x04000000L // Use when passing in a pointer to the file
#define BINKNOTHREADEDIO        0x08000000L // Don't use a background thread for IO

#define BINKSURFACEFAST            0x00000000L
#define BINKSURFACESLOW            0x08000000L
#define BINKSURFACEDIRECT        0x04000000L

#define BINKCOPYALL                0x80000000L // copy all pixels (not just changed)
#define BINKCOPY2XH                0x10000000L // Force doubling height scaling
#define BINKCOPY2XHI            0x20000000L // Force interleaving height scaling
#define BINKCOPY2XW                0x30000000L // copy the width zoomed by two
#define BINKCOPY2XWH            0x40000000L // copy the width and height zoomed by two
#define BINKCOPY2XWHI            0x50000000L // copy the width and height zoomed by two
#define BINKCOPY1XI                0x60000000L // copy the width and height zoomed by two
#define BINKCOPYNOSCALING        0x70000000L // Force scaling off

#define SMACKNEEDPAN    0x00020L // Will be setting the pan
#define SMACKNEEDVOLUME 0x00040L // Will be setting the volume
#define SMACKFRAMERATE  0x00080L // Override fr (call SmackFrameRate first)
#define SMACKLOADEXTRA  0x00100L // Load the extra buffer during SmackOpen
#define SMACKPRELOADALL 0x00200L // Preload the entire animation
#define SMACKNOSKIP     0x00400L // Don't skip frames if falling behind
#define SMACKSIMULATE   0x00800L // Simulate the speed (call SmackSim first)
#define SMACKFILEHANDLE 0x01000L // Use when passing in a file handle
#define SMACKTRACK1     0x02000L // Play audio track 1
#define SMACKTRACK2     0x04000L // Play audio track 2
#define SMACKTRACK3     0x08000L // Play audio track 3
#define SMACKTRACK4     0x10000L // Play audio track 4
#define SMACKTRACK5     0x20000L // Play audio track 5
#define SMACKTRACK6     0x40000L // Play audio track 6
#define SMACKTRACK7     0x80000L // Play audio track 7


#define SMACKBUFFER555      0x80000000
#define SMACKBUFFER565      0xC0000000

#define SMACKBLIT1X                1
#define SMACKBLIT2X                2
#define SMACKBLIT2XSMOOTHING       4
#define SMACKBLIT2XINTERLACE       8


int __stdcall SmackSoundUseMSS(HDIGDRIVER hDrv);
unsigned int __stdcall SmackUseMMX(unsigned int flag);
HSMACK __stdcall SmackOpen(HANDLE hSourceFile, unsigned int uFlags, unsigned int uExtraBuffers);
HSMACKBLIT __stdcall SmackBlitOpen(unsigned int uSurfaceFormat);
void __stdcall SmackToBuffer(HSMACK, unsigned int uX, unsigned int uY, unsigned int uPitch, unsigned int uHeight, void *pBuffer, unsigned int uFlags);
void __stdcall SmackBlitSetPalette(HSMACKBLIT hBlit, void *pPalette, unsigned int uPalType);
unsigned int __stdcall SmackDoFrame(HSMACK);
unsigned int __stdcall SmackToBufferRect(HSMACK, unsigned int uSmackSurface);
void __stdcall SmackBlit(HSMACKBLIT, void *pDest, unsigned int uDestPitch, unsigned int uDestX, unsigned int uDestY, void *pSrc, unsigned int uSrcPitch, unsigned int uSrcX, unsigned int uSrcY, unsigned int uSrcZ, unsigned int uSrcW);
void __stdcall SmackNextFrame(HSMACK);
unsigned int __stdcall SmackWait(HSMACK);
unsigned int __stdcall SmackSoundOnOff(HSMACK, unsigned int bOn);
void __stdcall SmackClose(HSMACK);
void __stdcall SmackBufferClose(HSMACKBUF);
void __stdcall SmackBlitClose(HSMACKBLIT);
int __stdcall SmackBlitClear(HSMACKBLIT a1, unsigned short *pFrameData, unsigned int uTargetSurfacePitch, unsigned int uOutX, unsigned int uOutY, unsigned int uOutZ, unsigned int uOutW, int a8);





int __stdcall SmackBufferOpen(HWND a1, long a2, long a3, long a4, long a5, long a6);
int __stdcall SmackVolumePan(_SMACK *a3, long a4, long a5, long a6);

int __stdcall SmackGoto(_SMACK *a1, long a2);

// sub_4D83D0: using guessed type int __stdcall SmackBufferNewPalette(_DWORD, _DWORD, _DWORD);
void __stdcall SmackBufferNewPalette(HSMACKBUF a1, void *a2, unsigned int a3);

// sub_4D83D4: using guessed type int __stdcall SmackColorRemapWithTrans(_DWORD, _DWORD, _DWORD, _DWORD, _DWORD);
void __stdcall SmackColorRemapWithTrans(_SMACK *a1, void *a2, unsigned int a3, unsigned int a4, unsigned int a5);














