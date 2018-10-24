#pragma once

#include <d3d.h>
#include <ddraw.h>

#include <GdiPlus.h>
#undef DrawText

#include <memory>

#include "Engine/Strings.h"
#include "Engine/VectorTypes.h"

#include "Engine/Graphics/RenderBase.h"

struct ODMFace;
class RenderD3D;
class Image;

class Render : public RenderBase {
 public:
    Render();
    virtual ~Render();

    virtual bool Initialize(OSWindow *window);

    virtual Texture *CreateTexture_ColorKey(const String &name, uint16_t colorkey);
    virtual Texture *CreateTexture_Solid(const String &name);
    virtual Texture *CreateTexture_Alpha(const String &name);

    virtual Texture *CreateTexture_PCXFromFile(const String &name);
    virtual Texture *CreateTexture_PCXFromIconsLOD(const String &name);
    virtual Texture *CreateTexture_PCXFromNewLOD(const String &name);

    virtual Texture *CreateTexture_Blank(unsigned int width, unsigned int height,
        IMAGE_FORMAT format, const void *pixels = nullptr);

    virtual Texture *CreateTexture(const String &name);
    virtual Texture *CreateSprite(const String &name, unsigned int palette_id,
                                  unsigned int lod_sprite_id);

    virtual void ClearBlack();
    virtual void PresentBlackScreen();

    virtual void SaveWinnersCertificate(const char *a1);
    virtual void ClearTarget(unsigned int uColor);
    virtual void Present();

    virtual bool InitializeFullscreen();

    virtual void CreateZBuffer();
    virtual void Release();

    virtual bool SwitchToWindow();
    virtual void RasterLine2D(int uX, int uY, int uZ, int uW, uint16_t uColor);
    virtual void ClearZBuffer(int a2, int a3);
    virtual void RestoreFrontBuffer();
    virtual void RestoreBackBuffer();
    virtual void BltBackToFontFast(int a2, int a3, Rect *pSrcRect);
    virtual void BeginSceneD3D();

    virtual unsigned int GetActorTintColor(int DimLevel, int tint, float WorldViewX, int a5, RenderBillboard *Billboard);

    virtual void DrawPolygon(struct Polygon *a3);
    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders);
    virtual void DrawIndoorPolygon(unsigned int uNumVertices,
                                   struct BLVFace *a3, int uPackedID,
                                   unsigned int uColor, int a8);

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    virtual void DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard,
                                      RenderBillboard *billboard);
    virtual void _4A4CC9_AddSomeBillboard(
        struct SpellFX_Billboard *a1, int diffuse);
    virtual void DrawBillboardList_BLV();

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                Texture *texture);
    virtual bool MoveTextureToDevice(Texture *texture);

    virtual void Update_Texture(Texture *texture);

    virtual void DeleteTexture(Texture *texture);

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
    virtual void BlendTextures(int x, int y, Image *imgin, Image *imgblend,
                               int time, int start_opacity, int end_opacity);
    virtual void DrawMonsterPortrait(Rect rc, SpriteFrame *Portrait, int Y_Offset);

    virtual void DrawMasked(float u, float v, class Image *img,
                            unsigned int color_dimming_level, uint16_t mask);
    virtual void TexturePixelRotateDraw(float u, float v, Image * img, int time);
    virtual void DrawTextureGrayShade(float u, float v, class Image *a4);
    virtual void DrawTransparentRedShade(float u, float v, class Image *a4);
    virtual void DrawTransparentGreenShade(float u, float v,
                                           class Image *pTexture);
    virtual void DrawFansTransparent(const RenderVertexD3D3 *vertices,
                                     unsigned int num_vertices);

    virtual void DrawTextAlpha(int x, int y, uint8_t *font_pixels, int a5,
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

    friend void Present_NoColorKey();

    void GetTargetPixelFormat(DDPIXELFORMAT *pOut);

 protected:
    IDirectDraw4 *pDirectDraw4;
    IDirectDrawSurface4 *pFrontBuffer4;
    IDirectDrawSurface4 *pBackBuffer4;

    unsigned int uDesiredDirect3DDevice;
    int *pDefaultZBuffer;
    unsigned int bWindowMode;
    RenderD3D *pRenderD3D;
    unsigned int uTargetRBits;
    unsigned int uTargetGBits;
    unsigned int uTargetBBits;
    unsigned int uNumD3DSceneBegins;
    unsigned int bRequiredTextureStagesAvailable;
    unsigned int uMaxDeviceTextureDim;
    unsigned int uMinDeviceTextureDim;

    // 2D drawing
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::Bitmap *p2DSurface;
    Gdiplus::Graphics *p2DGraphics;

    void DoRenderBillboards_D3D();
    void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);
    unsigned int GetParentBillboardID(unsigned int uBillboardID);
    unsigned int GetBillboardDrawListSize();

    void DrawBorderTiles(struct Polygon *poly);

    unsigned short *MakeScreenshot(signed int width, signed int height);
    bool CheckTextureStages();
    void ParseTargetPixelFormat();

    void CreateClipper(OSWindow *);

    void SavePCXImage16(const String &filename, uint16_t *picture_data,
                        int width, int height);
    void SavePCXImage32(const String &filename, uint16_t *picture_data,
                        int width, int height);

    Gdiplus::Bitmap *BitmapWithImage(Image *image);

    bool LockSurface_DDraw4(IDirectDrawSurface4 *pSurface,
                            DDSURFACEDESC2 *pDesc, unsigned int uLockFlags);
    void DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon);
    void DrawIndoorSkyPolygon(int uNumVertices, struct Polygon *pSkyPolygon);
};
