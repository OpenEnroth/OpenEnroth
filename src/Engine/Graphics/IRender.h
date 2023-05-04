#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/OurMath.h"
#include "Application/GameConfig.h"
#include "Utility/Color.h"
#include "Utility/Geometry/Rect.h"

class Sprite;
class SpriteFrame;
struct SoftwareBillboard;
struct DecalBuilder;
class LightmapBuilder;
class ParticleEngine;
struct SpellFxRenderer;
class Vis;
class Logger;

namespace LOD {
class File;
}

bool PauseGameDrawing();

struct RenderBillboard {
    float screenspace_projection_factor_x;
    float screenspace_projection_factor_y;
    float fov_x;
    float fov_y;
    int field_14_actor_id;
    Sprite *hwsprite;  // int16_t HwSpriteID;
    int16_t uPaletteIndex;
    int16_t uIndoorSectorID;
    int16_t field_1E;  // flags
    int16_t world_x;
    int16_t world_y;
    int16_t world_z;
    int16_t screen_space_x;
    int16_t screen_space_y;
    int16_t screen_space_z;
    uint16_t object_pid;
    uint16_t dimming_level;
    unsigned int sTintColor;
    SpriteFrame *pSpriteFrame;
};

/*   88 */
struct ODMRenderParams {
    ODMRenderParams() {
        this->shading_dist_shade = 0x800;
        shading_dist_shademist = 0x1000;
        this->bNoSky = 0;
        this->bDoNotRenderDecorations = 0;
        this->field_5C = 0;
        this->field_60 = 0;
        this->outdoor_no_wavy_water = 0;
        this->outdoor_no_mist = 0;
    }

    int shading_dist_shade;
    int shading_dist_shademist;
    int uNumPolygons = 0;
    unsigned int _unused_uNumEdges = 0;
    unsigned int _unused_uNumSurfs = 0;
    unsigned int _unused_uNumSpans = 0;
    unsigned int uNumBillboards = 0;
    float field_40 = 0;
    int outdoor_grid_band_3 = 0; // TODO: drop?
    unsigned int bNoSky;
    unsigned int bDoNotRenderDecorations;
    int field_5C;
    int field_60;
    int outdoor_no_wavy_water;
    int outdoor_no_mist;
    int building_gamme = 0;
    int terrain_gamma = 0;

    unsigned int uMapGridCellX = 0;  // moved from 157 struct IndoorCamera::0C
    unsigned int uMapGridCellY = 0;  // moved from 157 struct IndoorCamera::10
};
extern ODMRenderParams *pODMRenderParams;

struct RenderVertexSoft {
    Vec3f vWorldPosition {};
    Vec3f vWorldViewPosition {};
    float vWorldViewProjX = 0;
    float vWorldViewProjY = 0;
    float _rhw = 0;
    float u = 0;
    float v = 0;
    float flt_2C = 0;
};

struct RenderVertexD3D3 {
    Vec3f pos {};
    float rhw = 0;
    unsigned int diffuse = 0;
    unsigned int specular = 0;
    Vec2f texcoord {};
};

struct RenderBillboardD3D {
    inline RenderBillboardD3D()
        : texture(nullptr),
          uNumVertices(4),
          z_order(0.f),
          opacity(Transparent),
          field_90(-1),
          object_pid(0),
          screen_space_z(0),
          sParentBillboardID(-1),
          PaletteIndex(0) {}

    enum class OpacityType : uint32_t {
        Transparent = 0,
        Opaque_1 = 1,
        Opaque_2 = 2,
        Opaque_3 = 3,
        NoBlend = 0xFFFFFFFF
    };
    using enum OpacityType;

    Texture *texture;  // void *gapi_texture;//IDirect3DTexture2 *pTexture; for d3d
    unsigned int uNumVertices;
    RenderVertexD3D3 pQuads[4];
    float z_order;
    OpacityType opacity;
    int field_90;

    unsigned short object_pid;
    short screen_space_z;
    int sParentBillboardID;

    //int PaletteID;
    int PaletteIndex;
};

// TODO(pskelton): Simplify/remove/combine different billboard structs
struct SoftwareBillboard {
    void *pTarget;
    int *pTargetZ;
    int screen_space_x;
    int screen_space_y;
    short screen_space_z;
    float screenspace_projection_factor_x;
    float screenspace_projection_factor_y;
    char field_18[8];
    uint16_t *pPalette;
    uint16_t *pPalette2;
    unsigned int uFlags;  // & 4   - mirror horizontally
    unsigned int uTargetPitch;
    unsigned int uViewportX;
    unsigned int uViewportY;
    unsigned int uViewportZ;
    unsigned int uViewportW;
    int field_44;
    int sParentBillboardID;
    int sTintColor;
    unsigned short object_pid;
    int paletteID;
};

struct nk_tex_font {
    uint32_t texid;
    struct nk_font *font;
};

class HWLTexture;

class IRender {
 public:
    inline IRender(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        LightmapBuilder *lightmap_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis,
        Logger *logger
    ) {
        this->config = config;
        this->decal_builder = decal_builder;
        this->lightmap_builder = lightmap_builder;
        this->spell_fx_renderer = spellfx;
        this->particle_engine = particle_engine;
        this->vis = vis;
        this->log = logger;

        pActiveZBuffer = 0;
        uFogColor = 0;
        memset(pHDWaterBitmapIDs, 0, sizeof(pHDWaterBitmapIDs));
        hd_water_current_frame = 0;
        memset(pBillboardRenderListD3D, 0, sizeof(pBillboardRenderListD3D));
        uNumBillboardsToDraw = 0;
        drawcalls = 0;
    }
    virtual ~IRender() {}

    virtual bool Initialize() = 0;

    virtual bool NuklearInitialize(struct nk_tex_font *tfont) = 0;
    virtual bool NuklearCreateDevice() = 0;
    virtual bool NuklearRender(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer) = 0;
    virtual void NuklearRelease() = 0;
    virtual struct nk_tex_font *NuklearFontLoad(const char *font_path, size_t font_size) = 0;
    virtual void NuklearFontFree(struct nk_tex_font *tfont) = 0;
    virtual struct nk_image NuklearImageLoad(Image *img) = 0;
    virtual void NuklearImageFree(Image *img) = 0;

    virtual Texture *CreateTexture_Paletted(const std::string &name) = 0;
    virtual Texture *CreateTexture_ColorKey(const std::string &name, uint16_t colorkey) = 0;
    virtual Texture *CreateTexture_Solid(const std::string &name) = 0;
    virtual Texture *CreateTexture_Alpha(const std::string &name) = 0;

    virtual Texture *CreateTexture_PCXFromFile(const std::string &name) = 0;
    virtual Texture *CreateTexture_PCXFromIconsLOD(const std::string &name) = 0;
    virtual Texture *CreateTexture_PCXFromNewLOD(const std::string &name) = 0;
    virtual Texture *CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) = 0;

    virtual Texture *CreateTexture_Blank(unsigned int width, unsigned int height,
        IMAGE_FORMAT format, const void *pixels = nullptr) = 0;

    virtual Texture *CreateTexture(const std::string &name) = 0;
    virtual Texture *CreateSprite(const std::string &name, unsigned int palette_id,
                                  /*refactor*/ unsigned int lod_sprite_id) = 0;

    virtual void ClearBlack() = 0;
    virtual void PresentBlackScreen() = 0;

    virtual uint8_t *ReadScreenPixels() = 0;
    virtual void SaveWinnersCertificate(const std::string &filePath) = 0;
    virtual void ClearTarget(unsigned int uColor) = 0;
    virtual void Present() = 0;

    virtual bool InitializeFullscreen() = 0;

    virtual void CreateZBuffer() = 0;
    virtual void Release() = 0;

    virtual bool SwitchToWindow() = 0;

    virtual void BeginLines2D() = 0;
    virtual void EndLines2D() = 0;
    virtual void RasterLine2D(int uX, int uY, int uZ, int uW, uint32_t uColor32) = 0;
    virtual void DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void ClearZBuffer() = 0;
    virtual void RestoreFrontBuffer() = 0;
    virtual void RestoreBackBuffer() = 0;
    virtual void BltBackToFontFast(int a2, int a3, Recti *a4) = 0;
    virtual void BeginScene3D() = 0;

    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) = 0;

    virtual void MakeParticleBillboardAndPush(SoftwareBillboard *a2,
                                                  Texture *texture,
                                                  unsigned int uDiffuse,
                                                  int angle) = 0;
    virtual float GetGamma() = 0;

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() = 0;
    virtual void BillboardSphereSpellFX(struct SpellFX_Billboard *a1, int diffuse) = 0;
    virtual void TransformBillboardsAndSetPalettesODM() = 0;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                Texture *texture) = 0;
    virtual void RemoveTextureFromDevice(Texture *texture) = 0;
    virtual bool MoveTextureToDevice(Texture *texture) = 0;

    virtual void Update_Texture(Texture *texture) = 0;
    virtual void DeleteTexture(Texture *texture) = 0;


    virtual void BeginScene2D() = 0;
    virtual void ScreenFade(unsigned int color, float t) = 0;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW) = 0;
    virtual void ResetUIClipRect() = 0;

    virtual void DrawTextureNew(float u, float v, Image *img, uint32_t colourmask32 = 0xFFFFFFFF) = 0;
    virtual void DrawTextureCustomHeight(float u, float v, Image *, int height) = 0;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y, Image *) = 0;
    virtual void DrawImage(Image *, const Recti &rect, const uint paletteid = 0, uint32_t colourmask32 = 0xFFFFFFFF) = 0;

    virtual void ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal) = 0;
    virtual void BlendTextures(int a2, int a3, Image *a4, Image *a5, int t, int start_opacity, int end_opacity) = 0;
    virtual void TexturePixelRotateDraw(float u, float v, Image *img, int time) = 0;
    virtual void DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait_Sprite, int Y_Offset) = 0;

    virtual void DrawMasked(float u, float v, Image *img,
                            unsigned int color_dimming_level,
                            uint32_t mask = 0xFFFFFFFF) = 0;
    virtual void DrawTextureGrayShade(float u, float v, Image *a4) = 0;
    virtual void DrawTransparentRedShade(float u, float v, Image *a4) = 0;
    virtual void DrawTransparentGreenShade(float u, float v, Image *pTexture) = 0;
    // virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void BeginTextNew(Texture *main, Texture *shadow) = 0;
    virtual void EndTextNew() = 0;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, uint16_t colour) = 0;

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              uint32_t uColor32) = 0;

    virtual void DrawOutdoorBuildings() = 0;

    virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID = 0) = 0;
    virtual void DrawOutdoorSky() = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects() = 0;

    virtual void DrawOutdoorTerrain() = 0;

    virtual bool AreRenderSurfacesOk() = 0;

    virtual Image *TakeScreenshot(unsigned int width, unsigned int height) = 0;
    virtual void SaveScreenshot(const std::string &filename, unsigned int width,
                                unsigned int height) = 0;
    virtual Blob PackScreenshot(const unsigned int width, const unsigned int height) = 0;
    virtual void SavePCXScreenshot() = 0;
    virtual uint32_t *MakeScreenshot32(const int width, const int height) = 0;

    virtual std::vector<Actor*> getActorsInViewport(int pDepth) = 0;

    virtual void BeginLightmaps() = 0;
    virtual void EndLightmaps() = 0;
    virtual void BeginLightmaps2() = 0;
    virtual void EndLightmaps2() = 0;
    virtual bool DrawLightmap(struct Lightmap *pLightmap,
                              Vec3f *pColorMult, float z_bias) = 0;

    virtual void BeginDecals() = 0;
    virtual void EndDecals() = 0;
    virtual void DrawDecal(struct Decal *pDecal, float z_bias) = 0;

    virtual void DrawSpecialEffectsQuad(Texture *texture, int palette) = 0;

    virtual void DrawFromSpriteSheet(Recti *pSrcRect,
                               Pointi *pTargetPoint, int a3,
                               int blend_mode) = 0;

    virtual void DrawIndoorFaces() = 0;

    virtual void ReleaseTerrain() = 0;
    virtual void ReleaseBSP() = 0;

    virtual void DrawTwodVerts() = 0;

    virtual Sizei GetRenderDimensions() = 0;
    virtual Sizei GetPresentDimensions() = 0;
    virtual bool Reinitialize(bool firstInit = false) = 0;
    virtual void ReloadShaders() = 0;
    virtual void DoRenderBillboards_D3D() = 0;

    std::shared_ptr<GameConfig> config = nullptr;
    int *pActiveZBuffer;
    uint32_t uFogColor;
    unsigned int pHDWaterBitmapIDs[7];
    int hd_water_current_frame;
    Texture *hd_water_tile_anim[7];
    RenderBillboardD3D pBillboardRenderListD3D[1000];
    unsigned int uNumBillboardsToDraw;

    int drawcalls;

    Logger *log = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    LightmapBuilder *lightmap_builder = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
    Vis *vis = nullptr;

    virtual HWLTexture *LoadHwlBitmap(const std::string &name) = 0;
    virtual HWLTexture *LoadHwlSprite(const std::string &name) = 0;
};

extern std::shared_ptr<IRender> render;

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

struct SkyBillboardStruct {
    void CalcSkyFrustumVec(int a2, int a3, int a4, int a5, int a6, int a7);

    float field_0_party_dir_x;  // cam view transform
    float field_4_party_dir_y;
    float field_8_party_dir_z;
    float CamVecLeft_Y;
    float CamVecLeft_X;
    float CamVecLeft_Z;
    float CamVecFront_Y;
    float CamVecFront_X;
    float CamVecFront_Z;
    float CamLeftDot;
    float CamFrontDot;
};

extern SkyBillboardStruct SkyBillboard;

unsigned int _452442_color_cvt(uint16_t a1, uint16_t a2, int a3, int a4);

int GetActorTintColor(int max_dim, int min_dim, float distance, int a4,
                      struct RenderBillboard *a5);
int _43F55F_get_billboard_light_level(struct RenderBillboard *a1,
                                      int uBaseLightLevel);
int GetLightLevelAtPoint(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z);
unsigned int GetMaxMipLevels(unsigned int uDim);

void UpdateObjects();

class BSPModel;

inline uint32_t PixelDim(uint32_t pix, int dimming);
