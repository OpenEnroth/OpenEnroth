#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "Application/GameConfig.h"

#include "Engine/Graphics/Nuklear.h"
#include "Engine/MM7.h"

#include "Library/Image/Image.h"
#include "Library/Color/Color.h"
#include "Library/Color/ColorTable.h"

#include "Utility/Geometry/Rect.h"

#include "TextureRenderId.h"

class Actor;
class GraphicsImage;
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
    Color sTintColor;
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
    Color diffuse;
    Color specular;
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

    GraphicsImage *texture;
    unsigned int uNumVertices;
    std::array<RenderVertexD3D3, 4> pQuads;
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
    Color sTintColor;
    unsigned short object_pid;
    int paletteID;
};

struct nk_tex_font {
    uint32_t texid;
    struct nk_font *font;
};

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
        uFogColor = Color();
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
    virtual struct nk_image NuklearImageLoad(GraphicsImage *img) = 0;
    virtual void NuklearImageFree(GraphicsImage *img) = 0;

    virtual GraphicsImage *CreateTexture_Paletted(const std::string &name) = 0;
    virtual GraphicsImage *CreateTexture_ColorKey(const std::string &name, Color colorkey) = 0;
    virtual GraphicsImage *CreateTexture_Solid(const std::string &name) = 0;
    virtual GraphicsImage *CreateTexture_Alpha(const std::string &name) = 0;

    virtual GraphicsImage *CreateTexture_PCXFromFile(const std::string &name) = 0;
    virtual GraphicsImage *CreateTexture_PCXFromIconsLOD(const std::string &name) = 0;
    virtual GraphicsImage *CreateTexture_PCXFromNewLOD(const std::string &name) = 0;
    virtual GraphicsImage *CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) = 0;

    virtual GraphicsImage *CreateTexture_Blank(unsigned int width, unsigned int height) = 0;
    virtual GraphicsImage *CreateTexture_Blank(RgbaImage image) = 0;

    virtual GraphicsImage *CreateTexture(const std::string &name) = 0;
    virtual GraphicsImage *CreateSprite(const std::string &name, unsigned int palette_id) = 0;

    virtual void ClearBlack() = 0;
    virtual void PresentBlackScreen() = 0;

    virtual RgbaImage ReadScreenPixels() = 0;
    virtual void SaveWinnersCertificate(const std::string &filePath) = 0;
    virtual void ClearTarget(Color uColor) = 0;
    virtual void Present() = 0;

    virtual bool InitializeFullscreen() = 0;

    virtual void CreateZBuffer() = 0;
    virtual void Release() = 0;

    virtual bool SwitchToWindow() = 0;

    virtual void BeginLines2D() = 0;
    virtual void EndLines2D() = 0;
    virtual void RasterLine2D(int uX, int uY, int uZ, int uW, Color uColor32) = 0;
    virtual void DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void ClearZBuffer() = 0;
    virtual void RestoreFrontBuffer() = 0;
    virtual void RestoreBackBuffer() = 0;
    virtual void BltBackToFontFast(int a2, int a3, Recti *a4) = 0;
    virtual void BeginScene3D() = 0;

    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) = 0;

    virtual void MakeParticleBillboardAndPush(SoftwareBillboard *a2,
                                              GraphicsImage *texture,
                                              Color uDiffuse,
                                              int angle) = 0;
    virtual float GetGamma() = 0;

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() = 0;
    virtual void BillboardSphereSpellFX(struct SpellFX_Billboard *a1, Color diffuse) = 0;
    virtual void TransformBillboardsAndSetPalettesODM() = 0;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                GraphicsImage *texture) = 0;

    virtual TextureRenderId CreateTexture(RgbaImageView image) = 0;
    virtual void DeleteTexture(TextureRenderId id) = 0;
    virtual void UpdateTexture(TextureRenderId id, RgbaImageView image) = 0;

    virtual void Update_Texture(GraphicsImage *texture) = 0;


    virtual void BeginScene2D() = 0;
    virtual void ScreenFade(Color color, float t) = 0;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW) = 0;
    virtual void ResetUIClipRect() = 0;

    virtual void DrawTextureNew(float u, float v, GraphicsImage *img, Color colourmask32 = colorTable.White) = 0;
    virtual void DrawTextureCustomHeight(float u, float v, GraphicsImage *, int height) = 0;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y, GraphicsImage *) = 0;
    virtual void DrawImage(GraphicsImage *, const Recti &rect, const uint paletteid = 0, Color colourmask32 = colorTable.White) = 0;

    virtual void ZDrawTextureAlpha(float u, float v, GraphicsImage *pTexture, int zVal) = 0;
    virtual void BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t, int start_opacity, int end_opacity) = 0;
    virtual void TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) = 0;
    virtual void DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait_Sprite, int Y_Offset) = 0;

    virtual void DrawMasked(float u, float v, GraphicsImage *img,
                            unsigned int color_dimming_level,
                            Color mask = colorTable.White) = 0;
    virtual void DrawTextureGrayShade(float u, float v, GraphicsImage *a4) = 0;
    virtual void DrawTransparentRedShade(float u, float v, GraphicsImage *a4) = 0;
    virtual void DrawTransparentGreenShade(float u, float v, GraphicsImage *pTexture) = 0;
    // virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices) = 0;

    virtual void BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) = 0;
    virtual void EndTextNew() = 0;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) = 0;

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              Color uColor32) = 0;

    virtual void DrawOutdoorBuildings() = 0;

    virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID = 0) = 0;
    virtual void DrawOutdoorSky() = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects() = 0;

    virtual void DrawOutdoorTerrain() = 0;

    virtual bool AreRenderSurfacesOk() = 0;

    virtual GraphicsImage *TakeScreenshot(unsigned int width, unsigned int height) = 0;
    virtual void SaveScreenshot(const std::string &filename, unsigned int width,
                                unsigned int height) = 0;
    virtual Blob PackScreenshot(const unsigned int width, const unsigned int height) = 0;
    virtual void SavePCXScreenshot() = 0;
    virtual RgbaImage MakeScreenshot32(const int width, const int height) = 0;

    virtual std::vector<Actor *> getActorsInViewport(int pDepth) = 0;

    virtual void BeginLightmaps() = 0;
    virtual void EndLightmaps() = 0;
    virtual void BeginLightmaps2() = 0;
    virtual void EndLightmaps2() = 0;
    virtual bool DrawLightmap(struct Lightmap *pLightmap,
                              Vec3f *pColorMult, float z_bias) = 0;

    virtual void BeginDecals() = 0;
    virtual void EndDecals() = 0;
    virtual void DrawDecal(struct Decal *pDecal, float z_bias) = 0;

    virtual void DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) = 0;

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
    Color uFogColor;
    unsigned int pHDWaterBitmapIDs[7];
    int hd_water_current_frame;
    GraphicsImage *hd_water_tile_anim[7];
    RenderBillboardD3D pBillboardRenderListD3D[1000];
    unsigned int uNumBillboardsToDraw; // TODO(captainurist): this is not properly cleared if BeginScene3D is not called,
                                       //                     resulting in dangling textures in pBillboardRenderListD3D.

    int drawcalls;

    Logger *log = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    LightmapBuilder *lightmap_builder = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
    Vis *vis = nullptr;
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

Color GetActorTintColor(int max_dim, int min_dim, float distance, int a4, const RenderBillboard *a5);
int _43F55F_get_billboard_light_level(const RenderBillboard *a1, int uBaseLightLevel);
int GetLightLevelAtPoint(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z);

void UpdateObjects();
