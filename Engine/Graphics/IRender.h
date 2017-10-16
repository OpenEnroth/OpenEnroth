#pragma once
#include "Engine/Rect.h"
#include "Engine/VectorTypes.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Image.h"

class OSWindow;

class Sprite;
class SpriteFrame;
struct SoftwareBillboard;


bool PauseGameDrawing();

/*  161 */
#pragma pack(push, 1)
struct RenderBillboard
{
    fixed screenspace_projection_factor_x;
    fixed screenspace_projection_factor_y;
    float fov_x;
    float fov_y;
    int field_14_actor_id;
    Sprite *hwsprite;//signed __int16 HwSpriteID;
    __int16 uPalette;
    __int16 uIndoorSectorID;
    __int16 field_1E;
    __int16 world_x;
    __int16 world_y;
    __int16 world_z;
    __int16 screen_space_x;
    __int16 screen_space_y;
    __int16 screen_space_z;
    unsigned __int16 object_pid;
    unsigned __int16 dimming_level;
    unsigned int sTintColor;
    SpriteFrame *pSpriteFrame;
};
#pragma pack(pop)




unsigned __int16 Color16(unsigned __int32 r, unsigned __int32 g, unsigned __int32 b);
unsigned __int32 Color32(unsigned __int16 color16);
unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);





/*   88 */
#pragma pack(push, 1)
struct ODMRenderParams
{
    //----- (00462684) --------------------------------------------------------
    ODMRenderParams()
    {
        uPickDepth = 0;
        this->shading_dist_shade = 0x800;
        shading_dist_shademist = 0x1000;
        this->near_clip = 4;
        this->far_clip = 2 * 0x2000;
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
    int near_clip;
    int far_clip;         // far clip (shading_dist_mist in M&M6 terms)
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


/*  119 */
#pragma pack(push, 1)
struct RenderVertexSoft
{
    inline RenderVertexSoft() :
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


/*  242 */
#pragma pack(push, 1)
struct RenderBillboardD3D
{
    inline RenderBillboardD3D() :
        opacity(Transparent),
        field_90(-1),
        sParentBillboardID(-1),
        uNumVertices(4)
    {}

    enum OpacityType : unsigned __int32
    {
        Transparent = 0,
        Opaque_1 = 1,
        Opaque_2 = 2,
        Opaque_3 = 3,
        NoBlend = 0xFFFFFFFF
    };

    Texture *texture;//void *gapi_texture;//IDirect3DTexture2 *pTexture; for d3d
    unsigned int uNumVertices;
    RenderVertexD3D3 pQuads[4];
    float z_order;
    OpacityType opacity;
    int field_90;

    unsigned short object_pid;
    short screen_space_z;
    signed int sParentBillboardID;
};
#pragma pack(pop)




/*  248 */
#pragma pack(push, 1)
struct SoftwareBillboard
{
    void *pTarget;
    int *pTargetZ;
    int screen_space_x;
    int screen_space_y;
    short screen_space_z;
    fixed screenspace_projection_factor_x;
    fixed screenspace_projection_factor_y;
    char field_18[8];
    unsigned __int16 *pPalette;
    unsigned __int16 *pPalette2;
    unsigned int uFlags;        // & 4   - mirror horizontally
    unsigned int uTargetPitch;
    unsigned int uViewportX;
    unsigned int uViewportY;
    unsigned int uViewportZ;
    unsigned int uViewportW;
    int field_44;
    int sParentBillboardID;
    int sTintColor;
    unsigned short object_pid;
};
#pragma pack(pop)



/*  280 */
#pragma pack(push, 1)
struct HWLTexture
{
    inline HWLTexture() :
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






class IRender
{
  public:
    virtual ~IRender() {}

    static IRender *Create();

    virtual bool Initialize(OSWindow *window) = 0;

	virtual Texture *CreateTexture(const String &name) = 0;
    virtual Texture *CreateSprite(const String &name, unsigned int palette_id, /*refactor*/unsigned int lod_sprite_id) = 0;

    virtual void ClearBlack() = 0;
    virtual void PresentBlackScreen() = 0;

    virtual void SaveWinnersCertificate(const char *a1) = 0;
    virtual void ClearTarget(unsigned int uColor) = 0;
    virtual void Present() = 0;

    virtual void _49FD3A_fullscreen() = 0;
    virtual bool InitializeFullscreen() = 0;

    virtual void CreateZBuffer() = 0;
    virtual void Release() = 0;

    virtual bool SwitchToWindow() = 0;
    virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, unsigned __int16 uColor) = 0;
    virtual void ClearZBuffer(int a2, int a3) = 0;
    virtual void SetRasterClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW) = 0;
    virtual bool LockSurface(Texture *surface, Rect *, void **out_surface, int *out_pitch, int *out_width, int *out_height) = 0;
    virtual void UnlockSurface(Texture *surface) = 0;
    virtual void LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow) = 0;
    virtual void UnlockBackBuffer() = 0;
    virtual void LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow) = 0;
    virtual void UnlockFrontBuffer() = 0;
    virtual void RestoreFrontBuffer() = 0;
    virtual void RestoreBackBuffer() = 0;
    virtual void BltBackToFontFast(int a2, int a3, Rect *a4) = 0;
    virtual void BeginSceneD3D() = 0;

    virtual unsigned int GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6) = 0;

    virtual void DrawPolygon(struct Polygon *a3) = 0;
    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent, bool clampAtTextureBorders) = 0;
    virtual void DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, int uPackedID, unsigned int uColor, int a8) = 0;

    virtual void MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle) = 0;
    virtual void MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle) = 0;

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() = 0;
    virtual void DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard, RenderBillboard *billboard) = 0;
    virtual void _4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse) = 0;
    virtual void TransformBillboardsAndSetPalettesODM() = 0;
    virtual void DrawBillboardList_BLV() = 0;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, Texture *texture) = 0;
    virtual bool MoveTextureToDevice(Texture *texture) = 0;

    virtual void BeginScene() = 0;
    virtual void EndScene() = 0;
    virtual void ScreenFade(unsigned int color, float t) = 0;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW) = 0;
    virtual void ResetUIClipRect() = 0;

    virtual void DrawTextureNew(float u, float v, Image *) = 0;
    virtual void DrawTextureAlphaNew(float u, float v, Image *) = 0;
    virtual void DrawTextureCustomHeight(float u, float v, Image *, int height) = 0;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y, Image *) = 0;

    virtual void ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture, int a5) = 0;
    virtual void ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal) = 0;
    virtual void BlendTextures(int a2, int a3, Image *a4, Image *a5, int t, int start_opacity, int end_opacity) = 0;
    virtual void _4A65CC(unsigned int x, unsigned int y, Image *a4, Image *a5, int a6, int a7, int a8) = 0;

    virtual void DrawMasked(float u, float v, Image *img, unsigned int color_dimming_level, unsigned __int16 mask) = 0;
    virtual void DrawTextureGrayShade(float u, float v, Image *a4) = 0;
    virtual void DrawTransparentRedShade(float u, float v, Image *a4) = 0;
    virtual void DrawTransparentGreenShade(float u, float v, Image *pTexture) = 0;
    virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void DrawTextAlpha(int x, int y, unsigned char* font_pixels, int a5, unsigned int uFontHeight, unsigned __int16 *pPalette, bool present_time_transparency) = 0;
    virtual void DrawText(signed int uOutX, signed int uOutY, unsigned __int8 *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned __int16 *pFontPalette, unsigned __int16 uFaceColor, unsigned __int16 uShadowColor) = 0;

    virtual void FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, unsigned int uColor16) = 0;
    virtual void _4A6DF5(unsigned __int16 *pBitmap, unsigned int uBitmapPitch, struct Vec2_int_ *pBitmapXY, void *pTarget, unsigned int uTargetPitch, Vec4_int_ *a7) = 0;

    virtual void DrawBuildingsD3D() = 0;

    virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID = 0) = 0;
    virtual void DrawOutdoorSkyD3D() = 0;
    virtual void DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) = 0;
    virtual void DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon) = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects_ODM() = 0;

    virtual void RenderTerrainD3D() = 0;

    virtual bool AreRenderSurfacesOk() = 0;

    virtual void SaveScreenshot(const String &filename, unsigned int width, unsigned int height) = 0;
    virtual void PackScreenshot(unsigned int width, unsigned int height, void *out_data, unsigned int data_size, unsigned int *screenshot_size) = 0;
    virtual void SavePCXScreenshot() = 0;

    virtual int _46А6АС_GetActorsInViewport(int pDepth) = 0;

    virtual void BeginLightmaps() = 0;
    virtual void EndLightmaps() = 0;
    virtual void BeginLightmaps2() = 0;
    virtual void EndLightmaps2() = 0;
    virtual bool DrawLightmap(struct Lightmap *pLightmap, struct Vec3_float_ *pColorMult, float z_bias) = 0;

    virtual void BeginDecals() = 0;
    virtual void EndDecals() = 0;
    virtual void DrawDecal(struct Decal *pDecal, float z_bias) = 0;
  
    virtual void do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin, signed int sDiffuseBegin, const RenderVertexD3D3 *pLineEnd, signed int sDiffuseEnd, float z_stuff) = 0;
    virtual void DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, Texture *texture) = 0;

    virtual void am_Blt_Copy(struct Rect *pSrcRect, struct Point *pTargetXY, int a3) = 0;
    virtual void am_Blt_Chroma(struct Rect *pSrcRect, struct Point *pTargetPoint, int a3, int blend_mode) = 0;








    inline IRender()
    {
        pActiveZBuffer = 0;
        //pDirectDraw4 = 0;
        //pFrontBuffer4 = 0;
        //pBackBuffer4 = 0;
        pTargetSurface = 0;
        uTargetSurfacePitch = 0;
        bUseColoredLights = 0;
        bTinting = 0;
        bUsingSpecular = 0;
        uFogColor = 0;
        memset(pHDWaterBitmapIDs, 0, sizeof(pHDWaterBitmapIDs));
        hd_water_current_frame = 0;
        hd_water_tile_id = 0;
        pBeforePresentFunction = 0;
        bFogEnabled;
        memset(pBillboardRenderListD3D, 0, sizeof(pBillboardRenderListD3D));
        uNumBillboardsToDraw = 0;
    }

    int *pActiveZBuffer;
    void        *pTargetSurface;
    unsigned int uTargetSurfacePitch;
    unsigned int bUseColoredLights;
    unsigned int bTinting;
    unsigned int bUsingSpecular;
    uint32_t uFogColor;
    unsigned int pHDWaterBitmapIDs[7];
    int hd_water_current_frame;
    int hd_water_tile_id;
    Texture *hd_water_tile_anim[7];
    void (*pBeforePresentFunction)();
    uint32_t bFogEnabled;
    RenderBillboardD3D pBillboardRenderListD3D[1000];
    unsigned int uNumBillboardsToDraw;

    virtual void WritePixel16(int x, int y, unsigned __int16 color) = 0;
    virtual unsigned __int16 ReadPixel16(int x, int y) = 0;

    virtual void ToggleTint() = 0;
    virtual void ToggleColoredLights() = 0;

    virtual unsigned int GetRenderWidth() const = 0;
    virtual unsigned int GetRenderHeight() const = 0;

    virtual void Sub01() = 0;

    virtual HWLTexture *LoadHwlBitmap(const char *name) = 0;
    virtual HWLTexture *LoadHwlSprite(const char *name) = 0;
};

extern IRender *render;


extern int uNumDecorationsDrawnThisFrame;
extern RenderBillboard pBillboardRenderList[500];
extern unsigned int uNumBillboardsToDraw;
extern int uNumSpritesDrawnThisFrame;

extern RenderVertexSoft array_507D30[50];
extern RenderVertexSoft VertexRenderList[50];
extern RenderVertexSoft array_73D150[20];

extern RenderVertexD3D3 d3d_vertex_buffer[50];


int ODM_NearClip(unsigned int uVertexID);
int ODM_FarClip(unsigned int uNumVertices);



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




unsigned int _452442_color_cvt(unsigned __int16 a1, unsigned __int16 a2, int a3, int a4);

int GetActorTintColor(int max_dim, int min_dim, float distance, int a4, struct RenderBillboard *a5);
int _43F55F_get_billboard_light_level(struct RenderBillboard *a1, int uBaseLightLevel);
int _43F5C8_get_point_light_level_with_respect_to_lights(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z);
unsigned int GetMaxMipLevels(unsigned int uDim);
int _46E44E_collide_against_faces_and_portals(unsigned int b1); // idb
int _46E889_collide_against_bmodels(unsigned int ecx0);
int collide_against_floor(int x, int y, int z, unsigned int *pSectorID, unsigned int *pFaceID); // idb
void _46ED8A_collide_against_sprite_objects(unsigned int _this);
int _46EF01_collision_chech_player(int a1); // idb
void _46E0B2_collide_against_decorations();
int _46F04E_collide_against_portals();
unsigned int sub_46DEF2(signed int a2, unsigned int uLayingItemID);
void UpdateObjects();
bool sub_47531C(int a1, int *a2, int pos_x, int pos_y, int pos_z, int dir_x, int dir_y, int dir_z, struct BLVFace *face, int a10);
bool sub_4754BF(int a1, int *a2, int X, int Y, int Z, int dir_x, int dir_y, int dir_z, struct BLVFace *face, int a10, int a11);
int sub_475665(struct BLVFace *face, int a2, __int16 a3);
bool sub_4759C9(struct BLVFace *face, int a2, int a3, __int16 a4);
bool sub_475D85(Vec3_int_ *a1, Vec3_int_ *a2, int *a3, struct BLVFace *a4);
bool sub_475F30(int *a1, struct BLVFace *a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9);

bool IsBModelVisible(unsigned int uModelID, int *unused);