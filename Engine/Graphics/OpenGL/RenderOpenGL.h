#pragma once

#include "Engine/Graphics/RenderBase.h"

class RenderOpenGL : public RenderBase {
 public:
    RenderOpenGL();
    virtual ~RenderOpenGL();

    virtual bool Initialize(OSWindow *window);

    virtual Texture *CreateTexture(const String &name);
    virtual Texture *CreateSprite(
        const String &name, unsigned int palette_id,
        /*refactor*/ unsigned int lod_sprite_id);

    virtual void ClearBlack();
    virtual void PresentBlackScreen();

    virtual void SaveWinnersCertificate(const char *a1);
    virtual void ClearTarget(unsigned int uColor);
    virtual void Present();

    virtual bool InitializeFullscreen();

    virtual void CreateZBuffer();
    virtual void Release();

    virtual bool SwitchToWindow();
    virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ,
                              signed int uW, unsigned __int16 uColor);
    virtual void ClearZBuffer(int a2, int a3);
    virtual void RestoreFrontBuffer();
    virtual void RestoreBackBuffer();
    virtual void BltBackToFontFast(int a2, int a3, Rect *pSrcRect);
    virtual void BeginSceneD3D();

    virtual unsigned int GetActorTintColor(float a2, int tint, int a4, int a5,
                                           RenderBillboard *a6);

    virtual void DrawPolygon(struct Polygon *a3);
    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders);
    virtual void DrawIndoorPolygon(unsigned int uNumVertices,
                                   struct BLVFace *a3, int uPackedID,
                                   unsigned int uColor, int a8);

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    virtual void DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard,
                                      RenderBillboard *);
    virtual void _4A4CC9_AddSomeBillboard(
        struct stru6_stru1_indoor_sw_billboard *a1, int diffuse);
    virtual void DrawBillboardList_BLV();

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                Texture *texture);
    virtual bool MoveTextureToDevice(Texture *texture);

    virtual void BeginScene();
    virtual void EndScene();
    virtual void ScreenFade(unsigned int color, float t);

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW);
    virtual void ResetUIClipRect();

    virtual void DrawTextureNew(float u, float v, class Image *);
    virtual void DrawTextureAlphaNew(float u, float v, class Image *);
    virtual void DrawTextureCustomHeight(float u, float v, class Image *,
                                         int height);
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                                   Image *);
    virtual void DrawImage(Image *, const Rect &rect);

    virtual void ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture,
                                int a5);
    virtual void ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal);
    virtual void BlendTextures(int a2, int a3, Image *a4, Image *a5, int t,
                               int start_opacity, int end_opacity);
    virtual void TexturePixelRotateDraw(float u, float v, Image *img, int time);

    virtual void DrawMasked(float u, float v, class Image *img,
                            unsigned int color_dimming_level,
                            unsigned __int16 mask);
    virtual void DrawTextureGrayShade(float u, float v, class Image *a4);
    virtual void DrawTransparentRedShade(float u, float v, class Image *a4);
    virtual void DrawTransparentGreenShade(float u, float v,
                                           class Image *pTexture);
    virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices,
                                     unsigned int num_vertices);

    virtual void DrawTextAlpha(int x, int y, unsigned char *font_pixels, int a5,
                               unsigned int uFontHeight, uint8_t *pPalette,
                               bool present_time_transparency);
    virtual void DrawText(int uOutX, int uOutY, uint8_t *pFontPixels,
                          unsigned int uCharWidth, unsigned int uCharHeight,
                          uint8_t *pFontPalette, uint16_t uFaceColor,
                          uint16_t uShadowColor);

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              unsigned int uColor16);

    virtual void DrawBuildingsD3D();

    virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID);
    virtual void DrawOutdoorSkyD3D();

    virtual void PrepareDecorationsRenderList_ODM();

    virtual void RenderTerrainD3D();

    virtual bool AreRenderSurfacesOk();

    virtual Image *TakeScreenshot(unsigned int width, unsigned int height);
    virtual void SaveScreenshot(const String &filename, unsigned int width,
                                unsigned int height);
    virtual void PackScreenshot(unsigned int width, unsigned int height,
                                void *out_data, unsigned int data_size,
                                unsigned int *screenshot_size);
    virtual void SavePCXScreenshot();

    virtual int GetActorsInViewport(int pDepth);

    virtual void BeginLightmaps();
    virtual void EndLightmaps();
    virtual void BeginLightmaps2();
    virtual void EndLightmaps2();
    virtual bool DrawLightmap(struct Lightmap *pLightmap,
                              struct Vec3_float_ *pColorMult, float z_bias);

    virtual void BeginDecals();
    virtual void EndDecals();
    virtual void DrawDecal(struct Decal *pDecal, float z_bias);

    virtual void do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin,
                                        signed int sDiffuseBegin,
                                        const RenderVertexD3D3 *pLineEnd,
                                        signed int sDiffuseEnd, float z_stuff);
    virtual void DrawLines(const RenderVertexD3D3 *vertices,
                           unsigned int num_vertices);

    virtual void DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices,
                                        Texture *texture);

    virtual void am_Blt_Copy(Rect *pSrcRect, Point *pTargetXY, int a3);
    virtual void am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3,
                               int blend_mode);

    virtual HWLTexture *LoadHwlBitmap(const char *name);
    virtual HWLTexture *LoadHwlSprite(const char *name);

 public:
    virtual void WritePixel16(int x, int y, uint16_t color);

    virtual unsigned int GetRenderWidth() const;
    virtual unsigned int GetRenderHeight() const;

 protected:
    void DoRenderBillboards_D3D();
    void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);

    void DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon);
    void DrawIndoorSkyPolygon(signed int uNumVertices,
                              struct Polygon *pSkyPolygon);

    OSWindow *window;

    void *hdc;
    int clip_x, clip_y;
    int clip_z, clip_w;
    unsigned char *render_target_rgb;

    RenderHWLContainer pD3DBitmaps;
    RenderHWLContainer pD3DSprites;
};
