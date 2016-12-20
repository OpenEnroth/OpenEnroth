#pragma once

#include "lib\legacy_dx\d3d.h"
#include "OSWindow.h"
#include "RenderStruct.h"
#include "../VectorTypes.h"
#include "Engine/Graphics/Texture.h"

class Image;
class IRender
{
  public:
    virtual ~IRender() {}

    virtual bool Initialize(OSWindow *window) = 0;

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
    virtual bool LockSurface_DDraw4(IDirectDrawSurface4 *pSurface, DDSURFACEDESC2 *pDesc, unsigned int uLockFlags) = 0;
    virtual void GetTargetPixelFormat(DDPIXELFORMAT *pOut) = 0;
    virtual void LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow) = 0;
    virtual void UnlockBackBuffer() = 0;
    virtual void LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow) = 0;
    virtual void UnlockFrontBuffer() = 0;
    virtual void RestoreFrontBuffer() = 0;
    virtual void RestoreBackBuffer() = 0;
    virtual void BltToFront(RECT *pDstRect, IDirectDrawSurface *pSrcSurface, RECT *pSrcRect, unsigned int uBltFlags) = 0;
    virtual void BltBackToFontFast(int a2, int a3, RECT *a4) = 0;
    virtual void BeginSceneD3D() = 0;

    virtual unsigned int GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6) = 0;

    virtual void DrawPolygon(unsigned int uNumVertices, struct Polygon *a3, ODMFace *a4, IDirect3DTexture2 *pTexture) = 0;
    virtual void DrawTerrainPolygon(unsigned int uNumVertices, struct Polygon *a4, IDirect3DTexture2 *a5, bool transparent, bool clampAtTextureBorders) = 0;
    virtual void DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, IDirect3DTexture2 *pHwTex, struct Texture_MM7 *pTex, int uPackedID, unsigned int uColor, int a8) = 0;

    virtual void MakeParticleBillboardAndPush_BLV(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle) = 0;
    virtual void MakeParticleBillboardAndPush_ODM(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle) = 0;

    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() = 0;
    virtual void DrawBillboard_Indoor(RenderBillboardTransform_local0 *pSoftBillboard, Sprite *pSprite, int dimming_level) = 0;
    virtual void _4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse) = 0;
    virtual void TransformBillboardsAndSetPalettesODM() = 0;
    virtual void DrawBillboardList_BLV() = 0;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, IDirect3DTexture2 *a9) = 0;
    virtual bool LoadTexture(const char *pName, unsigned int bMipMaps, IDirectDrawSurface4 **pOutSurface, IDirect3DTexture2 **pOutTexture) = 0;
    virtual bool MoveSpriteToDevice(Sprite *pSprite) = 0;

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
    virtual void DrawOutdoorSkyPolygon(unsigned int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture) = 0;
    virtual void DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture) = 0;

    virtual void PrepareDecorationsRenderList_ODM() = 0;
    virtual void DrawSpriteObjects_ODM() = 0;

    virtual void RenderTerrainD3D() = 0;

    virtual bool AreRenderSurfacesOk() = 0;
    virtual bool IsGammaSupported() = 0;

    virtual void SaveScreenshot(const String &filename, unsigned int width, unsigned int height) = 0;
    virtual void PackScreenshot(unsigned int width, unsigned int height, void *out_data, unsigned int data_size, unsigned int *screenshot_size) = 0;
    virtual void SavePCXScreenshot() = 0;

    virtual int _46ภ6ภั_GetActorsInViewport(int pDepth) = 0;

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

    virtual void DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, IDirect3DTexture2 *texture) = 0;

    virtual void am_Blt_Copy(RECT *pSrcRect, POINT *pTargetXY, int a3) = 0;
    virtual void am_Blt_Chroma(RECT *pSrcRect, POINT *pTargetPoint, int a3, int blend_mode) = 0;








    inline IRender()
    {
      pActiveZBuffer = 0;
      pDirectDraw4 = 0;
      pFrontBuffer4 = 0;
      pBackBuffer4 = 0;
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
    unsigned int uNumBillboardsToDraw;

    virtual void WritePixel16(int x, int y, unsigned __int16 color) = 0;
    virtual unsigned __int16 ReadPixel16(int x, int y) = 0;

    virtual void ToggleTint() = 0;
    virtual void ToggleColoredLights() = 0;

    virtual unsigned int GetRenderWidth() = 0;
    virtual unsigned int GetRenderHeight() = 0;

    virtual void Sub01() = 0;
};