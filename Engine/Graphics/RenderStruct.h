#pragma once

#include <cstdint>
#include <cstdio>

#include "lib\legacy_dx\d3d.h"
#include "OSWindow.h"

#include "../VectorTypes.h"


#define ErrD3D(hr) \
  do \
  {  \
    extern void ErrHR(HRESULT, const char *, const char *, const char *, int); \
    ErrHR(hr, "Direct3D", __FUNCTION__, __FILE__, __LINE__); \
  } while(0)

struct Polygon;
struct Texture_MM7;
struct RGBTexture;
struct RenderBillboardTransform_local0;
struct ODMFace;


unsigned __int16 Color16(unsigned __int32 r, unsigned __int32 g, unsigned __int32 b);
unsigned __int32 Color32(unsigned __int16 color16);
unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);

/*  119 */
#pragma pack(push, 1)
struct RenderVertexSoft
{
  inline RenderVertexSoft():
    flt_2C(0.0f)
  {}

  Vec3_float_ vWorldPosition;
  Vec3_float_ vWorldViewPosition;
  float vWorldViewProjX;
  float vWorldViewProjY;
  float _rhw;
  float u;
  float v;
  float flt_2C;
};
#pragma pack(pop)



/*  112 */
#pragma pack(push, 1)
struct RenderVertexD3D3
{
  Vec3_float_ pos;
  float rhw;
  signed int diffuse;
  unsigned int specular;
  Vec2_float_ texcoord;
};
#pragma pack(pop)

class Sprite;
class SpriteFrame;

/*  161 */
#pragma pack(push, 1)
struct RenderBillboard
{
  int _screenspace_x_scaler_packedfloat;
  int _screenspace_y_scaler_packedfloat;
  float fov_x;
  float fov_y;
  union
  {
    int sZValue;
    struct
    {
      unsigned __int16 object_pid;
      signed __int16 actual_z;
    };
  };
  int field_14_actor_id;
  signed __int16 HwSpriteID;
  __int16 uPalette;
  __int16 uIndoorSectorID;
  __int16 field_1E;
  __int16 world_x;
  __int16 world_y;
  __int16 world_z;
  __int16 uScreenSpaceX;
  __int16 uScreenSpaceY;
  unsigned __int16 dimming_level;
  signed int sTintColor;
  SpriteFrame *pSpriteFrame;

   inline float GetFloatZ() const
 {
  return (float)object_pid / 65535.0f + (float)actual_z;
 }
};
#pragma pack(pop)


#pragma pack(push, 1)
struct RenderD3D__DevInfo
{
    inline RenderD3D__DevInfo():
        bIsDeviceCompatible(false), pName(nullptr), pDescription(nullptr),
        pGUID(nullptr), uCaps(0), pDriverName(nullptr), pDeviceDesc(nullptr),
        pDDraw4DevDesc(nullptr), pDirectDrawGUID(nullptr), uVideoMem(0)
    {}

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
#pragma pack(pop)



#pragma pack(push, 1)
struct RenderD3D_D3DDevDesc
{
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
#pragma pack(pop)









/*  280 */
#pragma pack(push, 1)
struct HWLTexture
{
  inline HWLTexture():
    field_0(0), field_4(0), field_8(0),
    field_C(0), field_10(0), field_14(0)
  {}

  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
  int uBufferWidth;
  int uBufferHeight;
  int uAreaWidth;
  int uAreaHeigth;
  unsigned int uWidth;
  unsigned int uHeight;
  int uAreaX;
  int uAreaY;
  unsigned __int16 *pPixels;
};
#pragma pack(pop)


/*  185 */
#pragma pack(push, 1)
struct RenderHWLContainer
{
       RenderHWLContainer();
  bool Load(const wchar_t *pFilename);
  bool Release();

  HWLTexture *LoadTexture(const char *pName, int bMipMaps);

  FILE *pFile;
  uint32_t uSignature;
  unsigned int uDataOffset;
  unsigned int uNumItems;
  char *pSpriteNames[50000];
  int pSpriteOffsets[50000];
  int bDumpDebug;
  int scale_hwls_to_half;
};
#pragma pack(pop)



/*  242 */
#pragma pack(push, 1)
struct RenderBillboardD3D
{
  inline RenderBillboardD3D():
    opacity(Transparent),
    field_90(-1),
    sParentBillboardID(-1),
    uNumVertices(4)
  {}

  enum OpacityType: unsigned __int32
  {
    Transparent = 0,
    Opaque_1 = 1,
    Opaque_2 = 2,
    Opaque_3 = 3,
    NoBlend = 0xFFFFFFFF
  };

  IDirect3DTexture2 *pTexture;
  unsigned int uNumVertices;
  RenderVertexD3D3 pQuads[4];
  float z_order;
  OpacityType opacity;
  int field_90;
  int sZValue;
  signed int sParentBillboardID;
};
#pragma pack(pop)




#pragma pack(push, 1)
struct RenderD3D_aux
{
  RenderD3D__DevInfo *pInfo;
  RenderD3D_D3DDevDesc *ptr_4;
};
#pragma pack(pop)



/*  183 */
#pragma pack(push, 1)
class RenderD3D
{
public:
  RenderD3D();
  
  void GetAvailableDevices(RenderD3D__DevInfo **pOutDevices);
  void Release();
  bool CreateDevice(unsigned int uDeviceID, int bWindowed, OSWindow *window);
  unsigned int GetDeviceCaps();
  void ClearTarget(unsigned int bClearColor, unsigned int uClearColor, unsigned int bClearDepth, float z_clear);
  void Present(bool bForceBlit);
  bool CreateTexture(unsigned int uTextureWidth, unsigned int uTextureHeight, IDirectDrawSurface4 **pOutSurface, IDirect3DTexture2 **pOutTexture, bool bAlphaChannel, bool bMipmaps, unsigned int uMinDeviceTexDim);
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
#pragma pack(pop)
extern struct RenderVertexD3D3  pVertices[50];

/*  182 */
#pragma pack(push, 1)

#pragma pack(pop)

bool PauseGameDrawing();

extern struct IDirectDrawClipper *pDDrawClipper;
extern struct IRender *pRenderer; // idb
extern struct pUnkTerrain *Unks;



/*  248 */
#pragma pack(push, 1)
struct RenderBillboardTransform_local0
{
  void *pTarget;
  int *pTargetZ;
  int uScreenSpaceX;
  int uScreenSpaceY;
  int _screenspace_x_scaler_packedfloat;
  int _screenspace_y_scaler_packedfloat;
  char field_18[8];
  unsigned __int16 *pPalette;
  unsigned __int16 *pPalette2;
  union
  {
    int sZValue;
    struct
    {
      unsigned short object_pid;
      short          zbuffer_depth;
    };
  };
  unsigned int uFlags;        // & 4   - mirror horizontally
  unsigned int uTargetPitch;
  unsigned int uViewportX;
  unsigned int uViewportY;
  unsigned int uViewportZ;
  unsigned int uViewportW;
  int field_44;
  int sParentBillboardID;
  int sTintColor;
};
#pragma pack(pop)




extern int uNumDecorationsDrawnThisFrame;
extern RenderBillboard pBillboardRenderList[500];
extern unsigned int uNumBillboardsToDraw;
extern int uNumSpritesDrawnThisFrame;




extern RenderVertexSoft array_507D30[50];
extern RenderVertexSoft VertexRenderList[50];
extern RenderVertexSoft array_73D150[20];

extern RenderVertexD3D3 d3d_vertex_buffer[50];


int ODM_NearClip(unsigned int uVertexID); // idb
int ODM_FarClip(unsigned int uNumVertices);