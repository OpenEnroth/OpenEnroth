#pragma once
#include <memory>

#include <d3d.h>
#include <ddraw.h>

#include "Engine/VectorTypes.h"

#define ErrD3D(hr)                                                           \
    do {                                                                     \
        extern void ErrHR(HRESULT, const char *, const char *, const char *, \
                          int);                                              \
        ErrHR(hr, "Direct3D", __FUNCTION__, __FILE__, __LINE__);             \
    } while (0)

struct Polygon;
struct Texture_MM7;
struct ODMFace;

struct RenderD3D__DevInfo {
    inline RenderD3D__DevInfo()
        : bIsDeviceCompatible(false),
          pName(nullptr),
          pDescription(nullptr),
          pGUID(nullptr),
          uCaps(0),
          pDriverName(nullptr),
          pDeviceDesc(nullptr),
          pDDraw4DevDesc(nullptr),
          pDirectDrawGUID(nullptr),
          uVideoMem(0) {}

    unsigned int bIsDeviceCompatible;
    char *pName;
    char *pDescription;
    GUID *pGUID;
    unsigned int uCaps;
    char *pDriverName;
    char *pDeviceDesc;
    char *pDDraw4DevDesc;
    GUID *pDirectDrawGUID;
    int uVideoMem;
};

class Sprite;
class SpriteFrame;

struct RenderD3D_D3DDevDesc {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    char *pDriverName;
    char *pDeviceDesc;
    char *pDDraw4DevDesc;
    GUID *pGUID;
    unsigned int uVideoMem;
};

struct RenderD3D_aux {
    RenderD3D__DevInfo *pInfo;
    RenderD3D_D3DDevDesc *ptr_4;
};

class OSWindow;

class RenderD3D {
 public:
    RenderD3D();

    void GetAvailableDevices(RenderD3D__DevInfo **pOutDevices);
    void Release();
    bool CreateDevice(unsigned int uDeviceID, int bWindowed, std::shared_ptr<OSWindow> window);
    unsigned int GetDeviceCaps();
    void ClearTarget(unsigned int bClearColor, unsigned int uClearColor,
                     unsigned int bClearDepth, float z_clear);
    void Present(bool bForceBlit);
    bool CreateTexture(unsigned int uTextureWidth, unsigned int uTextureHeight,
                       IDirectDrawSurface4 **pOutSurface,
                       IDirect3DTexture2 **pOutTexture, bool bAlphaChannel,
                       bool bMipmaps, unsigned int uMinDeviceTexDim);
    void HandleLostResources();

    unsigned int bWindowed;
    int field_4;
    int field_8;
    HWND hWindow;
    int field_10;
    int field_14;
    int field_18;
    RenderD3D__DevInfo *pAvailableDevices;
    IDirectDraw4 *pHost;
    IDirect3D3 *pDirect3D;
    IUnknown *pUnk;
    IDirectDrawSurface4 *pBackBuffer;
    IDirectDrawSurface4 *pFrontBuffer;
    IDirectDrawSurface4 *pZBuffer;
    IDirect3DDevice3 *pDevice;
    IDirect3DViewport3 *pViewport;
    int field_40;
    int field_44;
    char pErrorMessage[48];
    char field_78[208];
};

extern struct RenderVertexD3D3 pVertices[50];

extern struct IDirectDrawClipper *pDDrawClipper;
extern struct pUnkTerrain *Unks;
