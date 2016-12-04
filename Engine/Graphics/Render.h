#pragma once

#include <cstdint>
#include <cstdio>
#include <array>

#include "lib\legacy_dx\d3d.h"
#include "OSWindow.h"
#include "RenderStruct.h"

#include "../VectorTypes.h"

#include "IRender.h"

#pragma pack(push, 1)
struct DDM_DLV_Header
{
	//----- (00462607) --------------------------------------------------------
	inline DDM_DLV_Header()
	{
		this->uLastRepawnDay = 0;
		this->uNumRespawns = 0;
		this->uReputation = 0;
		this->field_C_alert = 0;
		this->uNumFacesInBModels = 0;
		this->uNumDecorations = 0;
		this->uNumBModels = 0;
	}


	int uNumRespawns;
	int uLastRepawnDay;
	int uReputation;
	int field_C_alert;
	unsigned int uNumFacesInBModels;
	unsigned int uNumDecorations;
	unsigned int uNumBModels;
	int field_1C;
	int field_20;
	int field_24;
};
#pragma pack(pop)


struct ODMFace;


/*  141 */
// stru148
#pragma pack(push, 1)
struct Polygon
{
	inline Polygon()
	{
		uNumVertices = 0;
	}

	int _479295();
	//void _48276F_sr();
	void _normalize_v_18();
	void Create_48607B(struct stru149 *a2);

	float field_0;
	float field_4;
	float field_8;
	float field_C;
	float field_10;
	unsigned int uNumVertices;
	Vec3_int_ v_18;
	int field_24;
	int sTextureDeltaU;
	int sTextureDeltaV;
	__int16 flags;
	__int16 field_32;
	int field_34;
	struct stru149 *ptr_38;
	struct Texture_MM7 *pTexture;
	struct Span *_unused_prolly_head;
	struct Span *_unused_prolly_tail;
	int **ptr_48;
	unsigned __int16 uTileBitmapID;
	__int16 field_4E;
	__int16 field_50;
	__int16 field_52;
	ODMFace *pODMFace;
	char dimming_level;
	char field_59;
	char field_5A;
	char field_5B;
	char terrain_grid_z;
	char terrain_grid_x;
	unsigned __int8 uBModelID;
	unsigned __int8 uBModelFaceID;
	struct Edge *pEdgeList1[20];
	unsigned int uEdgeList1Size;
	struct Edge *pEdgeList2[20];
	unsigned int uEdgeList2Size;
	char field_108;
	char field_109;
	char field_10A;
	char field_10B;
};
#pragma pack(pop)
extern std::array<struct Polygon, 2000 + 18000> array_77EC08;

/*  142 */
#pragma pack(push, 1)
struct stru149
{
	void _48616B_frustum_odm(int a2, int a3, int a4, int a5, int a6, int a7);
	void _48653D_frustum_blv(int a2, int a3, int a4, int a5, int a6, int a7);
	void _48694B_frustum_sky();

	int field_0_party_dir_x;
	int field_4_party_dir_y;
	int field_8_party_dir_z;
	int angle_from_north;//field_C
	int angle_from_west;//field_10
	int viewing_angle_from_west_east;
	int angle_from_east;//field_18
	int angle_from_south;//field_1C
	int viewing_angle_from_north_south;//field_20
	int field_24;
	int field_28;
};
#pragma pack(pop)
extern stru149 stru_8019C8;

/*   88 */
#pragma pack(push, 1)
struct ODMRenderParams
{
	//----- (00462684) --------------------------------------------------------
	ODMRenderParams()
	{
		uPickDepth = 0;
		this->shading_dist_shade = 2048;
		shading_dist_shademist = 4096;
		shading_dist_mist = 8192;
		int_fov_rad = 0;
		this->bNoSky = 0;
		this->bDoNotRenderDecorations = 0;
		this->field_5C = 0;
		this->field_60 = 0;
		this->outdoor_no_wavy_water = 0;
		this->outdoor_no_mist = 0;
	}

	void Initialize();

	int uPickDepth;
	int shading_dist_shade;
	int shading_dist_shademist;
	int shading_dist_mist;
	unsigned int uCameraFovInDegrees;
	int int_fov_rad;                          // 157 struct IndoorCamera::fov_rad
	int int_fov_rad_inv;                      // 157 struct IndoorCamera::fov_rad_inv
	int _unused_camera_rotation_y_int_sine;   // merged with BLVRenderParams equivalents
	int _unused_camera_rotation_y_int_cosine; // into IndoorCameraD3D
	int _unused_camera_rotation_x_int_sine;   // --//--
	int _unused_camera_rotation_x_int_cosine; // --//--
	int uNumPolygons;
	unsigned int _unused_uNumEdges;
	unsigned int _unused_uNumSurfs;
	unsigned int _unused_uNumSpans;
	unsigned int uNumBillboards;
	float field_40;
	int field_44;
	int outdoor_grid_band_3;
	int field_4C;
	int field_50;
	unsigned int bNoSky;
	unsigned int bDoNotRenderDecorations;
	int field_5C;
	int field_60;
	int outdoor_no_wavy_water;
	int outdoor_no_mist;
	int building_gamme;
	int terrain_gamma;

	unsigned int uMapGridCellX; // moved from 157 struct IndoorCamera::0C
	unsigned int uMapGridCellZ; // moved from 157 struct IndoorCamera::10
};
#pragma pack(pop)
extern ODMRenderParams *pODMRenderParams;



struct Render: public IRender
{
  Render();
  virtual ~Render();

  static Render *Create() {return new Render;}

  virtual bool Initialize(OSWindow *window);

  virtual void ClearBlack();
  virtual void PresentBlackScreen();

  virtual void SaveWinnersCertificate(const char *a1);
  virtual void ClearTarget(unsigned int uColor);
  virtual void Present();

  virtual void _49FD3A_fullscreen();
  virtual bool InitializeFullscreen();

  virtual void CreateZBuffer();
  virtual void Release();

  virtual bool SwitchToWindow();
  virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, unsigned __int16 uColor);
  virtual void ClearZBuffer(int a2, int a3);
  virtual void SetRasterClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW);
  virtual bool LockSurface_DDraw4(IDirectDrawSurface4 *pSurface, DDSURFACEDESC2 *pDesc, unsigned int uLockFlags);
  virtual void GetTargetPixelFormat(DDPIXELFORMAT *pOut);
  virtual void LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow);
  virtual void UnlockBackBuffer();
  virtual void LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow);
  virtual void UnlockFrontBuffer();
  virtual void RestoreFrontBuffer();
  virtual void RestoreBackBuffer();
  virtual void BltToFront(RECT *pDstRect, IDirectDrawSurface *pSrcSurface, RECT *pSrcRect, unsigned int uBltFlags);
  virtual void BltBackToFontFast(int a2, int a3, RECT *pSrcRect);
  virtual void BeginSceneD3D();

  virtual unsigned int GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6);

  virtual void DrawPolygon(unsigned int uNumVertices, struct Polygon *a3, ODMFace *a4, IDirect3DTexture2 *pTexture);
  virtual void DrawTerrainPolygon(unsigned int uNumVertices, struct Polygon *a4, IDirect3DTexture2 *a5, bool transparent, bool clampAtTextureBorders);
  virtual void DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, IDirect3DTexture2 *pHwTex, struct Texture_MM7 *pTex, int uPackedID, unsigned int uColor, int a8);

  virtual void MakeParticleBillboardAndPush_BLV(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle);
  virtual void MakeParticleBillboardAndPush_ODM(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle);

  virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
  virtual void DrawBillboard_Indoor(RenderBillboardTransform_local0 *pSoftBillboard, Sprite *pSprite, int dimming_level);
  virtual void _4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse);
  virtual void TransformBillboardsAndSetPalettesODM();
  virtual void DrawBillboardList_BLV();

  virtual void DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, IDirect3DTexture2 *a9);
  virtual bool LoadTexture(const char *pName, unsigned int bMipMaps, IDirectDrawSurface4 **pOutSurface, IDirect3DTexture2 **pOutTexture);
  virtual bool MoveSpriteToDevice(Sprite *pSprite);

  virtual void BeginScene();
  virtual void EndScene();
  virtual void ScreenFade(unsigned int color, float t);

  virtual void SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW);
  virtual void ResetUIClipRect();
  virtual void CreditsTextureScroll(unsigned int pX, unsigned int pY, int move_X, int move_Y, struct RGBTexture *pTexture);

  virtual void DrawTextureNew(float u, float v, struct Image *);
  virtual void DrawTextureAlphaNew(float u, float v, struct Image *);
  virtual void DrawTextureCustomHeight(float u, float v, class Image *img, int height);
  //virtual void DrawTextureNew(float u, float v, struct Texture_MM7 *);
  //virtual void DrawTextureTransparentColorKey(signed int x, signed int y, struct Texture_MM7 *tex);
  //virtual void DrawTextureIndexedAlpha(unsigned int uX, unsigned int uY, struct Texture_MM7 *pTexture);

  virtual void ZBuffer_Fill_2(signed int a2, signed int a3, struct Image *pTexture, int a5);
  virtual void ZDrawTextureAlpha(float u, float v, struct Image *pTexture, int zVal);
  virtual void BlendTextures(unsigned int a2, unsigned int a3, struct Image *a4, struct Texture_MM7 *a5, int t, int start_opacity, int end_opacity);
  virtual void _4A65CC(unsigned int x, unsigned int y, struct Texture_MM7 *a4, struct Texture_MM7 *a5, int a6, int a7, int a8);

  virtual void DrawMasked(float u, float v, struct Image *img, unsigned int color_dimming_level, unsigned __int16 mask);
  virtual void DrawTextureGrayShade(float u, float v, struct Image *a4);
  virtual void DrawTransparentRedShade(float u, float v, struct Image *a4);
  virtual void DrawTransparentGreenShade(float u, float v, struct Image *pTexture);
  virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices);

  virtual void DrawTextAlpha(int x, int y, unsigned char* font_pixels, int a5, unsigned int uFontHeight, unsigned __int16 *pPalette, bool present_time_transparency);
  virtual void DrawText(signed int uOutX, signed int uOutY, unsigned __int8 *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned __int16 *pFontPalette, unsigned __int16 uFaceColor, unsigned __int16 uShadowColor);

  virtual void FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, unsigned int uColor16);
  virtual void _4A6DF5(unsigned __int16 *pBitmap, unsigned int uBitmapPitch, struct Vec2_int_ *pBitmapXY, void *pTarget, unsigned int uTargetPitch, Vec4_int_ *a7);

  virtual void DrawBuildingsD3D();
  //struct BSPModel *DrawBuildingsSW();
  //int OnOutdoorRedrawSW();

  virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID);
  virtual void DrawOutdoorSkyD3D();
  //int DrawSkySW(struct Span *a1, Polygon *a2, int a3);
  virtual void DrawOutdoorSkyPolygon(unsigned int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture);
  virtual void DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture);

  virtual void PrepareDecorationsRenderList_ODM();
  virtual void DrawSpriteObjects_ODM();

  //float DrawBezierTerrain();
  virtual void RenderTerrainD3D();


  //void DrawTerrainSW(int a1, int a2, int a3, int a4);

  //void ExecOutdoorDrawSW();
  virtual bool AreRenderSurfacesOk();
  virtual bool IsGammaSupported();

  virtual void SaveScreenshot(const char *pFilename, unsigned int width, unsigned int height);
  virtual void PackScreenshot(unsigned int width, unsigned int height, void *out_data, unsigned int data_size, unsigned int *screenshot_size);
  virtual void SavePCXScreenshot();

  virtual int _46ภ6ภั_GetActorsInViewport(int pDepth);

  virtual void BeginLightmaps();
  virtual void EndLightmaps();  
  virtual void BeginLightmaps2();
  virtual void EndLightmaps2();
  virtual bool DrawLightmap(struct Lightmap *pLightmap, struct Vec3_float_ *pColorMult, float z_bias);

  virtual void BeginDecals();
  virtual void EndDecals();
  virtual void DrawDecal(struct Decal *pDecal, float z_bias);
  
  virtual void do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin, signed int sDiffuseBegin, const RenderVertexD3D3 *pLineEnd, signed int sDiffuseEnd, float z_stuff);
  virtual void DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices);

  virtual void DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, IDirect3DTexture2 *texture);

  virtual void am_Blt_Copy(RECT *pSrcRect, POINT *pTargetXY, int a3);
  virtual void am_Blt_Chroma(RECT *pSrcRect, POINT *pTargetPoint, int a3, int blend_mode);

  public:/*
    int *pActiveZBuffer;
    IDirectDraw4 *pDirectDraw4;
    IDirectDrawSurface4 *pFrontBuffer4;
    IDirectDrawSurface4 *pBackBuffer4;
    void        *pTargetSurface;
    unsigned int uTargetSurfacePitch;
    unsigned int bUseColoredLights;
    unsigned int bTinting;
    unsigned int bUsingSpecular;
    uint32_t uFogColor;
    unsigned int pHDWaterBitmapIDs[7];
    int hd_water_current_frame;
    int hd_water_tile_id;
    void (*pBeforePresentFunction)();
    uint32_t bFogEnabled;
    RenderBillboardD3D pBillboardRenderListD3D[1000];
    unsigned int uNumBillboardsToDraw;*/

    virtual void WritePixel16(int x, int y, unsigned __int16 color)
    {
      if (ddpfPrimarySuface.dwRGBBitCount == 32)
      {
        auto p = (unsigned __int32 *)pTargetSurface + x + y * uTargetSurfacePitch;
        *p = Color32(color);
      }
      else if (ddpfPrimarySuface.dwRGBBitCount == 16)
      {
        auto p = (unsigned __int16 *)pTargetSurface + x + y * uTargetSurfacePitch;
        *p = color;
      }
      else __debugbreak();
    }

    virtual unsigned __int16 ReadPixel16(int x, int y)
    {
      if (ddpfPrimarySuface.dwRGBBitCount == 32)
      {
        auto p = (unsigned __int32 *)pTargetSurface + x + y * uTargetSurfacePitch;
        return Color16((*p >> 16) & 255, (*p >> 8) & 255, *p & 255);
      }
      else if (ddpfPrimarySuface.dwRGBBitCount == 16)
      {
        auto p = (unsigned __int16 *)pTargetSurface + x + y * uTargetSurfacePitch;
        return *p;
      }
      else __debugbreak();
    }

    virtual void ToggleTint()          {bTinting = !bTinting;}
    virtual void ToggleColoredLights() {bUseColoredLights = !bUseColoredLights;}

    virtual unsigned int GetRenderWidth() {return window->GetWidth();}
    virtual unsigned int GetRenderHeight() {return window->GetHeight();}

    virtual void Sub01()
    {
      if (pRenderD3D && !bWindowMode)
        _49FD3A_fullscreen();
    }

    friend void Present_NoColorKey();

  protected:
    unsigned int uDesiredDirect3DDevice;
    int raster_clip_x;
    int raster_clip_y;    // clipping rect for raster ops
    int raster_clip_z;    //    like RasterLine2D for (mini)map
    int raster_clip_w;
    int *pDefaultZBuffer;
    OSWindow *window;
    unsigned int bWindowMode;
    RenderD3D *pRenderD3D;
    DDPIXELFORMAT ddpfPrimarySuface;
    unsigned int uTargetRBits;
    unsigned int uTargetGBits;
    unsigned int uTargetBBits;
    unsigned int uTargetRMask;
    unsigned int uTargetGMask;
    unsigned int uTargetBMask;
    unsigned int uNumSceneBegins;
    unsigned __int32 *pTargetSurface_unaligned;
    unsigned int uClipY;
    unsigned int uClipX;
    unsigned int uClipW;
    unsigned int uClipZ;
    unsigned int bClip;
    unsigned int uNumD3DSceneBegins;
    int using_software_screen_buffer;
    RenderHWLContainer pD3DBitmaps;
    RenderHWLContainer pD3DSprites;
    unsigned int bRequiredTextureStagesAvailable;
    unsigned int uLevelOfDetail;
    unsigned int uMaxDeviceTextureDim;
    unsigned int uMinDeviceTextureDim;

    void DoRenderBillboards_D3D();
    void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);
    void TransformBillboard(RenderBillboardTransform_local0 *a2, Sprite *pSprite, int dimming_level, RenderBillboard *pBillboard);
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    unsigned int GetParentBillboardID(unsigned int uBillboardID);
    unsigned int GetBillboardDrawListSize();

    void DrawBorderTiles(struct Polygon *poly);

    unsigned short *MakeScreenshot(signed int width, signed int height);
    bool CheckTextureStages();
    void ParseTargetPixelFormat();

    void CreateDirectDraw();
    void SetDirectDrawCooperationMode(HWND hWnd, bool bFullscreen);
    void SetDirectDrawDisplayMode(unsigned int uWidth, unsigned int uHeight, unsigned int uBPP);
    void CreateFrontBuffer();
    void CreateBackBuffer();
    void CreateDirectDrawPrimarySurface();
    void CreateClipper(HWND a2);

    void PackPCXpicture(unsigned short* picture_data, int wight, int heidth, void *data_buff, int max_buff_size,unsigned int* packed_size);
    void SavePCXImage(const char *Filename, unsigned short* picture_data, int width, int height);

  //int windowed_mode_width;
  //int windowed_mode_height;
};

unsigned int __fastcall _452442_color_cvt(unsigned __int16 a1, unsigned __int16 a2, int a3, int a4);

int __fastcall GetActorTintColor(int max_dim, int min_dim, float distance, int a4, struct RenderBillboard *a5);
int __fastcall _43F55F_get_billboard_light_level(struct RenderBillboard *a1, int uBaseLightLevel);
int __fastcall _43F5C8_get_point_light_level_with_respect_to_lights(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z);
unsigned int __fastcall GetMaxMipLevels(unsigned int uDim);
int _46E44E_collide_against_faces_and_portals(unsigned int b1); // idb
int __fastcall _46E889_collide_against_bmodels(unsigned int ecx0);
int collide_against_floor(int x, int y, int z, unsigned int *pSectorID, unsigned int *pFaceID); // idb
void __fastcall _46ED8A_collide_against_sprite_objects(unsigned int _this);
int _46EF01_collision_chech_player(int a1); // idb
void _46E0B2_collide_against_decorations();
int _46F04E_collide_against_portals();
unsigned int __fastcall sub_46DEF2(signed int a2, unsigned int uLayingItemID);
void UpdateObjects();
bool sub_47531C(int a1, int *a2, int pos_x, int pos_y, int pos_z, int dir_x, int dir_y, int dir_z, struct BLVFace *face, int a10);
bool sub_4754BF(int a1, int *a2, int X, int Y, int Z, int dir_x, int dir_y, int dir_z, struct BLVFace *face, int a10, int a11);
int sub_475665(struct BLVFace *face, int a2, __int16 a3);
bool __fastcall sub_4759C9(struct BLVFace *face, int a2, int a3, __int16 a4);
bool __fastcall sub_475D85(Vec3_int_ *a1, Vec3_int_ *a2, int *a3, struct BLVFace *a4);
bool __fastcall sub_475F30(int *a1, struct BLVFace *a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9);

bool __fastcall IsBModelVisible(unsigned int uModelID, int *unused);
