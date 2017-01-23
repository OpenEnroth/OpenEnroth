#include <Windows.h>
#undef DrawText
#undef PlaySound
#undef Polygon

#include <gl\gl.h>
#include <gl\glu.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/OurMath.h"
#include "Engine/SpellFxRenderer.h"

#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Sprites.h"

#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"

#include "Platform/OsWindow.h"
#include "Platform/Api.h"



unsigned int BlendColors(unsigned int a1, unsigned int a2);



//IRender *IRender::Create() { return new RenderOpenGL(); }




RenderOpenGL::RenderOpenGL() : IRender() { bFogEnabled = false; }
RenderOpenGL::~RenderOpenGL() {}

unsigned int RenderOpenGL::GetRenderWidth() const { return window->GetWidth(); }
unsigned int RenderOpenGL::GetRenderHeight() const { return window->GetHeight(); }

void RenderOpenGL::ClearBlack() { __debugbreak(); }
void RenderOpenGL::SaveWinnersCertificate(const char *a1) { __debugbreak(); }
void RenderOpenGL::_49FD3A_fullscreen() { __debugbreak(); }
bool RenderOpenGL::InitializeFullscreen() { __debugbreak(); return 0; }
void RenderOpenGL::CreateZBuffer()
{
    pActiveZBuffer = (int *)malloc(0x12C000);
    memset32(pActiveZBuffer, 0xFFFF0000, 0x4B000u); //    // inlined Render::ClearActiveZBuffer  (mm8::004A085B)
}
void RenderOpenGL::Release() { __debugbreak(); }
void RenderOpenGL::RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, unsigned __int16 uColor) { __debugbreak(); }
void RenderOpenGL::LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow) { __debugbreak(); }
void RenderOpenGL::UnlockBackBuffer() { __debugbreak(); }
void RenderOpenGL::LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow) { __debugbreak(); }
void RenderOpenGL::UnlockFrontBuffer() { __debugbreak(); }
void RenderOpenGL::RestoreFrontBuffer() {}
void RenderOpenGL::RestoreBackBuffer() {}
void RenderOpenGL::BltBackToFontFast(int a2, int a3, Rect *a4) { __debugbreak(); }
void RenderOpenGL::BeginSceneD3D()
{
    glClearColor(0.9f, 0.5f, 0.1f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
unsigned int RenderOpenGL::GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6) { __debugbreak(); return 0; }
void RenderOpenGL::DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, int uPackedID, unsigned int uColor, int a8) { __debugbreak(); }
void RenderOpenGL::MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2, Texture *a3, unsigned int uDiffuse, int angle) { __debugbreak(); }
void RenderOpenGL::MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2, Texture *a3, unsigned int uDiffuse, int angle) { __debugbreak(); }
void RenderOpenGL::DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard, RenderBillboard *) { __debugbreak(); }
void RenderOpenGL::_4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse) { __debugbreak(); }
void RenderOpenGL::DrawBillboardList_BLV() { __debugbreak(); }
void RenderOpenGL::DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, Texture *texture) { __debugbreak(); }
void RenderOpenGL::ScreenFade(unsigned int color, float t) { __debugbreak(); }
void RenderOpenGL::DrawTextureOffset(int pX, int pY, int move_X, int move_Y, Image *pTexture) { __debugbreak(); }
void RenderOpenGL::ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture, int a5) { __debugbreak(); }
void RenderOpenGL::ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal) { __debugbreak(); }
void RenderOpenGL::BlendTextures(int a2, int a3, Image *a4, Image *a5, int t, int start_opacity, int end_opacity) { __debugbreak(); }
void RenderOpenGL::_4A65CC(unsigned int x, unsigned int y, Image *a4, Image *a5, int a6, int a7, int a8) { __debugbreak(); }
void RenderOpenGL::DrawTransparentRedShade(float u, float v, Image *a4) { __debugbreak(); }
void RenderOpenGL::DrawTransparentGreenShade(float u, float v, Image *pTexture) { __debugbreak(); }
void RenderOpenGL::DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices) { __debugbreak(); }
void RenderOpenGL::DrawMasked(float u, float v, Image *pTexture, unsigned int, unsigned __int16 mask) { __debugbreak(); }
void RenderOpenGL::_4A6DF5(unsigned __int16 *pBitmap, unsigned int uBitmapPitch, struct Vec2_int_ *pBitmapXY, void *pTarget, unsigned int uTargetPitch, Vec4_int_ *a7) { __debugbreak(); }
void RenderOpenGL::DrawTextureGrayShade(float a2, float a3, Image *a4) { __debugbreak(); }
void RenderOpenGL::DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID) { __debugbreak(); }
void RenderOpenGL::DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon) { __debugbreak(); }
bool RenderOpenGL::AreRenderSurfacesOk() { return true; }
void RenderOpenGL::SaveScreenshot(const String &filename, unsigned int width, unsigned int height) { __debugbreak(); }
void RenderOpenGL::SavePCXScreenshot() { __debugbreak(); }
int RenderOpenGL::_46А6АС_GetActorsInViewport(int pDepth) { __debugbreak(); return 0; }
void RenderOpenGL::BeginLightmaps() { __debugbreak(); }
void RenderOpenGL::EndLightmaps() { __debugbreak(); }
void RenderOpenGL::BeginLightmaps2() { __debugbreak(); }
void RenderOpenGL::EndLightmaps2() { __debugbreak(); }
bool RenderOpenGL::DrawLightmap(struct Lightmap *pLightmap, struct Vec3_float_ *pColorMult, float z_bias) { __debugbreak(); return 0; }
void RenderOpenGL::BeginDecals() { __debugbreak(); }
void RenderOpenGL::EndDecals() { __debugbreak(); }
void RenderOpenGL::DrawDecal(struct Decal *pDecal, float z_bias) { __debugbreak(); }
void RenderOpenGL::do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin, signed int sDiffuseBegin, const RenderVertexD3D3 *pLineEnd, signed int sDiffuseEnd, float z_stuff) { __debugbreak(); }
void RenderOpenGL::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) { __debugbreak(); }
void RenderOpenGL::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, Texture *texture) { __debugbreak(); }
void RenderOpenGL::am_Blt_Copy(Rect *pSrcRect, Point *pTargetXY, int blend_mode) { __debugbreak(); }
void RenderOpenGL::am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3, int blend_mode) { __debugbreak(); }



void RenderOpenGL::PrepareDecorationsRenderList_ODM()
{
    //__debugbreak();
}
void RenderOpenGL::DrawSpriteObjects_ODM()
{
    //__debugbreak();
}



/*#pragma pack(push, 1)
typedef struct {
	char  idlength;
	char  colourmaptype;
	char  datatypecode;
	short int colourmaporigin;
	short int colourmaplength;
	char  colourmapdepth;
	short int x_origin;
	short int y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
} tga;
#pragma pack(pop)

FILE *CreateTga(const char *filename, int image_width, int image_height)
{
	auto f = fopen(filename, "w+b");

	tga tga_header;
	memset(&tga_header, 0, sizeof(tga_header));

	tga_header.colourmaptype = 0;
	tga_header.datatypecode = 2;
	//tga_header.colourmaporigin = 0;
	//tga_header.colourmaplength = image_width * image_height;
	//tga_header.colourmapdepth = 32;
	tga_header.x_origin = 0;
	tga_header.y_origin = 0;
	tga_header.width = image_width;
	tga_header.height = image_height;
	tga_header.bitsperpixel = 32;
	tga_header.imagedescriptor = 32; // top-down
	fwrite(&tga_header, 1, sizeof(tga_header), f);

	return f;
}*/




Texture *RenderOpenGL::CreateTexture(const String &name)
{
    return TextureOpenGL::Create(
        new Bitmaps_LOD_Loader(pBitmaps_LOD, name)
    );
}

Texture *RenderOpenGL::CreateSprite(const String &name, unsigned int palette_id, /*refactor*/unsigned int lod_sprite_id)
{
    return TextureOpenGL::Create(
        new Sprites_LOD_Loader(pSprites_LOD, palette_id, name, lod_sprite_id)
    );
}


bool RenderOpenGL::MoveTextureToDevice(Texture *texture)
{
    auto t = (TextureOpenGL *)texture;
/*    return false;
}

bool RenderOpenGL::LoadTextureOpenGL(const String &name, bool mipmaps, int *out_texture)
{*/
    /*String filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    HWLTexture* pHWLTexture = pD3DBitmaps.LoadTexture(filename.c_str(), mipmaps);
    if (!pHWLTexture)
        return false;

    auto rgb = new unsigned char[pHWLTexture->uWidth * pHWLTexture->uHeight * 4];
    for (unsigned int i = 0; i < pHWLTexture->uWidth * pHWLTexture->uHeight; ++i)
    {
        rgb[i * 4 + 3] = pHWLTexture->pPixels[i] & 0x8000 ? 0xFF : 0x00;
        rgb[i * 4 + 2] = 8 * ((pHWLTexture->pPixels[i] >> 0) & 0x1F);
        rgb[i * 4 + 1] = 8 * ((pHWLTexture->pPixels[i] >> 5) & 0x1F);
        rgb[i * 4 + 0] = 8 * ((pHWLTexture->pPixels[i] >> 10) & 0x1F);
    }*/

    /*if (filename == "plansky3")
    {
    if (auto f = CreateTga((name + ".tga").c_str(), pHWLTexture->uWidth, pHWLTexture->uHeight))
    {
        for (unsigned int i = 0; i < pHWLTexture->uWidth * pHWLTexture->uHeight; ++i)
        {
            int r = rgb[i * 4 + 0];
            int g = rgb[i * 4 + 1];
            int b = rgb[i * 4 + 2];
            int a = rgb[i * 4 + 2];
            int c = (a << 24 ) | (r << 16) | (g << 8) | b;

            fwrite(&c, 4, 1, f);
        }
        fclose(f);
    }
    }*/

    auto native_format = t->GetFormat();
    unsigned __int8 *pixels = nullptr;
    if (native_format == IMAGE_FORMAT_R5G6B5)
        pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8);
    else if (native_format == IMAGE_FORMAT_A1R5G5B5)
        pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8A8);

    if (pixels)
    {
        GLuint texid;
        glGenTextures(1, &texid);
        t->SetOpenGlTexture(texid);

        int gl_format = native_format == IMAGE_FORMAT_A1R5G5B5 ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, gl_format, t->GetWidth(), t->GetHeight(), 0, gl_format, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);


        return true;
    }
    return false;
}

void _set_3d_projection_matrix()
{
    float near_clip = 8.0;
    float far_clip = 4 * pODMRenderParams->shading_dist_mist;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0f, (GLfloat)window->GetWidth() / (GLfloat)window->GetHeight(), near_clip, far_clip);

}
void _set_3d_modelview_matrix()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);

    int camera_x = pParty->vPosition.x;
    int camera_y = pParty->vPosition.z + pParty->sEyelevel;
    int camera_z = pParty->vPosition.y;
    gluLookAt(
        camera_x, camera_y, camera_z,

        camera_x - pParty->y_rotation_granularity * cosf(2 * 3.14159 * pParty->sRotationY / 2048.0) - 3,
        camera_y - pParty->y_rotation_granularity * sinf(2 * 3.14159 * pParty->sRotationX / 2048.0),
        camera_z - pParty->y_rotation_granularity * sinf(2 * 3.14159 * pParty->sRotationY / 2048.0),

        0, 1, 0
    );
}


void _set_ortho_projection()
{
    glViewport(0, 0, window->GetWidth(), window->GetHeight());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window->GetWidth(), window->GetHeight(), 0, -1, 1);
}

void _set_ortho_modelview()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const int terrain_block_scale = 512;
const int terrain_height_scale = 32;
void RenderOpenGL::RenderTerrainD3D()
{
    glEnable(GL_TEXTURE_2D);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    for (int z = 0; z < 128 - 1; ++z)
    {
        for (int x = 0; x < 128 - 1; ++x)
        {
            auto tile = pOutdoor->DoGetTile(x, z);
            if (!tile)
                continue;

            struct Polygon p;
            auto *poly = &p;

            poly->texture = tile->GetTexture();
            if (tile->IsWaterTile())
            {
                poly->texture = this->hd_water_tile_anim[this->hd_water_current_frame];
            }

            //Генерация местоположения вершин-------------------------------------------------------------------------
            //решётка вершин делится на две части от -64 до 0 и от 0 до 64
            //
            //          X
            // -64      0     64
            //   --------------- 64
            //   |      |      |
            //   |      |      |
            //   |      |      |
            //  0|------+------|  Z
            //   |      |      |
            //   |      |      |
            //   |      |      |
            //   ---------------
            //                  -64

            int x1 = x;
            int x2 = x + 1;
            int z1 = z;
            int z2 = z + 1;
            int y11 = pOutdoor->pTerrain.pHeightmap[z1 * 128 + x1];
            int y21 = pOutdoor->pTerrain.pHeightmap[z1 * 128 + x2];
            int y12 = pOutdoor->pTerrain.pHeightmap[z2 * 128 + x1];
            int y22 = pOutdoor->pTerrain.pHeightmap[z2 * 128 + x2];


            /*auto norm = &pTerrainNormals[x * 128 + z];
            float dim =
                (norm->x * (float)pOutdoor->vSunlight.x / 65536.0) -
                (norm->y * (float)pOutdoor->vSunlight.y / 65536.0) -
                (norm->z * (float)pOutdoor->vSunlight.z / 65536.0);

            poly->dimming_level = 20.0 - floorf(20.0 * dim + 0.5f);*/
            poly->dimming_level = 20.0f;

            poly->uEdgeList1Size = x;
            poly->uEdgeList2Size = z;

            // draw animated water under shore
            bool water_border_tile = false;
            if (tile->IsWaterBorderTile())
            {
                glDepthMask(GL_FALSE);
                {
                    poly->texture = this->hd_water_tile_anim[this->hd_water_current_frame];
                    this->DrawTerrainPolygon(poly, true, true);

                    poly->texture = tile->GetTexture();
                }
                glDepthMask(GL_TRUE);

                water_border_tile = true;
            }

            this->DrawTerrainPolygon(poly, water_border_tile, true);
        }
    }
}


void RenderOpenGL::DrawTerrainPolygon(struct Polygon *a3, bool transparent, bool clampAtTextureBorders)
{
    auto texture = (TextureOpenGL *)a3->texture;

    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    // clamping doesnt really help here in opengl so had to alter texture coordinates a bit
    float clamp_fix_u = 1.0f / texture->GetWidth();
    float clamp_fix_v = 1.0f / texture->GetHeight();
    if (clampAtTextureBorders)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (transparent)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float dim = 1.0f;// a3->dimming_level / 20.0f;
    int x1 = a3->uEdgeList1Size;
    int z1 = a3->uEdgeList2Size;
    int x2 = x1 + 1;
    int z2 = z1 + 1;

    int y11 = pOutdoor->pTerrain.pHeightmap[z1 * 128 + x1];
    int y21 = pOutdoor->pTerrain.pHeightmap[z1 * 128 + x2];
    int y12 = pOutdoor->pTerrain.pHeightmap[z2 * 128 + x1];
    int y22 = pOutdoor->pTerrain.pHeightmap[z2 * 128 + x2];

    glBegin(GL_TRIANGLES);
    {
        glTexCoord2f(0.0f + clamp_fix_u, 0.0f + clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x1 - 64) * terrain_block_scale, y11 * terrain_height_scale, (64 - z1) * terrain_block_scale);

        glTexCoord2f(0.0f + clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x1 - 64) * terrain_block_scale, y12 * terrain_height_scale, (64 - z2) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y22 * terrain_height_scale, (64 - z2) * terrain_block_scale);

        // ---

        glTexCoord2f(0.0f + clamp_fix_u, 0.0f + clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x1 - 64) * terrain_block_scale, y11 * terrain_height_scale, (64 - z1) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y22 * terrain_height_scale, (64 - z2) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 0.0f + clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y21 * terrain_height_scale, (64 - z1) * terrain_block_scale);
    }
    glEnd();

    if (transparent)
    {
        glDisable(GL_BLEND);
    }
}







void RenderOpenGL::DrawOutdoorSkyD3D()
{
    int v9; // eax@4
    int v10; // ebx@4
    int v13; // edi@6
    int v14; // ecx@6
    int v15; // eax@8
    int v16; // eax@12
    signed __int64 v17; // qtt@13
    signed int v18; // ecx@13
    struct Polygon pSkyPolygon; // [sp+14h] [bp-150h]@1
    int v30; // [sp+134h] [bp-30h]@1
    int v32; // [sp+13Ch] [bp-28h]@6
    int v33; // [sp+140h] [bp-24h]@2
    signed __int64 v34; // [sp+144h] [bp-20h]@1
    int v35; // [sp+148h] [bp-1Ch]@4
    int v36; // [sp+14Ch] [bp-18h]@2
    int v37; // [sp+154h] [bp-10h]@8
    int v38; // [sp+158h] [bp-Ch]@1
    int v39; // [sp+15Ch] [bp-8h]@4

    v30 = (signed __int64)((double)(pODMRenderParams->int_fov_rad * pIndoorCameraD3D->vPartyPos.z)
        / ((double)pODMRenderParams->int_fov_rad + 8192.0)
        + (double)(pViewport->uScreenCenterY));
    v34 = cos((double)pIndoorCameraD3D->sRotationX * 0.0030664064) * (double)pODMRenderParams->shading_dist_mist;
    v38 = (signed __int64)((double)(pViewport->uScreenCenterY)
        - (double)pODMRenderParams->int_fov_rad
        / (v34 + 0.0000001)
        * (sin((double)pIndoorCameraD3D->sRotationX * 0.0030664064)
            * -(double)pODMRenderParams->shading_dist_mist
            - (double)pIndoorCameraD3D->vPartyPos.z));
    pSkyPolygon.Create_48607B(&stru_8019C8);//заполняется ptr_38
    pSkyPolygon.ptr_38->_48694B_frustum_sky();

    //if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
    //pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
    //else
    //pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
    //pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ? (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);
    pSkyPolygon.texture = pOutdoor->sky_texture;
    if (pSkyPolygon.texture)
    {
        pSkyPolygon.dimming_level = 0;
        pSkyPolygon.uNumVertices = 4;
        //centering(центруем)-----------------------------------------------------------------
        pSkyPolygon.v_18.x = -stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX + 16);
        pSkyPolygon.v_18.y = 0;
        pSkyPolygon.v_18.z = -stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX + 16);

        //sky wiew position(положение неба на экране)------------------------------------------
        //                X
        // 0._____________________________.3
        //  |8,8                    468,8 |
        //  |                             |
        //  |                             |
        // Y|                             |
        //  |                             |
        //  |8,351                468,351 |
        // 1._____________________________.2
        // 
        VertexRenderList[0].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;//8
        VertexRenderList[0].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;//8

        VertexRenderList[1].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;//8
        VertexRenderList[1].vWorldViewProjY = (double)v38;//247

        VertexRenderList[2].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;//468
        VertexRenderList[2].vWorldViewProjY = (double)v38;//247

        VertexRenderList[3].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;//468
        VertexRenderList[3].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;//8

        pSkyPolygon.sTextureDeltaU = 224 * pMiscTimer->uTotalGameTimeElapsed;//7168
        pSkyPolygon.sTextureDeltaV = 224 * pMiscTimer->uTotalGameTimeElapsed;//7168

        pSkyPolygon.field_24 = 0x2000000;//maybe attributes
        v33 = 65536 / (signed int)(signed __int64)(((double)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) / 2) / tan(0.6457717418670654) + 0.5);
        for (uint i = 0; i < pSkyPolygon.uNumVertices; ++i)
        {
            //rotate skydome(вращение купола неба)--------------------------------------
            // В игре принята своя система измерения углов. Полный угол (180). Значению угла 0 соответствует 
            // направление на север и/или юг (либо на восток и/или запад), значению 65536 еденицам(0х10000) соответствует угол 90.
            // две переменные хранят данные по углу обзора. field_14 по западу и востоку. field_20 по югу и северу
            // от -25080 до 25080
            v39 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
            v35 = v39 + pSkyPolygon.ptr_38->angle_from_north;

            v39 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_north_south, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.f)));
            v36 = v39 + pSkyPolygon.ptr_38->angle_from_east;

            v9 = fixpoint_mul(pSkyPolygon.v_18.z, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
            v10 = pSkyPolygon.v_18.x + v9;
            if (v10 > 0)
                v10 = 0;
            v13 = v33 * (pViewport->uScreenCenterX - (signed __int64)VertexRenderList[i].vWorldViewProjX);
            v34 = -pSkyPolygon.field_24;
            v32 = (signed __int64)VertexRenderList[i].vWorldViewProjY - 1.0;
            v14 = v33 * (v30 - v32);
            while (1)
            {
                if (v10)
                {
                    v37 = abs((int)v34 >> 14);
                    v15 = abs(v10);
                    if (v37 <= v15 || v32 <= (signed int)pViewport->uViewportTL_Y)
                    {
                        if (v10 <= 0)
                            break;
                    }
                }
                v16 = fixpoint_mul(pSkyPolygon.v_18.z, v14);
                --v32;
                v14 += v33;
                v10 = pSkyPolygon.v_18.x + v16;
            }
            HEXRAYS_LODWORD(v17) = HEXRAYS_LODWORD(v34) << 16;
            HEXRAYS_HIDWORD(v17) = v34 >> 16;
            v18 = v17 / v10;
            if (v18 < 0)
                v18 = pODMRenderParams->shading_dist_mist;
            v37 = v35 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_west, v13);
            v35 = 224 * pMiscTimer->uTotalGameTimeElapsed + ((signed int)fixpoint_mul(v37, v18) >> 3);
            VertexRenderList[i].u = (double)v35 / (2 * (double)pSkyPolygon.texture->GetWidth() * 65536.0);

            v36 = v36 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_south, v13);
            v35 = 224 * pMiscTimer->uTotalGameTimeElapsed + ((signed int)fixpoint_mul(v36, v18) >> 3);
            VertexRenderList[i].v = (double)v35 / (2 * (double)pSkyPolygon.texture->GetHeight() * 65536.0);

            VertexRenderList[i].vWorldViewPosition.x = (double)pODMRenderParams->shading_dist_mist;
            VertexRenderList[i]._rhw = 1.0 / (double)(v18 >> 16);
        }


        _set_ortho_projection();
        _set_ortho_modelview();


        VertexRenderList[1].vWorldViewProjY = VertexRenderList[1].vWorldViewProjY + 80.0;
        VertexRenderList[2].vWorldViewProjY = VertexRenderList[2].vWorldViewProjY + 80.0;

        this->DrawOutdoorSkyPolygon(&pSkyPolygon);

        VertexRenderList[0].vWorldViewProjY = (double)v10;
        VertexRenderList[1].vWorldViewProjY = VertexRenderList[1].vWorldViewProjY + 30.0;
        VertexRenderList[2].vWorldViewProjY = VertexRenderList[2].vWorldViewProjY + 30.0;
        VertexRenderList[3].vWorldViewProjY = (double)v10;

        //this->DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}



//----- (004A2DA3) --------------------------------------------------------
void RenderOpenGL::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon)
{
    auto texture = (TextureOpenGL *)pSkyPolygon->texture;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    VertexRenderList[0].u = 0 - (float)pParty->sRotationY / 512;
    VertexRenderList[1].u = 0 - (float)pParty->sRotationY / 512;
    VertexRenderList[2].u = 1 - (float)pParty->sRotationY / 512;
    VertexRenderList[3].u = 1 - (float)pParty->sRotationY / 512;

    if (pParty->sRotationX > 0)
    {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationX / 1024;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationX / 1024;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationX / 1024;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationX / 1024;
    }
    else
    {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationX / 256;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationX / 256;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationX / 256;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationX / 256;
    }

    glBegin(GL_QUADS);
    {
        for (int i = 0; i < pSkyPolygon->uNumVertices; ++i)
        {
            unsigned int diffuse = ::GetActorTintColor(31, 0, VertexRenderList[i].vWorldViewPosition.x, 1, 0);

            glColor4f(
                ((diffuse >> 16) & 0xFF) / 255.0f,
                ((diffuse >> 8) & 0xFF) / 255.0f,
                (diffuse & 0xFF) / 255.0f,
                1.0f
            );

            glTexCoord2f(
                VertexRenderList[i].u,
                /*max_v*/ -VertexRenderList[i].v
            );

            glVertex3f(
                VertexRenderList[i].vWorldViewProjX,
                VertexRenderList[i].vWorldViewProjY,
                -0.99989998 // z is negative in OpenGL
            );
        }
    }
    glEnd();
}









void RenderOpenGL::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene()
{
    pEngine->draw_debug_outlines();
    this->DoRenderBillboards_D3D();
    pEngine->GetSpellFxRenderer()->RenderSpecialEffects();
}


//----- (004A1C1E) --------------------------------------------------------
void RenderOpenGL::DoRenderBillboards_D3D()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    _set_ortho_projection();
    _set_ortho_modelview();
    //_set_3d_projection_matrix();
    //_set_3d_modelview_matrix();

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i)
    {
        if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend)
        {
            SetBillboardBlendOptions(pBillboardRenderListD3D[i].opacity);
        }

        auto texture = (TextureOpenGL *)pBillboardRenderListD3D[i].texture;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

        glBegin(GL_TRIANGLE_FAN);
        {
            auto billboard = &pBillboardRenderListD3D[i];
            auto b = &pBillboardRenderList[i];

            // since OpenGL 1.0 can't mirror texture borders, we should offset UV to avoid black edges
            billboard->pQuads[0].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[0].texcoord.y += 0.5f / texture->GetHeight();
            billboard->pQuads[1].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[1].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[2].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[2].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[3].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[3].texcoord.y += 0.5f / texture->GetHeight();

            for (unsigned int j = 0; j < billboard->uNumVertices; ++j)
            {
                glColor4f(
                    ((billboard->pQuads[j].diffuse >> 16) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 8) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f
                );

                glTexCoord2f(
                    billboard->pQuads[j].texcoord.x,
                    //-billboard->pQuads[j].texcoord.y
                    billboard->pQuads[j].texcoord.y
                );

                glVertex3f(
                    //b->world_x + (billboard->pQuads[j].texcoord.x - 0.5f) * b->pSpriteFrame->scale.GetFloat(),
                    //b->world_z + (billboard->pQuads[j].texcoord.y - 0.0f) * b->pSpriteFrame->scale.GetFloat(),
                    //b->world_y
                    billboard->pQuads[j].pos.x /** billboard->pQuads[j].rhw*/,
                    billboard->pQuads[j].pos.y /** billboard->pQuads[j].rhw*/,
                    billboard->pQuads[j].pos.z /** billboard->pQuads[j].rhw*/
                    //1.0f/billboard->pQuads[j].rhw
                );
            }
        }
        glEnd();
    }
    uNumBillboardsToDraw = 0;

    if (bFogEnabled)
    {
        bFogEnabled = false;
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP);

        GLfloat fog_color[] =
        {
            ((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
            ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
            ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f,
            1.0f
        };
        glFogfv(GL_FOG_COLOR, fog_color);
    }


    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}


//----- (004A1DA8) --------------------------------------------------------
void RenderOpenGL::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1)
{
    switch (a1)
    {
        case RenderBillboardD3D::Transparent:
        {
            if (bFogEnabled)
            {
                bFogEnabled = false;
                glEnable(GL_FOG);
                glFogi(GL_FOG_MODE, GL_EXP);

                GLfloat fog_color[] =
                {
                    ((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f,
                    1.0f
                };
                glFogfv(GL_FOG_COLOR, fog_color);
            }

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;

        case RenderBillboardD3D::Opaque_1:
        case RenderBillboardD3D::Opaque_2:
        case RenderBillboardD3D::Opaque_3:
        {
            if (bUsingSpecular)
            {
                if (!bFogEnabled)
                {
                    bFogEnabled = true;
                    glDisable(GL_FOG);
                }
            }

            glBlendFunc(GL_ONE, GL_ZERO);
        }
        break;

        default:
            logger->Warning(L"SetBillboardBlendOptions: invalid opacity type (%u)", a1);
            assert(false);
            break;
    }
}




void RenderOpenGL::TransformBillboardsAndSetPalettesODM()
{
    SoftwareBillboard billboard; // [sp+4h] [bp-60h]@1

    billboard.sParentBillboardID = -1;
    billboard.pTarget = render->pTargetSurface;
    billboard.pTargetZ = render->pActiveZBuffer;
    billboard.uTargetPitch = render->uTargetSurfacePitch;
    billboard.uViewportX = pViewport->uViewportTL_X;
    billboard.uViewportY = pViewport->uViewportTL_Y;
    billboard.uViewportZ = pViewport->uViewportBR_X - 1;
    billboard.uViewportW = pViewport->uViewportBR_Y;
    pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;

    for (unsigned int i = 0; i < ::uNumBillboardsToDraw; ++i)
    {
        auto p = &pBillboardRenderList[i];

        if (p->hwsprite)
        {
            billboard.screen_space_x = p->screen_space_x;
            billboard.screen_space_y = p->screen_space_y;
            billboard.screen_space_z = p->screen_space_z;
            billboard.sParentBillboardID = i;
            billboard.screenspace_projection_factor_x = p->screenspace_projection_factor_x;
            billboard.screenspace_projection_factor_y = p->screenspace_projection_factor_y;
            billboard.sTintColor = p->sTintColor;
            billboard.object_pid = p->object_pid;
            billboard.uFlags = p->field_1E;

            TransformBillboard(
                &billboard,
                p
            );
        }
    }
}



//----- (004A4023) --------------------------------------------------------
void RenderOpenGL::TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard)
{
    unsigned int v8; // esi@2
    double v14; // st6@14
    double v15; // st5@14
    float v29; // [sp+28h] [bp-8h]@5
    float v30; // [sp+2Ch] [bp-4h]@5


    Sprite *pSprite = pBillboard->hwsprite;
    int dimming_level = pBillboard->dimming_level;

    v8 = Billboard_ProbablyAddToListAndSortByZOrder(a2->screen_space_z);

    v30 = a2->screenspace_projection_factor_x.GetFloat();
    v29 = a2->screenspace_projection_factor_y.GetFloat();

    unsigned int diffuse = ::GetActorTintColor(dimming_level, 0, a2->screen_space_z, 0, pBillboard);
    if (a2->sTintColor & 0x00FFFFFF && bTinting)
    {
        diffuse = BlendColors(a2->sTintColor, diffuse);
        if (a2->sTintColor & 0xFF000000)
            diffuse = 0x007F7F7F & ((unsigned int)diffuse >> 1);
    }

    unsigned int specular = 0;
    if (bUsingSpecular)
        specular = sub_47C3D7_get_fog_specular(0, 0, a2->screen_space_z);

    v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
    if (a2->uFlags & 4)
        v14 *= -1.0;
    pBillboardRenderListD3D[v8].pQuads[0].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[0].pos.x = (double)a2->screen_space_x - v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[0].pos.y = (double)a2->screen_space_y - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[0].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[0].rhw = 1.0 / a2->screen_space_z;
    pBillboardRenderListD3D[v8].pQuads[0].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[0].texcoord.x = 0.0;
    pBillboardRenderListD3D[v8].pQuads[0].texcoord.y = 0.0;

    v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
    if (a2->uFlags & 4)
        v14 = v14 * -1.0;
    pBillboardRenderListD3D[v8].pQuads[1].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[1].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[1].pos.x = (double)a2->screen_space_x - v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[1].pos.y = (double)a2->screen_space_y - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[1].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[1].rhw = 1.0 / a2->screen_space_z;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.y = 1.0;

    v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
    if (a2->uFlags & 4)
        v14 *= -1.0;
    pBillboardRenderListD3D[v8].pQuads[2].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[2].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[2].pos.x = (double)a2->screen_space_x + v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[2].pos.y = (double)a2->screen_space_y - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[2].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[2].rhw = 1.0 / a2->screen_space_z;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.y = 1.0;

    v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
    if (a2->uFlags & 4)
        v14 *= -1.0;
    pBillboardRenderListD3D[v8].pQuads[3].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[3].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[3].pos.x = (double)a2->screen_space_x + v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[3].pos.y = (double)a2->screen_space_y - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[3].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[3].rhw = 1.0 / a2->screen_space_z;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.y = 0.0;

    pBillboardRenderListD3D[v8].uNumVertices = 4;
    pBillboardRenderListD3D[v8].texture = pSprite->texture;
    pBillboardRenderListD3D[v8].z_order = a2->screen_space_z;
    pBillboardRenderListD3D[v8].field_90 = a2->field_44;
    pBillboardRenderListD3D[v8].object_pid = a2->object_pid;
    pBillboardRenderListD3D[v8].screen_space_z = a2->screen_space_z;
    pBillboardRenderListD3D[v8].sParentBillboardID = a2->sParentBillboardID;

    if (a2->sTintColor & 0xFF000000)
        pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Opaque_3;
    else
        pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Transparent;
}



//----- (004A1B22) --------------------------------------------------------
unsigned int RenderOpenGL::Billboard_ProbablyAddToListAndSortByZOrder(float z)
{
    unsigned int v7; // edx@6

    if (uNumBillboardsToDraw >= 999)
        return 0;
    if (!uNumBillboardsToDraw)
    {
        uNumBillboardsToDraw = 1;
        return 0;
    }

    for (int left = 0, right = uNumBillboardsToDraw; left < right; ) // binsearch
    {
        v7 = left + (right - left) / 2;
        if (z <= render->pBillboardRenderListD3D[v7].z_order)
            right = v7;
        else
            left = v7 + 1;
    }

    if (z > render->pBillboardRenderListD3D[v7].z_order)
    {
        if (v7 == render->uNumBillboardsToDraw - 1)
            v7 = render->uNumBillboardsToDraw;
        else
        {
            if ((signed int)render->uNumBillboardsToDraw > (signed int)v7)
            {
                for (uint i = 0; i < render->uNumBillboardsToDraw - v7; i++)
                {
                    memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
                        &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
                        sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
                }
            }
            ++v7;
        }
        uNumBillboardsToDraw++;
        return v7;
    }
    if (z <= render->pBillboardRenderListD3D[v7].z_order)
    {
        if ((signed int)render->uNumBillboardsToDraw > (signed int)v7)
        {
            for (uint i = 0; i < render->uNumBillboardsToDraw - v7; i++)
            {
                memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
                    &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
                    sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
            }
        }
        uNumBillboardsToDraw++;
        return v7;
    }
    return v7;
}














//----- (004A0E97) --------------------------------------------------------
void RenderOpenGL::SetRasterClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW)
{
    this->raster_clip_x = uX;
    this->raster_clip_y = uY;
    this->raster_clip_z = uZ;
    this->raster_clip_w = uW;
}

void RenderOpenGL::PackScreenshot(unsigned int width, unsigned int height, void *out_data, unsigned int data_size, unsigned int *screenshot_size)
{
    /*auto pixels = MakeScreenshot(150, 112);
    PackPCXpicture(pixels, 150, 112, data, 1000000, out_screenshot_size);
    free(pixels);*/
}

void RenderOpenGL::SetUIClipRect(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
{
    this->clip_x = x;
    this->clip_y = y;
    this->clip_z = z;
    this->clip_w = w;
    glScissor(x, y, z, w);
}

void RenderOpenGL::ResetUIClipRect()
{
    this->SetUIClipRect(0, 0, this->window->GetWidth(), this->window->GetHeight());
}

void RenderOpenGL::PresentBlackScreen()
{
    __debugbreak();
}

void RenderOpenGL::BeginScene() {}
void RenderOpenGL::EndScene() {}

void RenderOpenGL::ClearTarget(unsigned int uColor)
{
    __debugbreak();
}

void RenderOpenGL::ClearZBuffer(int, int)
{
    memset32(this->pActiveZBuffer, -65536, 0x4B000);
}




void RenderOpenGL::DrawTextureAlphaNew(float u, float v, Image *img)
{
    int uHeight; // ebx@4
    unsigned int v11; // edx@9
    unsigned int v12; // esi@12
    unsigned int v13; // esi@15
    unsigned int v15; // esi@18
                      //unsigned __int8 *v19; // [sp+18h] [bp-8h]@4
    int uWidth; // [sp+1Ch] [bp-4h]@4

    if (!img)
        return;

    uHeight = img->GetHeight();
    ///v19 = pTexture->paletted_pixels;
    uWidth = img->GetWidth();

    auto pixels = (const unsigned __int32 *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    int uX = u * 640.0f;
    int uY = v * 480.0f;
    int clipped_out_x = uX;
    int clipped_out_y = uY;
    //if (this->bClip)
    {
        if ((signed int)uX < (signed int)this->clip_x)
        {
            pixels += this->clip_x - uX;
            uWidth += uX - this->clip_x;
            clipped_out_x = clip_x;
        }

        uHeight = img->GetHeight();
        if ((signed int)uY < (signed int)this->clip_y)
        {
            pixels += img->GetWidth() * (this->clip_y - uY);
            uHeight = uY - this->clip_y + img->GetHeight();
            clipped_out_y = clip_y;
        }
        v11 = this->clip_x;
        if ((signed int)this->clip_x < (signed int)uX)
            v11 = uX;

        if ((signed int)(v11 + uWidth) >(signed int)this->clip_z)
        {
            v12 = this->clip_x;
            if ((signed int)this->clip_x < (signed int)uX)
                v12 = uX;
            uWidth = this->clip_z - v12;
        }
        v13 = this->clip_y;
        if ((signed int)this->clip_y < (signed int)uY)
            v13 = uY;

        if ((signed int)(uHeight + v13) >(signed int)this->clip_w)
        {
            v15 = this->clip_y;
            if ((signed int)this->clip_y < (signed int)uY)
                v15 = uY;
            uHeight = this->clip_w - v15;
        }
    }

    for (int y = 0; y < uHeight; ++y)
    {
        for (int x = 0; x < uWidth; ++x)
        {
            if (*pixels & 0xFF000000)
                WritePixel16(
                    clipped_out_x + x,
                    clipped_out_y + y,
                    Color16(
                        (*pixels >> 16) & 0xFF,
                        (*pixels >> 8) & 0xFF,
                        *pixels & 0xFF
                        )
                    );
            ++pixels;
        }
        pixels += img->GetWidth() - uWidth;
    }
}


void RenderOpenGL::DrawTextureNew(float u, float v, Image *tex)
{
    DrawTextureCustomHeight(u, v, tex, tex->GetHeight());
}


void RenderOpenGL::DrawTextureCustomHeight(float u, float v, class Image *img, int custom_height)
{
    unsigned __int16 *v6; // esi@3
    unsigned int v8; // eax@5
    unsigned int v11; // eax@7
    unsigned int v12; // ebx@8
    unsigned int v15; // eax@14
    int v19; // [sp+10h] [bp-8h]@3

    if (!img)
        return;

    unsigned int uOutX = 640 * u;
    unsigned int uOutY = 480 * v;

    int width = img->GetWidth();
    int height = min(img->GetHeight(), custom_height);
    v6 = (unsigned __int16 *)img->GetPixels(IMAGE_FORMAT_R5G6B5);

    //v5 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v19 = width;
    //if (this->bClip)
    {
        if ((signed int)uOutX < (signed int)this->clip_x)
        {
            v8 = this->clip_x - uOutX;
            unsigned int v9 = uOutX - this->clip_x;
            v8 *= 2;
            width += v9;
            v6 = (unsigned __int16 *)((char *)v6 + v8);
            //v5 = (unsigned __int16 *)((char *)v5 + v8);
        }
        if ((signed int)uOutY < (signed int)this->clip_y)
        {
            v11 = this->clip_y - uOutY;
            v6 += v19 * v11;
            height += uOutY - this->clip_y;
            //v5 += this->uTargetSurfacePitch * v11;
        }
        v12 = max(this->clip_x, uOutX);
        if ((signed int)(width + v12) >(signed int)this->clip_z)
        {
            width = this->clip_z - max(this->clip_x, uOutX);
        }
        v15 = max(this->clip_y, uOutY);
        if ((signed int)(v15 + height) > (signed int)this->clip_w)
        {
            height = this->clip_w - max(this->clip_y, uOutY);
        }
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            WritePixel16(uOutX + x, uOutY + y, *v6);
            //*v5 = *v6;
            //++v5;
            ++v6;
        }
        v6 += v19 - width;
        //v5 += this->uTargetSurfacePitch - v4;
    }
}

void RenderOpenGL::DrawText(signed int uOutX, signed int uOutY, unsigned __int8 *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned __int16 *pFontPalette, unsigned __int16 uFaceColor, unsigned __int16 uShadowColor)
{
    unsigned int v9; // edi@2
    unsigned int v10; // esi@2
    unsigned int v12; // ebx@3
                      //signed int v13; // edx@5
    int v14; // edx@6
    signed int v15; // ebx@7
                    //unsigned int v16; // edx@9
    signed int v17; // edi@10
    signed int v18; // ebx@13
    unsigned int v19; // edx@15
    signed int v20; // esi@16
    unsigned __int16 v22; // dx@24
    unsigned __int8 *v24; // [sp+Ch] [bp-4h]@2

    v9 = uCharWidth;
    v10 = uCharHeight;
    //v11 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v24 = pFontPixels;

    int clipped_out_x = uOutX, clipped_out_y = uOutY;
    //if (this->bClip)
    {
        v12 = this->clip_x;
        if (uOutX < (signed int)v12)
        {
            v24 = &pFontPixels[v12 - uOutX];
            //v11 += v12 - uOutX;
            clipped_out_x = clip_x;
            v9 = uCharWidth + uOutX - v12;
        }
        //v13 = this->uClipY;
        if (uOutY < this->clip_y)
        {
            v14 = this->clip_y - uOutY;
            v24 += uCharWidth * v14;
            v10 = uCharHeight + uOutY - this->clip_y;
            //v11 += this->uTargetSurfacePitch * v14;
            clipped_out_y = clip_y;
        }
        v15 = this->clip_x;
        if (this->clip_x < uOutX)
            v15 = uOutX;
        //v16 = this->uClipZ;
        if ((signed int)(v9 + v15) >(signed int)this->clip_z)
        {
            v17 = this->clip_x;
            if (this->clip_x < uOutX)
                v17 = uOutX;
            v9 = this->clip_z - v17;
        }
        v18 = this->clip_y;
        if (this->clip_y < uOutY)
            v18 = uOutY;
        v19 = this->clip_w;
        if ((signed int)(v10 + v18) >(signed int)v19)
        {
            v20 = this->clip_y;
            if (this->clip_y < uOutY)
                v20 = uOutY;
            v10 = v19 - v20;
        }
    }

    for (uint y = 0; y < v10; ++y)
    {
        for (uint x = 0; x < v9; ++x)
        {
            if (*v24)
            {
                v22 = uShadowColor;
                if (*v24 != 1)
                    v22 = uFaceColor;
                WritePixel16(clipped_out_x + x, clipped_out_y + y, v22);
            }
            ++v24;
        }
        v24 += uCharWidth - v9;
    }
}

void RenderOpenGL::DrawTextAlpha(int x, int y, unsigned char* font_pixels, int a5, unsigned int uFontHeight, unsigned __int16 *pPalette, bool present_time_transparency)
{
    int v8; // edi@2
    unsigned int v9; // esi@2
    unsigned char *v11; // edx@2
    int v14; // edx@6
    signed int v15; // ebx@7
    signed int v17; // edi@10
    signed int v18; // ebx@13
    signed int v20; // esi@16
    unsigned __int16 v24; // si@35
    int v25; // [sp+Ch] [bp-4h]@2
    unsigned int v28; // [sp+20h] [bp+10h]@30

    int a2 = x;
    int a3 = y;
    uint a6 = uFontHeight;

    v8 = a5;
    v9 = a6;
    //v10 = &pTargetSurface[x + y * uTargetSurfacePitch];
    v11 = (unsigned char *)font_pixels;
    v25 = (int)font_pixels;
    int clipped_out_x = x;
    int clipped_out_y = y;
    //if (this->bClip)
    {
        if (a2 < (signed int)this->clip_x)
        {
            v25 = this->clip_x - a2 + (int)font_pixels;
            //v10 += v12 - a2;
            v8 = a5 + a2 - this->clip_x;
            clipped_out_x = clip_x;
        }
        if (a3 < this->clip_y)
        {
            v14 = this->clip_y - a3;
            v25 += a5 * v14;
            v9 = a6 + a3 - this->clip_y;
            //v10 += this->uTargetSurfacePitch * v14;
            clipped_out_y = clip_y;
        }
        v15 = this->clip_x;
        if (this->clip_x < a2)
            v15 = a2;
        if (v8 + v15 >(signed int)this->clip_z)
        {
            v17 = this->clip_x;
            if (v17 < a2)
                v17 = a2;
            v8 = this->clip_z - v17;
        }
        v18 = this->clip_y;
        if (this->clip_y < a3)
            v18 = a3;
        if ((signed int)(v9 + v18) >(signed int)this->clip_w)
        {
            v20 = this->clip_y;
            if (this->clip_y < a3)
                v20 = a3;
            v9 = this->clip_w - v20;
        }
        v11 = (unsigned char *)v25;
    }

    if (present_time_transparency)
    {
        v28 = 0x7FF; // transparent color 16bit render->uTargetGMask | render->uTargetBMask;
        for (uint dy = 0; dy < v9; ++dy)
        {
            for (int dx = 0; dx < v8; ++dx)
            {
                if (*v11)
                    v24 = pPalette[*v11];
                else
                    v24 = v28;
                WritePixel16(clipped_out_x + dx, clipped_out_y + dy, v24);
                ++v11;

            }
            v11 += a5 - v8;
        }
    }
    else
    {
        for (uint dy = 0; dy < v9; ++dy)
        {
            for (int dx = 0; dx < v8; ++dx)
            {
                if (*v11)
                    WritePixel16(clipped_out_x + dx, clipped_out_y + dy, pPalette[*v11]);
                ++v11;
            }
            v11 += a5 - v8;
        }
    }
}



void RenderOpenGL::Present()
{
    glEnable(GL_TEXTURE_2D);

    static GLuint screen_quad_id = 0;
    if (!screen_quad_id)
    {
        glGenTextures(1, &screen_quad_id);
		glBindTexture(GL_TEXTURE_2D, screen_quad_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    {
        int width = window->GetWidth();
        int pitch = 4 * width;
        int alpha_blended_size = pitch * window->GetHeight();

        auto alpha_blended = new unsigned char[alpha_blended_size];
        memset(alpha_blended, 0, alpha_blended_size);

        auto src = (unsigned __int32 *)this->render_target_rgb;
        auto dst = (unsigned __int32 *)alpha_blended;
        for (uint y = 0; y < 8; ++y)
        {
            memcpy(
                dst + y * width,
                src + y * width,
                width * sizeof(__int32)
            );
        }

        for (uint y = 8; y < 352; ++y)
        {
            int left_border_width_px = 8;
            memcpy(
                dst + y * width,
                src + y * width,
                left_border_width_px * sizeof(__int32)
            );
            memcpy(
                dst + left_border_width_px + game_viewport_width + y * width,
                src + left_border_width_px + game_viewport_width + y * width,
                (width - left_border_width_px - game_viewport_width) * sizeof(__int32)
            );
        }

        for (uint y = 352; y < 480; ++y)
        {
            memcpy(
                dst + y * width,
                src + y * width,
                width * sizeof(__int32)
            );
        }

        for (uint y = pViewport->uViewportTL_Y; y < pViewport->uViewportBR_Y + 1; ++y)
        {
            for (uint x = pViewport->uViewportTL_X; x < pViewport->uViewportBR_X; ++x)
            {
                if (src[x + y * width] != 0xFFF8FC00)  // F8FC00 =  Color32(Color16(g_mask | b_mask)) - alpha color key
                {
                    dst[x + y * width] = src[x + y * width];
                }
                //else
                //    dst[x + y * width] = 0x00000000;
            }
        }

		glBindTexture(GL_TEXTURE_2D, screen_quad_id);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, window->GetWidth(), window->GetHeight(),
            0, GL_RGBA, GL_UNSIGNED_BYTE, alpha_blended
        );

        delete[] alpha_blended;
    }

    _set_ortho_projection();
    _set_ortho_modelview();

    glBindTexture(GL_TEXTURE_2D, screen_quad_id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    //glTranslatef(0.5f, 0.5f, 0.0f); // texels to pixels // somehow it works against expected - makes things more blurry
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f);  glVertex2f(0.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f);  glVertex2f(0.0f, window->GetHeight());
        glTexCoord2f(1.0f, 1.0f);  glVertex2f(window->GetWidth(), window->GetHeight());
        glTexCoord2f(1.0f, 0.0f);  glVertex2f(window->GetWidth(), 0.0f);
    }
    glEnd();


    SwapBuffers((HDC)this->hdc);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}




RenderVertexSoft ogl_draw_buildings_vertices[20];
void RenderOpenGL::DrawBuildingsD3D()
{
    int v27; // eax@57
    unsigned int v34; // eax@80
    int v49; // [sp+2Ch] [bp-2Ch]@10
    int v50; // [sp+30h] [bp-28h]@34
    int v51; // [sp+34h] [bp-24h]@35
    int v52; // [sp+38h] [bp-20h]@36
    int v53; // [sp+3Ch] [bp-1Ch]@8

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    for (uint model_id = 0; model_id < pOutdoor->uNumBModels; model_id++)
    {
        int reachable;
        if (IsBModelVisible(model_id, &reachable))
        {
            pOutdoor->pBModels[model_id].field_40 |= 1;
            if (pOutdoor->pBModels[model_id].uNumFaces > 0)
            {
                for (int face_id = 0; face_id < pOutdoor->pBModels[model_id].uNumFaces; face_id++)
                {
                    if (!pOutdoor->pBModels[model_id].pFaces[face_id].Invisible())
                    {
                        v53 = 0;
                        auto poly = &array_77EC08[pODMRenderParams->uNumPolygons];

                        poly->flags = 0;
                        poly->field_32 = 0;
                        poly->texture = pOutdoor->pBModels[model_id].pFaces[face_id].GetTexture();

                        if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLUID)
                            poly->flags |= 2;
                        if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_INDOOR_SKY)
                            poly->flags |= 0x400;

                        if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_DIAGONAL)
                            poly->flags |= 0x400;
                        else if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_VERTICAL)
                            poly->flags |= 0x800;

                        if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_HORIZONTAL)
                            poly->flags |= 0x2000;
                        else if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_DONT_CACHE_TEXTURE)
                            poly->flags |= 0x1000;

                        poly->sTextureDeltaU = pOutdoor->pBModels[model_id].pFaces[face_id].sTextureDeltaU;
                        poly->sTextureDeltaV = pOutdoor->pBModels[model_id].pFaces[face_id].sTextureDeltaV;

                        unsigned int flow_anim_timer = GetTickCount() >> 4;
                        unsigned int flow_u_mod = poly->texture->GetWidth() - 1;
                        unsigned int flow_v_mod = poly->texture->GetHeight() - 1;

                        if (pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z && abs(pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z) >= 59082)
                        {
                            if (poly->flags & 0x400)
                                poly->sTextureDeltaV += flow_anim_timer & flow_v_mod;
                            if (poly->flags & 0x800)
                                poly->sTextureDeltaV -= flow_anim_timer & flow_v_mod;
                        }
                        else
                        {
                            if (poly->flags & 0x400)
                                poly->sTextureDeltaV -= flow_anim_timer & flow_v_mod;
                            if (poly->flags & 0x800)
                                poly->sTextureDeltaV += flow_anim_timer & flow_v_mod;
                        }

                        if (poly->flags & 0x1000)
                            poly->sTextureDeltaU -= flow_anim_timer & flow_u_mod;
                        else if (poly->flags & 0x2000)
                            poly->sTextureDeltaU += flow_anim_timer & flow_u_mod;

                        v50 = 0;
                        v49 = 0;

                        for (uint vertex_id = 1; vertex_id <= pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; vertex_id++)
                        {
                            array_73D150[vertex_id - 1].vWorldPosition.x = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].x;
                            array_73D150[vertex_id - 1].vWorldPosition.y = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].y;
                            array_73D150[vertex_id - 1].vWorldPosition.z = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].z;
                            array_73D150[vertex_id - 1].u = (poly->sTextureDeltaU + (signed __int16)pOutdoor->pBModels[model_id].pFaces[face_id].pTextureUIDs[vertex_id - 1]) * (1.0 / (double)poly->texture->GetWidth());
                            array_73D150[vertex_id - 1].v = (poly->sTextureDeltaV + (signed __int16)pOutdoor->pBModels[model_id].pFaces[face_id].pTextureVIDs[vertex_id - 1]) * (1.0 / (double)poly->texture->GetHeight());
                        }
                        memcpy(ogl_draw_buildings_vertices, array_73D150, sizeof(array_73D150));

                        for (uint i = 1; i <= pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; i++)
                        {
                            if (pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[0]].z == array_73D150[i - 1].vWorldPosition.z)
                                ++v53;
                            pIndoorCameraD3D->ViewTransform(&array_73D150[i - 1], 1);
                            if (array_73D150[i - 1].vWorldViewPosition.x < 8.0 || array_73D150[i - 1].vWorldViewPosition.x > pODMRenderParams->shading_dist_mist)
                            {
                                if (array_73D150[i - 1].vWorldViewPosition.x >= 8.0)
                                    v49 = 1;
                                else
                                    v50 = 1;
                            }
                            else
                                pIndoorCameraD3D->Project(&array_73D150[i - 1], 1, 0);
                        }

                        if (v53 == pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices)
                            poly->field_32 |= 1;
                        poly->pODMFace = &pOutdoor->pBModels[model_id].pFaces[face_id];
                        poly->uNumVertices = pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices;
                        poly->field_59 = 5;
                        v51 = fixpoint_mul(-pOutdoor->vSunlight.x, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.x);
                        v53 = fixpoint_mul(-pOutdoor->vSunlight.y, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.y);
                        v52 = fixpoint_mul(-pOutdoor->vSunlight.z, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z);
                        poly->dimming_level = 20 - fixpoint_mul(20, v51 + v53 + v52);
                        if (poly->dimming_level < 0)
                            poly->dimming_level = 0;
                        if (poly->dimming_level > 31)
                            poly->dimming_level = 31;
                        if (pODMRenderParams->uNumPolygons >= 1999 + 5000)
                            return;
                        if (ODMFace::IsBackfaceNotCulled(array_73D150, poly))
                        {
                            pOutdoor->pBModels[model_id].pFaces[face_id].bVisible = 1;
                            poly->uBModelFaceID = face_id;
                            poly->uBModelID = model_id;
                            v27 = 8 * (face_id | (model_id << 6));
                            v27 |= 6;
                            poly->field_50 = v27;
                            for (int vertex_id = 0; vertex_id < pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; ++vertex_id)
                            {
                                memcpy(&VertexRenderList[vertex_id], &array_73D150[vertex_id], sizeof(VertexRenderList[vertex_id]));
                                VertexRenderList[vertex_id]._rhw = 1.0 / (array_73D150[vertex_id].vWorldViewPosition.x + 0.0000001);
                            }

                            if (v50)
                            {
                                poly->uNumVertices = ODM_NearClip(pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices);
                                ODM_Project(poly->uNumVertices);
                            }
                            if (v49)
                            {
                                poly->uNumVertices = ODM_FarClip(pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices);
                                ODM_Project(poly->uNumVertices);
                            }

                            if (poly->uNumVertices)
                            {
                                if (poly->IsWater())
                                {
                                    if (poly->IsWaterAnimDisabled())
                                        poly->texture = render->hd_water_tile_anim[0];
                                    else
                                        poly->texture = render->hd_water_tile_anim[render->hd_water_current_frame];
                                }

                                render->DrawPolygon(poly);
                            }
                        }
                    }
                }
            }
        }
    }
}


void RenderOpenGL::DrawPolygon(struct Polygon *poly)
{
    auto texture = (TextureOpenGL *)poly->texture;
    auto a4 = poly->pODMFace;
    auto uNumVertices = poly->uNumVertices;

    if (poly->uNumVertices >= 3)
    {
        //v54 = pEngine->pLightmapBuilder->StationaryLightsCount;

        int a2 = 0xFFFFFFFF;
        pEngine->AlterGamma_ODM(a4, &a2);

        if (!pEngine->pLightmapBuilder->StationaryLightsCount || byte_4D864C && pEngine->uFlags & 2)
        {
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

            glBegin(GL_TRIANGLE_FAN);

            int outline_color;
            if (GetTickCount() % 300 >= 150)
                outline_color = 0xFFFF2020;
            else outline_color = 0xFF901010;

            for (uint i = 0; i < uNumVertices; ++i)
            {
                d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
                d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
                d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(poly->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
                pEngine->AlterGamma_ODM(a4, &d3d_vertex_buffer[i].diffuse);

                if (a4->uAttributes & FACE_OUTLINED)
                {
                    d3d_vertex_buffer[i].diffuse = outline_color;
                }

                if (this->bUsingSpecular)
                    d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);
                else
                    d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
                d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;

                glTexCoord2f(ogl_draw_buildings_vertices[i].u, ogl_draw_buildings_vertices[i].v);

                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    bUsingSpecular ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f : 1.0f
                );

                glVertex3f(
                    //d3d_vertex_buffer[i].pos.x /*/ d3d_vertex_buffer[i].rhw*/,
                    //d3d_vertex_buffer[i].pos.y /*/ d3d_vertex_buffer[i].rhw*/,
                    //d3d_vertex_buffer[i].pos.z /*/ d3d_vertex_buffer[i].rhw*/

                    ogl_draw_buildings_vertices[i].vWorldPosition.x,
                    ogl_draw_buildings_vertices[i].vWorldPosition.z,
                    ogl_draw_buildings_vertices[i].vWorldPosition.y
                );

            }

            glEnd();
        }
        else
        {
            /*for (uint i = 0; i < uNumVertices; ++i)
            {

                d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
                d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
                d3d_vertex_buffer[i].diffuse = GetActorTintColor(a3->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
                if (this->bUsingSpecular)
                    d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);
                else
                    d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
                d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;

            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
            if (bUsingSpecular)
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

            ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                d3d_vertex_buffer,
                uNumVertices,
                D3DDP_DONOTLIGHT));
            //v50 = (const char *)v5->pRenderD3D->pDevice;
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
            //(*(void (**)(void))(*(int *)v50 + 88))();
            pEngine->pLightmapBuilder->DrawLightmaps(-1);
            for (uint i = 0; i < uNumVertices; ++i)
            {
                d3d_vertex_buffer[i].diffuse = a2;
            }
            ErrD3D(pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture()));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
            if (!render->bUsingSpecular)
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                d3d_vertex_buffer,
                uNumVertices,
                D3DDP_DONOTLIGHT));
            if (bUsingSpecular)
            {
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

                for (uint i = 0; i < uNumVertices; ++i)
                {
                    d3d_vertex_buffer[i].diffuse = render->uFogColor | d3d_vertex_buffer[i].specular & 0xFF000000;
                    d3d_vertex_buffer[i].specular = 0;
                }

                ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA));
                ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                    D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                    d3d_vertex_buffer,
                    uNumVertices,
                    D3DDP_DONOTLIGHT));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
                //v40 = render->pRenderD3D->pDevice->lpVtbl;
                v41 = GetLevelFogColor();
                pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF);
                v6 = 0;
                pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0);
            }
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, v6));*/
        }
    }
}










bool RenderOpenGL::SwitchToWindow()
{
    pParty->uFlags |= PARTY_FLAGS_1_0002;
    pViewport->SetFOV(_6BE3A0_fov * 65536.0f);
    CreateZBuffer();

    return true;
}




bool RenderOpenGL::Initialize(OSWindow *window)
{
	pD3DBitmaps.Load(L"data\\d3dbitmap.hwl");
	pD3DSprites.Load(L"data\\d3dsprite.hwl");

    if (this->window = window)
    {
        static PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
        {
            sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
            1,											// Version Number
            PFD_DRAW_TO_WINDOW |						// Format Must Support Window
            PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
            PFD_DOUBLEBUFFER,							// Must Support Double Buffering
            PFD_TYPE_RGBA,								// Request An RGBA Format
            32,										    // Select Our Color Depth
            0, 0, 0, 0, 0, 0,							// Color Bits Ignored
            0,											// No Alpha Buffer
            0,											// Shift Bit Ignored
            0,											// No Accumulation Buffer
            0, 0, 0, 0,									// Accumulation Bits Ignored
            16,											// 16Bit Z-Buffer (Depth Buffer)  
            0,											// No Stencil Buffer
            0,											// No Auxiliary Buffer
            PFD_MAIN_PLANE,								// Main Drawing Layer
            0,											// Reserved
            0, 0, 0										// Layer Masks Ignored
        };

        HGLRC hRC;
        if (this->hdc = GetDC((HWND)window->GetApiHandle()))							// Did We Get A Device Context?
        {
            HDC hDC = (HDC)this->hdc;
            int pixel_format_id = 0;
            if (pixel_format_id = ChoosePixelFormat(hDC, &pfd))	// Did Windows Find A Matching Pixel Format?
            {
                if (SetPixelFormat(hDC, pixel_format_id, &pfd))		// Are We Able To Set The Pixel Format?
                {
                    if (hRC = wglCreateContext(hDC))				// Are We Able To Get A Rendering Context?
                    {
                        if (wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
                        {
                            glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
                            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
                            glClearDepth(1.0f);									// Depth Buffer Setup
                            glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
                            glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
                            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                            glViewport(0, 0, window->GetWidth(), window->GetHeight());  // Reset The Current Viewport

                            glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
                            glLoadIdentity();									// Reset The Projection Matrix

                                                                                // Calculate The Aspect Ratio Of The Window
                            gluPerspective(45.0f, (GLfloat)window->GetWidth() / (GLfloat)window->GetHeight(), 0.1f, 100.0f);

                            glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
                            glLoadIdentity();									// Reset The Modelview Matrix

                            SwapBuffers(hDC);				                    // Swap Buffers (Double Buffering)

                            this->clip_x = this->clip_y = 0;
                            this->clip_z = window->GetWidth();
                            this->clip_w = window->GetHeight();
                            this->render_target_rgb = new unsigned char[4 * window->GetWidth() * window->GetHeight()];

							/*while (1)
							{
								MSG msg;
								while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
								{
									TranslateMessage(&msg);
									DispatchMessage(&msg);
								}

								glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
								glClearDepth(1.0f);
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

								glMatrixMode(GL_PROJECTION);
								glLoadIdentity();
								gluPerspective(45.0f, (GLfloat)window->GetWidth() / (GLfloat)window->GetHeight(), 0.1f, 100.0f);
								glMatrixMode(GL_MODELVIEW);
								glLoadIdentity();
								gluLookAt(0, 0.7, 3, 0, 0, -1, 0, 1, 0);
								//glTranslatef(0.0f, 0.0f, -3.0f);

								glBegin(GL_TRIANGLES);
								{
									glColor3f(0.0f, 0.0f, 1.0f);
									glVertex3f(0.0f, 1.0f, 0.0f);

									glColor3f(0.0f, 1.0f, 0.0f);
									glVertex3f(-1.0f, -1.0f, 0.0f);

									glColor3f(1.0f, 0.0f, 0.0f);
									glVertex3f(1.0f, -1.0f, 0.0f);
								}
								glEnd();

								SwapBuffers(hDC);
							}*/
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


HWLTexture *RenderOpenGL::LoadHwlBitmap(const char *name)
{
    return pD3DBitmaps.LoadTexture(name, 0);
}

HWLTexture *RenderOpenGL::LoadHwlSprite(const char *name)
{
    return pD3DSprites.LoadTexture(name, 0);
}

void RenderOpenGL::WritePixel16(int x, int y, unsigned __int16 color)
{
    int idx = 4 * (y * window->GetWidth() + x);
    this->render_target_rgb[idx + 0] = 8 * ((color >> 11) & 0x1F);
    this->render_target_rgb[idx + 1] = 4 * ((color >> 5) & 0x3F);
    this->render_target_rgb[idx + 2] = 8 * (color & 0x1F);
    this->render_target_rgb[idx + 3] = 0xFF;
}

unsigned __int16 RenderOpenGL::ReadPixel16(int x, int y)
{
    int idx = 4 * (y * window->GetWidth() + x);
    return Color16(
        this->render_target_rgb[idx + 0],
        this->render_target_rgb[idx + 1],
        this->render_target_rgb[idx + 2]
    );
}

void RenderOpenGL::UnlockSurface(Texture *texture)
{
    __debugbreak();
}

bool RenderOpenGL::LockSurface(Texture *texture, Rect *, void **out_surface, int *out_pitch, int *out_width, int *out_height)
{
    __debugbreak();
    return false;
}


//----- (004A6D87) --------------------------------------------------------
void RenderOpenGL::FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, unsigned int uColor16)
{
    for (uint y = uY; y < uY + uHeight; ++y)
    {
        for (uint x = uX; x < uX + uWidth; ++x)
        {
            this->WritePixel16(x, y, uColor16);
        }
    }
}