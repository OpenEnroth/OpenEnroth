#pragma once

#include <memory>
#include <vector>

#include "Library/Image/Image.h"
#include "Library/Color/Color.h"
#include "Library/Color/ColorTable.h"
#include "Library/Geometry/Rect.h"
#include "Engine/HitMap.h"

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
struct Decal;
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

    virtual RgbaImage ReadScreenPixels() = 0;
    virtual void ClearTarget(Color uColor) = 0;
    virtual void Present() = 0;

    virtual void BeginLines2D() = 0;
    virtual void EndLines2D() = 0;
    virtual void RasterLine2D(Pointi a, Pointi b, Color acolor, Color bcolor) = 0;

    void RasterLine2D(Pointi a, Pointi b, Color color) {
        RasterLine2D(a, b, color, color);
    }

    virtual void BeginScene3D() = 0;

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

    virtual void BeginScene2D() = 0;
    virtual void ScreenFade(Color color, float t) = 0;

    virtual void SetUIClipRect(const Recti &rect) = 0;
    virtual void ResetUIClipRect() = 0;

    /**
     * Core 2D quad drawing function. All other 2D texture drawing functions (except `DrawImage`) delegate to this one.
     *
     * @param texture                   The texture to draw.
     * @param srcRect                   Source rectangle in texture pixel coordinates.
     * @param dstRect                   Destination rectangle in screen pixels.
     * @param color                     Multiplicative color mask applied to the texture. Use `colorTable.White` for no
     *                                  tinting. RGB components scale the texture colors, alpha controls transparency.
     */
    virtual void DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, const Recti &dstRect, Color color) = 0;
    void DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, Pointi dstPoint, Color color);

    void DrawTextureNew(float u, float v, GraphicsImage *img, Color colourmask = colorTable.White);
    void FillRectFast(int uX, int uY, int uWidth, int uHeight, Color uColor32);

    // TODO(captainurist): DrawImage uses palette, should be refactored to use a separate palette shader.
    virtual void DrawImage(GraphicsImage *, const Recti &rect, int paletteid = 0, Color colourmask32 = colorTable.White) = 0;

    virtual void BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t, int start_opacity, int end_opacity) = 0;
    virtual void DrawMonsterPortrait(const Recti &rc, SpriteFrame *Portrait_Sprite, int Y_Offset) = 0;

    virtual void BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) = 0;
    virtual void EndTextNew() = 0;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) = 0;

    virtual void DrawOutdoorBuildings() = 0;

    virtual void DrawIndoorSky(int uNumVertices, int uFaceID = 0) = 0;
    virtual void DrawOutdoorSky() = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects() = 0;

    virtual void DrawOutdoorTerrain() = 0;

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

    virtual void BeginDecals() = 0;
    virtual void EndDecals() = 0;
    virtual void DrawDecal(Decal *pDecal, float z_bias) = 0;

    virtual void DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) = 0;

    virtual void DrawIndoorFaces() = 0;

    virtual void ReleaseTerrain() = 0;
    virtual void ReleaseBSP() = 0;

    virtual void DrawTwodVerts() = 0;

    virtual Sizei GetRenderDimensions() = 0;
    virtual Sizei GetPresentDimensions() = 0;
    virtual Pointi MapToRender(Pointi position) = 0;
    virtual Pointi MapToPresent(Pointi position) = 0;

    virtual bool Reinitialize(bool firstInit = false) = 0;
    virtual bool ReloadShaders() = 0;
    virtual void DoRenderBillboards_D3D() = 0;

    virtual void flushAndScale() = 0;
    virtual void swapBuffers() = 0;
    virtual void beginOverlays() = 0;
    virtual void endOverlays() = 0;

    std::shared_ptr<GameConfig> config = nullptr;

    Color uFogColor;

    static const int MAX_BILLBOARDS_D3D = 5000;
    RenderBillboardD3D pBillboardRenderListD3D[MAX_BILLBOARDS_D3D];
    std::array<RenderBillboardD3D*, MAX_BILLBOARDS_D3D> pSortedBillboardRenderListD3D;
    unsigned int uNumBillboardsToDraw; // TODO(captainurist): this is not properly cleared if BeginScene3D is not called,
                                       //                     resulting in dangling textures in pBillboardRenderListD3D.

    int drawcalls;

    /**
     * @returns                         1x1 white texture for solid color fills. Initialized lazily on first use.
     */
    GraphicsImage *solidFillTexture();

 protected:
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
    Vis *vis = nullptr;

 private:
    GraphicsImage *_solidFillTexture = nullptr;
};

extern Renderer *render;

extern int uNumDecorationsDrawnThisFrame;
extern RenderBillboard pBillboardRenderList[500];
extern unsigned int uNumBillboardsToDraw;
extern int uNumSpritesDrawnThisFrame;

extern RenderVertexSoft VertexRenderList[50];
extern RenderVertexSoft array_73D150[20];


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
