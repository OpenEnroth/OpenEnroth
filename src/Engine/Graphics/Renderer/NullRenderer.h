#pragma once

#include <string>

#include "BaseRenderer.h"

class NullRenderer : public BaseRenderer {
 public:
    using BaseRenderer::BaseRenderer;

    virtual bool Initialize() override;
    virtual bool Reinitialize(bool firstInit) override;

    virtual RgbaImage ReadScreenPixels() override;
    virtual void ClearTarget(Color uColor) override;
    virtual void Present() override;

    virtual bool InitializeFullscreen() override;

    virtual void Release() override;

    virtual bool SwitchToWindow() override;

    virtual void BeginLines2D() override;
    virtual void EndLines2D() override;
    virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ,
                              signed int uW, Color uColor32) override;
    virtual void DrawLines(const RenderVertexD3D3 *vertices,
                           unsigned int num_vertices) override;

    virtual void RestoreFrontBuffer() override;
    virtual void RestoreBackBuffer() override;
    virtual void BltBackToFontFast(int a2, int a3, Recti *pSrcRect) override;
    virtual void BeginScene3D() override;

    virtual void DrawTerrainPolygon(Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) override;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                GraphicsImage *texture) override;

    virtual TextureRenderId CreateTexture(RgbaImageView image) override;
    virtual void DeleteTexture(TextureRenderId id) override;
    virtual void UpdateTexture(TextureRenderId id, RgbaImageView image) override;

    virtual void Update_Texture(GraphicsImage *texture) override;

    virtual void BeginScene2D() override;
    virtual void ScreenFade(Color color, float t) override;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW) override;
    virtual void ResetUIClipRect() override;

    virtual void DrawTextureNew(float u, float v, GraphicsImage *, Color colourmask = colorTable.White) override;

    virtual void DrawTextureCustomHeight(float u, float v, GraphicsImage *,
                                         int height) override;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                                   GraphicsImage *) override;
    virtual void DrawImage(GraphicsImage *, const Recti &rect, unsigned int paletteid = 0, Color colourmask = colorTable.White) override;

    virtual void BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                               int start_opacity, int end_opacity) override;
    virtual void TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) override;

    virtual void BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) override;
    virtual void EndTextNew() override;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) override;

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              Color uColor32) override;

    virtual void DrawOutdoorBuildings() override;

    virtual void DrawIndoorSky(unsigned int uNumVertices, int uFaceID) override;
    virtual void DrawOutdoorSky() override;
    virtual void DrawOutdoorTerrain() override;

    virtual bool AreRenderSurfacesOk() override;

    virtual RgbaImage MakeViewportScreenshot(const int width, const int height) override;
    virtual RgbaImage MakeFullScreenshot() override;

    virtual void BeginLightmaps() override;
    virtual void EndLightmaps() override;
    virtual void BeginLightmaps2() override;
    virtual void EndLightmaps2() override;
    virtual bool DrawLightmap(Lightmap *pLightmap,
                              Vec3f *pColorMult, float z_bias) override;

    virtual void BeginDecals() override;
    virtual void EndDecals() override;
    virtual void DrawDecal(Decal *pDecal, float z_bias) override;

    virtual void DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3,
                                     int blend_mode) override;

    virtual void DrawIndoorFaces() override;

    virtual void ReleaseTerrain() override;
    virtual void ReleaseBSP() override;

    virtual void DrawTwodVerts() override;

    virtual bool ReloadShaders() override;

    virtual void DoRenderBillboards_D3D() override;

    virtual void flushAndScale() override;
    virtual void swapBuffers() override;

    virtual void beginOverlays() override;
    virtual void endOverlays() override;
};
