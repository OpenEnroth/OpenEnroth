#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Library/Image/Image.h"
#include "Library/Color/Color.h"
#include "Library/Color/ColorTable.h"
#include "Library/Geometry/Rect.h"

#include "TextureRenderId.h"
#include "Engine/Graphics/RenderEntities.h"

class Actor;
class GraphicsImage;
class GameConfig;
class Sprite;
class SpriteFrame;
struct SoftwareBillboard;
struct DecalBuilder;
class ParticleEngine;
struct SpellFxRenderer;
struct SpellFX_Billboard;
class Vis;
struct Lightmap;
struct Decal;
struct Polygon;
struct nk_context;

bool PauseGameDrawing();

class Renderer {
 public:
    Renderer(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis
    );
    virtual ~Renderer();

    virtual bool Initialize() = 0;

    virtual void ClearBlack() = 0;
    virtual void PresentBlackScreen() = 0;

    virtual RgbaImage ReadScreenPixels() = 0;
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

    virtual void DrawTerrainPolygon(Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) = 0;

    virtual void MakeParticleBillboardAndPush(SoftwareBillboard *a2,
                                              GraphicsImage *texture,
                                              Color uDiffuse,
                                              int angle) = 0;
    virtual float GetGamma() = 0;

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() = 0;
    virtual void BillboardSphereSpellFX(SpellFX_Billboard *a1, Color diffuse) = 0;
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
    virtual void DrawImage(GraphicsImage *, const Recti &rect, const unsigned int paletteid = 0, Color colourmask32 = colorTable.White) = 0;

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

    virtual void DrawIndoorSky(unsigned int uNumVertices, int uFaceID = 0) = 0;
    virtual void DrawOutdoorSky() = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects() = 0;

    virtual void DrawOutdoorTerrain() = 0;

    virtual bool AreRenderSurfacesOk() = 0;

    /**
     * Takes a screenshot of the game viewport, w/o the UI elements.
     *
     * @param width                         Required width
     * @param height                        Required height.
     * @return                              Taken screenshot image.
     */
    virtual RgbaImage MakeViewportScreenshot(int width, int height) = 0;

    virtual RgbaImage MakeFullScreenshot() = 0;

    virtual std::vector<Actor *> getActorsInViewport(int pDepth) = 0;

    virtual void BeginLightmaps() = 0;
    virtual void EndLightmaps() = 0;
    virtual void BeginLightmaps2() = 0;
    virtual void EndLightmaps2() = 0;
    virtual bool DrawLightmap(Lightmap *pLightmap,
                              Vec3f *pColorMult, float z_bias) = 0;

    virtual void BeginDecals() = 0;
    virtual void EndDecals() = 0;
    virtual void DrawDecal(Decal *pDecal, float z_bias) = 0;

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
    virtual bool ReloadShaders() = 0;
    virtual void DoRenderBillboards_D3D() = 0;

    virtual void flushAndScale() = 0;
    virtual void swapBuffers() = 0;
    virtual void beginOverlays() = 0;
    virtual void endOverlays() = 0;

    std::shared_ptr<GameConfig> config = nullptr;
    int *pActiveZBuffer;
    Color uFogColor;
    int hd_water_current_frame;
    GraphicsImage *hd_water_tile_anim[7];
    RenderBillboardD3D pBillboardRenderListD3D[1000];
    unsigned int uNumBillboardsToDraw; // TODO(captainurist): this is not properly cleared if BeginScene3D is not called,
                                       //                     resulting in dangling textures in pBillboardRenderListD3D.

    int drawcalls;

    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
    Vis *vis = nullptr;
};

extern Renderer *render;

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

void UpdateObjects();
