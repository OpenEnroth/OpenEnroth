#pragma once
/*
#include <cstdint>
#include <cstdio>
#include <array>
*/
#include <d3d.h>
#include <ddraw.h>

#include "Engine/Strings.h"
#include "Engine/VectorTypes.h"

#include "Engine/Graphics/IRender.h"


struct ODMFace;


class RenderD3D;

class Image;
class Render : public IRender
{
    public:
        Render();
        virtual ~Render();

        virtual bool Initialize(OSWindow *window);

        virtual Texture *CreateTexture(const String &name) override;
        virtual Texture *CreateSprite(const String &name, unsigned int palette_id, /*refactor*/unsigned int lod_sprite_id) override;

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
        virtual bool LockSurface(Texture *texture, Rect *, void **out_surface, int *out_pitch, int *out_width, int *out_height);
        virtual void UnlockSurface(Texture *texture);
        virtual void LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow);
        virtual void UnlockBackBuffer();
        virtual void LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow);
        virtual void UnlockFrontBuffer();
        virtual void RestoreFrontBuffer();
        virtual void RestoreBackBuffer();
        virtual void BltBackToFontFast(int a2, int a3, Rect *pSrcRect);
        virtual void BeginSceneD3D();

        virtual unsigned int GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6);

        virtual void DrawPolygon(struct Polygon *a3);
        virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent, bool clampAtTextureBorders);
        virtual void DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, int uPackedID, unsigned int uColor, int a8);

        virtual void MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle);
        virtual void MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle);

        virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
        virtual void DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard, RenderBillboard *billboard);
        virtual void _4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse);
        virtual void TransformBillboardsAndSetPalettesODM();
        virtual void DrawBillboardList_BLV();

        virtual void DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, Texture *texture);
        virtual bool MoveTextureToDevice(Texture *texture) override;

        virtual void BeginScene();
        virtual void EndScene();
        virtual void ScreenFade(unsigned int color, float t);

        virtual void SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW);
        virtual void ResetUIClipRect();

        virtual void DrawTextureNew(float u, float v, class Image *);
        virtual void DrawTextureAlphaNew(float u, float v, class Image *);
        virtual void DrawTextureCustomHeight(float u, float v, class Image *, int height);
        virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y, Image *);

        virtual void ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture, int a5);
        virtual void ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal);
        virtual void BlendTextures(int x, int y, Image *imgin, Image *imgblend, int time, int start_opacity, int end_opacity);
        virtual void _4A65CC(unsigned int x, unsigned int y, Image *a4, Image *a5, int a6, int a7, int a8);

        virtual void DrawMasked(float u, float v, class Image *img, unsigned int color_dimming_level, unsigned __int16 mask);
        virtual void DrawTextureGrayShade(float u, float v, class Image *a4);
        virtual void DrawTransparentRedShade(float u, float v, class Image *a4);
        virtual void DrawTransparentGreenShade(float u, float v, class Image *pTexture);
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
        virtual void DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon);
        virtual void DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon);

        virtual void PrepareDecorationsRenderList_ODM();
        virtual void DrawSpriteObjects_ODM();

        virtual void RenderTerrainD3D();

        virtual bool AreRenderSurfacesOk();

        virtual void SaveScreenshot(const String &filename, unsigned int width, unsigned int height);
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

        virtual void DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, Texture *texture) override;

        virtual void am_Blt_Copy(Rect *pSrcRect, Point *pTargetXY, int a3);
        virtual void am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3, int blend_mode);

        virtual HWLTexture *LoadHwlBitmap(const char *name);
        virtual HWLTexture *LoadHwlSprite(const char *name);

    public:
        virtual void WritePixel16(int x, int y, uint16_t color);

        virtual void ToggleTint() { bTinting = !bTinting; }
        virtual void ToggleColoredLights() { bUseColoredLights = !bUseColoredLights; }

        virtual unsigned int GetRenderWidth() const;
        virtual unsigned int GetRenderHeight() const;

        virtual void Sub01()
        {
            if (pRenderD3D && !bWindowMode)
                _49FD3A_fullscreen();
        }

        friend void Present_NoColorKey();

        void GetTargetPixelFormat(DDPIXELFORMAT *pOut);
        bool LockSurface_DDraw4(IDirectDrawSurface4 *pSurface, DDSURFACEDESC2 *pDesc, unsigned int uLockFlags);

    protected:
        IDirectDraw4 *pDirectDraw4;
        IDirectDrawSurface4 *pFrontBuffer4;
        IDirectDrawSurface4 *pBackBuffer4;

        unsigned int uDesiredDirect3DDevice;
        int *pDefaultZBuffer;
        OSWindow *window;
        unsigned int bWindowMode;
        RenderD3D *pRenderD3D;
        unsigned int uTargetRBits;
        unsigned int uTargetGBits;
        unsigned int uTargetBBits;
        unsigned int uTargetRMask;
        unsigned int uTargetGMask;
        unsigned int uTargetBMask;
        unsigned int uClipY;
        unsigned int uClipX;
        unsigned int uClipW;
        unsigned int uClipZ;
        unsigned int bClip;
        unsigned int uNumD3DSceneBegins;
        RenderHWLContainer pD3DBitmaps;
        RenderHWLContainer pD3DSprites;
        unsigned int bRequiredTextureStagesAvailable;
        unsigned int uLevelOfDetail;
        unsigned int uMaxDeviceTextureDim;
        unsigned int uMinDeviceTextureDim;

        void DoRenderBillboards_D3D();
        void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);
        void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);
        unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
        unsigned int GetParentBillboardID(unsigned int uBillboardID);
        unsigned int GetBillboardDrawListSize();

        void DrawBorderTiles(struct Polygon *poly);

        unsigned short *MakeScreenshot(signed int width, signed int height);
        bool CheckTextureStages();
        void ParseTargetPixelFormat();

        void CreateDirectDraw();
        void SetDirectDrawCooperationMode(OSWindow *, bool bFullscreen);
        void SetDirectDrawDisplayMode(unsigned int uWidth, unsigned int uHeight, unsigned int uBPP);
        void CreateFrontBuffer();
        void CreateBackBuffer();
        void CreateDirectDrawPrimarySurface();
        void CreateClipper(OSWindow *);

        void SavePCXImage16(const String &filename, uint16_t *picture_data, int width, int height);
        void SavePCXImage32(const String &filename, uint16_t *picture_data, int width, int height);
};
