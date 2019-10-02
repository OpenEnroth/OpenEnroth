#ifdef _WINDOWS
    #pragma comment(lib, "opengl32.lib")
    #pragma comment(lib, "glu32.lib")

    //  on windows, this is required in gl/glu.h
    #if !defined(APIENTRY)
        #define APIENTRY __stdcall
    #endif

    #if !defined(WINGDIAPI)
        #define WINGDIAPI
    #endif

    #if !defined(CALLBACK)
        #define CALLBACK __stdcall
    #endif
#endif

#include <gl/glu.h>
#include <gL/gl.h>


#include <SDL.h>
#include <SDL_opengl_glext.h>

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"

#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"

#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Objects/Actor.h"

#include "Platform/Api.h"
#include "Platform/OsWindow.h"


RenderOpenGL::RenderOpenGL()
    : RenderBase() {
}

RenderOpenGL::~RenderOpenGL() { /*__debugbreak();*/ }

void RenderOpenGL::Release() { __debugbreak(); }

void RenderOpenGL::SaveWinnersCertificate(const char *a1) { __debugbreak(); }

bool RenderOpenGL::InitializeFullscreen() {
    __debugbreak();
    return 0;
}

unsigned int RenderOpenGL::GetActorTintColor(int DimLevel, int tint, float WorldViewX, int a5, RenderBillboard *Billboard) {
    __debugbreak();
    return 0;
}


// when losing and regaining window focus - not required for OGL??
void RenderOpenGL::RestoreFrontBuffer() { /*__debugbreak();*/ }
void RenderOpenGL::RestoreBackBuffer() { /*__debugbreak();*/ }

void RenderOpenGL::BltBackToFontFast(int a2, int a3, Rect *a4) {
    __debugbreak();  // never called anywhere
}



unsigned int RenderOpenGL::GetRenderWidth() const { return window->GetWidth(); }
unsigned int RenderOpenGL::GetRenderHeight() const { return window->GetHeight(); }

void RenderOpenGL::ClearBlack() {  // used only in game over win
    ClearZBuffer(0, 479);  // dummy params
    ClearTarget(0);
}

void RenderOpenGL::ClearTarget(unsigned int uColor) {
    memset32(render_target_rgb, Color32(uColor), 0x4B000);
}

void RenderOpenGL::ClearZBuffer(int, int) {
    // parameter alwyas 0 + 479 but never used
    memset32(this->pActiveZBuffer, -65536, 0x4B000);
}

void RenderOpenGL::CreateZBuffer() {
    pActiveZBuffer = (int *)malloc(window->GetWidth() * window->GetHeight() * sizeof(int));  // 640 * 480 * 4
    memset32(pActiveZBuffer, 0xFFFF0000, 0x4B000u);      // inlined Render::ClearActiveZBuffer  (mm8::004A085B)
}

void RenderOpenGL::RasterLine2D(signed int uX, signed int uY, signed int uZ,
                                signed int uW, unsigned __int16 uColor) {
    unsigned int b = (uColor & 0x1F)*8;
    unsigned int g = ((uColor >> 5) & 0x3F)*4;
    unsigned int r = ((uColor >> 11) & 0x1F)*8;

    glDisable(GL_TEXTURE_2D);
    glLineWidth(1);
    glColor3ub(r, g, b);

    // pixel centers around 0.5 so tweak to avoid gaps and squashing
    if (uZ == uX) {
       uW += 1;
    }

    glBegin(GL_LINES);
    glVertex3f(uX, uY, 0);
    glVertex3f(uZ+.5, uW+.5, 0);
    glEnd();
}

void RenderOpenGL::BeginSceneD3D() {
    // Setup for 3D

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.9f, 0.5f, 0.1f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render->uNumBillboardsToDraw = 0;  // moved from drawbillboards - cant reset this until mouse picking finished
}

extern unsigned int BlendColors(unsigned int a1, unsigned int a2);

void RenderOpenGL::DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard,
                                        RenderBillboard *billboard) {
    int v11;     // eax@9
    int v12;     // eax@9
    double v15;  // st5@12
    double v16;  // st4@12
    double v17;  // st3@12
    double v18;  // st2@12
    int v19;     // ecx@14
    double v20;  // st3@14
    int v21;     // ecx@16
    double v22;  // st3@16
    float v27;   // [sp+24h] [bp-Ch]@5
    float v29;   // [sp+2Ch] [bp-4h]@5
    float v31;   // [sp+3Ch] [bp+Ch]@5
    float a1;    // [sp+40h] [bp+10h]@5

    // if (this->uNumD3DSceneBegins == 0) {
    //    return;
    //}

    Sprite *pSprite = billboard->hwsprite;
    int dimming_level = billboard->dimming_level;

    // v4 = pSoftBillboard;
    // v5 = (double)pSoftBillboard->zbuffer_depth;
    // pSoftBillboarda = pSoftBillboard->zbuffer_depth;
    // v6 = pSoftBillboard->zbuffer_depth;
    unsigned int v7 = Billboard_ProbablyAddToListAndSortByZOrder(
        pSoftBillboard->screen_space_z);
    // v8 = dimming_level;
    // device_caps = v7;
    int v28 = dimming_level & 0xFF000000;
    if (dimming_level & 0xFF000000) {
        pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Opaque_3;
    } else {
        pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Transparent;
    }
    // v10 = a3;
    pBillboardRenderListD3D[v7].field_90 = pSoftBillboard->field_44;
    pBillboardRenderListD3D[v7].screen_space_z = pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].object_pid = pSoftBillboard->object_pid;
    pBillboardRenderListD3D[v7].sParentBillboardID =
        pSoftBillboard->sParentBillboardID;
    // v25 = pSoftBillboard->uScreenSpaceX;
    // v24 = pSoftBillboard->uScreenSpaceY;
    a1 = pSoftBillboard->screenspace_projection_factor_x;
    v29 = pSoftBillboard->screenspace_projection_factor_y;
    v31 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v27 = (double)(pSprite->uBufferHeight - pSprite->uAreaY);
    if (pSoftBillboard->uFlags & 4) {
        v31 = v31 * -1.0;
    }
    if (config->is_tinting && pSoftBillboard->sTintColor) {
        v11 = ::GetActorTintColor(dimming_level, 0,
            pSoftBillboard->screen_space_z, 0, 0);
        v12 = BlendColors(pSoftBillboard->sTintColor, v11);
        if (v28)
            v12 =
            (unsigned int)((char *)&array_77EC08[1852].pEdgeList1[17] + 3) &
            ((unsigned int)v12 >> 1);
    } else {
        v12 = ::GetActorTintColor(dimming_level, 0,
            pSoftBillboard->screen_space_z, 0, 0);
    }
    // v13 = (double)v25;
    pBillboardRenderListD3D[v7].pQuads[0].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[0].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[0].pos.x =
        pSoftBillboard->screen_space_x - v31 * a1;
    // v14 = (double)v24;
    // v32 = v14;
    pBillboardRenderListD3D[v7].pQuads[0].pos.y =
        pSoftBillboard->screen_space_y - v27 * v29;
    v15 = 1.0 - 1.0 / (pSoftBillboard->screen_space_z * 0.061758894);
    pBillboardRenderListD3D[v7].pQuads[0].pos.z = v15;
    v16 = 1.0 / pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].pQuads[0].rhw =
        1.0 / pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.y = 0.0;
    v17 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v18 = (double)(pSprite->uBufferHeight - pSprite->uAreaY -
        pSprite->uAreaHeight);
    if (pSoftBillboard->uFlags & 4) {
        v17 = v17 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[1].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[1].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[1].pos.x =
        pSoftBillboard->screen_space_x - v17 * a1;
    pBillboardRenderListD3D[v7].pQuads[1].pos.y =
        pSoftBillboard->screen_space_y - v18 * v29;
    pBillboardRenderListD3D[v7].pQuads[1].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[1].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.y = 1.0;
    v19 = pSprite->uBufferHeight - pSprite->uAreaY - pSprite->uAreaHeight;
    v20 = (double)(pSprite->uAreaX + pSprite->uAreaWidth +
        (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if (pSoftBillboard->uFlags & 4) {
        v20 = v20 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[2].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[2].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[2].pos.x =
        v20 * a1 + pSoftBillboard->screen_space_x;
    pBillboardRenderListD3D[v7].pQuads[2].pos.y =
        pSoftBillboard->screen_space_y - (double)v19 * v29;
    pBillboardRenderListD3D[v7].pQuads[2].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[2].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.y = 1.0;
    v21 = pSprite->uBufferHeight - pSprite->uAreaY;
    v22 = (double)(pSprite->uAreaX + pSprite->uAreaWidth +
        (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if (pSoftBillboard->uFlags & 4) {
        v22 = v22 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[3].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[3].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[3].pos.x =
        v22 * a1 + pSoftBillboard->screen_space_x;
    pBillboardRenderListD3D[v7].pQuads[3].pos.y =
        pSoftBillboard->screen_space_y - (double)v21 * v29;
    pBillboardRenderListD3D[v7].pQuads[3].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[3].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.y = 0.0;
    // v23 = pSprite->pTexture;
    pBillboardRenderListD3D[v7].uNumVertices = 4;
    pBillboardRenderListD3D[v7].z_order = pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].texture = pSprite->texture;
}

void RenderOpenGL::_4A4CC9_AddSomeBillboard(
    struct SpellFX_Billboard *a1, int diffuse) {
    __debugbreak();
}

void RenderOpenGL::DrawBillboardList_BLV() {
    SoftwareBillboard soft_billboard = { 0 };
    soft_billboard.sParentBillboardID = -1;
    //  soft_billboard.pTarget = pBLVRenderParams->pRenderTarget;
    soft_billboard.pTargetZ = pBLVRenderParams->pTargetZBuffer;
    //  soft_billboard.uTargetPitch = uTargetSurfacePitch;
    soft_billboard.uViewportX = pBLVRenderParams->uViewportX;
    soft_billboard.uViewportY = pBLVRenderParams->uViewportY;
    soft_billboard.uViewportZ = pBLVRenderParams->uViewportZ - 1;
    soft_billboard.uViewportW = pBLVRenderParams->uViewportW;

    pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;
    for (uint i = 0; i < ::uNumBillboardsToDraw; ++i) {
        RenderBillboard *p = &pBillboardRenderList[i];
        if (p->hwsprite) {
            soft_billboard.screen_space_x = p->screen_space_x;
            soft_billboard.screen_space_y = p->screen_space_y;
            soft_billboard.screen_space_z = p->screen_space_z;
            soft_billboard.sParentBillboardID = i;
            soft_billboard.screenspace_projection_factor_x =
                p->screenspace_projection_factor_x;
            soft_billboard.screenspace_projection_factor_y =
                p->screenspace_projection_factor_y;
            soft_billboard.object_pid = p->object_pid;
            soft_billboard.uFlags = p->field_1E;
            soft_billboard.sTintColor = p->sTintColor;

            DrawBillboard_Indoor(&soft_billboard, p);
        }
    }
}


void RenderOpenGL::DrawProjectile(float srcX, float srcY, float a3, float a4,
                                  float dstX, float dstY, float a7, float a8,
                                  Texture *texture) {
    __debugbreak();
}
void RenderOpenGL::ScreenFade(unsigned int color, float t) { __debugbreak(); }


void RenderOpenGL::DrawTextureOffset(int pX, int pY, int move_X, int move_Y,
                                     Image *pTexture) {
    DrawTextureNew((pX - move_X)/640., (pY - move_Y)/480., pTexture);
}


void RenderOpenGL::DrawImage(Image *img, const Rect &rect) {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    bool blendFlag = 1;

    // check if loaded
    auto texture = (TextureOpenGL *)img;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    if (blendFlag) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float depth = 0;

    GLfloat Vertices[] = { (float)rect.x, (float)rect.y, depth,
        (float)rect.z, (float)rect.y, depth,
        (float)rect.z, (float)rect.w, depth,
        (float)rect.x, (float)rect.w, depth };

    GLfloat TexCoord[] = { 0, 0,
        1, 0,
        1, 1,
        0, 1 };

    GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning(L"OpenGL error: (%u)", err);
    }
}


void RenderOpenGL::ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture,
                                  int a5) {
    // __debugbreak();
    // blank in d3d
}


void RenderOpenGL::ZDrawTextureAlpha(float u, float v, Image *img, int zVal) {
    if (!img) return;

    int uOutX = u * this->window->GetWidth();
    int uOutY = v * this->window->GetHeight();
    unsigned int imgheight = img->GetHeight();
    unsigned int imgwidth = img->GetWidth();
    auto pixels = (uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (int xs = 0; xs < imgwidth; xs++) {
        for (int ys = 0; ys < imgheight; ys++) {
            if (pixels[xs + imgwidth * ys] & 0xFF000000) {
                this->pActiveZBuffer[uOutX + xs + window->GetWidth() * (uOutY + ys)] = zVal;
            }
        }
    }
}




void RenderOpenGL::BlendTextures(int a2, int a3, Image *a4, Image *a5, int t,
                                 int start_opacity, int end_opacity) {
    __debugbreak();
}


void RenderOpenGL::TexturePixelRotateDraw(float u, float v, Image *img, int time) {
    if (img) {
        auto pixelpoint = (const uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        int width = img->GetWidth();
        int height = img->GetHeight();

        int brightloc = -1;
        int brightval = 0;
        int darkloc = -1;
        int darkval = 765;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int nudge = x + y * width;
                // Test the brightness against the threshold
                int bright = (*(pixelpoint + nudge) & 0xFF) + ((*(pixelpoint + nudge) >> 8) & 0xFF) + ((*(pixelpoint + nudge) >> 16) & 0xFF);
                if (bright == 0) continue;

                if (bright > brightval) {
                    brightval = bright;
                    brightloc = nudge;
                }
                if (bright < darkval) {
                    darkval = bright;
                    darkloc = nudge;
                }
            }
        }

        // find brightest
        unsigned int bmax = (*(pixelpoint + brightloc) & 0xFF);
        unsigned int gmax = ((*(pixelpoint + brightloc) >> 8) & 0xFF);
        unsigned int rmax = ((*(pixelpoint + brightloc) >> 16) & 0xFF);

        // find darkest not black
        unsigned int bmin = (*(pixelpoint + darkloc) & 0xFF);
        unsigned int gmin = ((*(pixelpoint + darkloc) >> 8) & 0xFF);
        unsigned int rmin = ((*(pixelpoint + darkloc) >> 16) & 0xFF);

        // steps pixels
        float bstep = (bmax - bmin) / 128.;
        float gstep = (gmax - gmin) / 128.;
        float rstep = (rmax - rmin) / 128.;

        int timestep = time % 256;

        // loop through
        for (int ydraw = 0; ydraw < height; ++ydraw) {
            for (int xdraw = 0; xdraw < width; ++xdraw) {
                if (*pixelpoint) {  // check orig item not got blakc pixel
                    unsigned int bcur = (*(pixelpoint) & 0xFF);
                    unsigned int gcur = ((*(pixelpoint) >> 8) & 0xFF);
                    unsigned int rcur = ((*(pixelpoint) >> 16) & 0xFF);
                    int pixstepb = (bcur - bmin) / bstep + timestep;
                    if (pixstepb > 255) pixstepb = pixstepb - 256;
                    if (pixstepb >= 0 && pixstepb < 128)  // 0-127
                        bcur = bmin + pixstepb * bstep;
                    if (pixstepb >= 128 && pixstepb < 256) {  // 128-255
                        pixstepb = pixstepb - 128;
                        bcur = bmax - pixstepb * bstep;
                    }
                    int pixstepr = (rcur - rmin) / rstep + timestep;
                    if (pixstepr > 255) pixstepr = pixstepr - 256;
                    if (pixstepr >= 0 && pixstepr < 128)  // 0-127
                        rcur = rmin + pixstepr * rstep;
                    if (pixstepr >= 128 && pixstepr < 256) {  // 128-255
                        pixstepr = pixstepr - 128;
                        rcur = rmax - pixstepr * rstep;
                    }
                    int pixstepg = (gcur - gmin) / gstep + timestep;
                    if (pixstepg > 255) pixstepg = pixstepg - 256;
                    if (pixstepg >= 0 && pixstepg < 128)  // 0-127
                        gcur = gmin + pixstepg * gstep;
                    if (pixstepg >= 128 && pixstepg < 256) {  // 128-255
                        pixstepg = pixstepg - 128;
                        gcur = gmax - pixstepg * gstep;
                    }
                    // out pixel
                    // temppix[xdraw + ydraw * width] = (rcur << 24) | (gcur << 16) | (bcur << 8) | 0xFF;//Color32(rcur, gcur, bcur);
                    render_target_rgb[int((u*640)+xdraw + 640 *(v*480+ydraw))] = Color32(rcur, gcur, bcur);
                }
                pixelpoint++;
            }
        }
        // draw image
        // render->Update_Texture(img);
        // render->DrawTextureAlphaNew(u, v, img);
        // temp->Release();
    }
}



void RenderOpenGL::DrawMonsterPortrait(Rect rc, SpriteFrame *Portrait, int Y_Offset) {
    Rect rct;
    rct.x = rc.x + 64 + Portrait->hw_sprites[0]->uAreaX - Portrait->hw_sprites[0]->uBufferWidth / 2;
    rct.y = rc.y + Y_Offset + Portrait->hw_sprites[0]->uAreaY;
    rct.z = rct.x + Portrait->hw_sprites[0]->uAreaWidth;
    rct.w = rct.y + Portrait->hw_sprites[0]->uAreaHeight;

    render->SetUIClipRect(rc.x, rc.y, rc.z, rc.w);
    render->DrawImage(Portrait->hw_sprites[0]->texture, rct);
    render->ResetUIClipRect();
}

void RenderOpenGL::DrawTransparentRedShade(float u, float v, Image *a4) {
    DrawMasked(u, v, a4, 0, 0xF800);
}

void RenderOpenGL::DrawTransparentGreenShade(float u, float v, Image *pTexture) {
    DrawMasked(u, v, pTexture, 0, 0x07E0);
}

void RenderOpenGL::DrawFansTransparent(const RenderVertexD3D3 *vertices,
                                       unsigned int num_vertices) {
    __debugbreak();
}

void RenderOpenGL::DrawMasked(float u, float v, Image *pTexture, unsigned int color_dimming_level,
                              unsigned __int16 mask) {
    if (!pTexture) {
        return;
    }
    uint32_t width = pTexture->GetWidth();
    uint32_t *pixels = (uint32_t *)pTexture->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    // Image *temp = Image::Create(width, pTexture->GetHeight(), IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    int x = 640 * u;
    int y = 480 * v;

    for (unsigned int dy = 0; dy < pTexture->GetHeight(); ++dy) {
        for (unsigned int dx = 0; dx < width; ++dx) {
            if (*pixels & 0xFF000000)
                /*temppix[dx + dy * width] = Color32((((*pixels >> 16) & 0xFF) >> color_dimming_level),
                (((*pixels >> 8) & 0xFF) >> color_dimming_level), ((*pixels & 0xFF) >> color_dimming_level))
                &  Color32(mask);*/
                render_target_rgb[x+dx + 640*(y+dy)] = Color32((((*pixels >> 16) & 0xFF) >> color_dimming_level),
                (((*pixels >> 8) & 0xFF) >> color_dimming_level), ((*pixels & 0xFF) >> color_dimming_level))
                &  Color32(mask);
            ++pixels;
        }
    }
    // render->DrawTextureAlphaNew(u, v, temp);
    // temp->Release();;
}

void RenderOpenGL::DrawTextureGrayShade(float a2, float a3, Image *a4) {
    __debugbreak();
}
void RenderOpenGL::DrawIndoorSky(unsigned int uNumVertices,
                                 unsigned int uFaceID) {
    __debugbreak();
}
void RenderOpenGL::DrawIndoorSkyPolygon(signed int uNumVertices,
                                        struct Polygon *pSkyPolygon) {
    __debugbreak();
}

Image *RenderOpenGL::TakeScreenshot(unsigned int width, unsigned int height) { return nullptr; }
bool RenderOpenGL::AreRenderSurfacesOk() { return true; }
void RenderOpenGL::SaveScreenshot(const String &filename, unsigned int width,
                                  unsigned int height) {
    // __debugbreak();
}
void RenderOpenGL::SavePCXScreenshot() { __debugbreak(); }
int RenderOpenGL::GetActorsInViewport(int pDepth) {
    __debugbreak();
    return 0;
}
void RenderOpenGL::BeginLightmaps() { __debugbreak(); }
void RenderOpenGL::EndLightmaps() { __debugbreak(); }
void RenderOpenGL::BeginLightmaps2() { __debugbreak(); }
void RenderOpenGL::EndLightmaps2() { __debugbreak(); }

bool RenderOpenGL::DrawLightmap(struct Lightmap *pLightmap,
                                struct Vec3_float_ *pColorMult, float z_bias) {
    __debugbreak();
    return 0;
}
void RenderOpenGL::BeginDecals() { __debugbreak(); }
void RenderOpenGL::EndDecals() { __debugbreak(); }
void RenderOpenGL::DrawDecal(struct Decal *pDecal, float z_bias) {
    __debugbreak();
}
void RenderOpenGL::do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin,
                                          signed int sDiffuseBegin,
                                          const RenderVertexD3D3 *pLineEnd,
                                          signed int sDiffuseEnd,
                                          float z_stuff) {
    __debugbreak();
}
void RenderOpenGL::DrawLines(const RenderVertexD3D3 *vertices,
                             unsigned int num_vertices) {
    __debugbreak();
}
void RenderOpenGL::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices,
                                          Texture *texture) {
    __debugbreak();
}
void RenderOpenGL::am_Blt_Copy(Rect *pSrcRect, Point *pTargetXY,
                               int blend_mode) {
    __debugbreak();
}
void RenderOpenGL::am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3,
                                 int blend_mode) {
    __debugbreak();
}



void RenderOpenGL::PrepareDecorationsRenderList_ODM() {
    unsigned int v6;        // edi@9
    int v7;                 // eax@9
    SpriteFrame *frame;     // eax@9
    unsigned __int16 *v10;  // eax@9
    int v13;                // ecx@9
    char r;                 // ecx@20
    char g;                 // dl@20
    char b_;                // eax@20
    Particle_sw local_0;    // [sp+Ch] [bp-98h]@7
    unsigned __int16 *v37;  // [sp+84h] [bp-20h]@9
    int v38;                // [sp+88h] [bp-1Ch]@9

    for (unsigned int i = 0; i < uNumLevelDecorations; ++i) {
        // LevelDecoration* decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST) ||
            pLevelDecorations[i].IsObeliskChestActive()) &&
            !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            DecorationDesc *decor_desc = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);
            if (!(decor_desc->uFlags & 0x80)) {
                if (!(decor_desc->uFlags & 0x22)) {
                    v6 = pMiscTimer->uTotalGameTimeElapsed;
                    v7 = abs(pLevelDecorations[i].vPosition.x +
                        pLevelDecorations[i].vPosition.y);

                    frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                        v6 + v7);

                    if (engine->config->seasons_change) {
                        frame = LevelDecorationChangeSeason(decor_desc, v6 + v7, pParty->uCurrentMonth);
                    }

                    if (!frame) {
                        continue;
                    }

                    // v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                    // v6 + v7);

                    v10 = (unsigned __int16 *)stru_5C6E00->Atan2(
                        pLevelDecorations[i].vPosition.x -
                        pIndoorCameraD3D->vPartyPos.x,
                        pLevelDecorations[i].vPosition.y -
                        pIndoorCameraD3D->vPartyPos.y);
                    v38 = 0;
                    v13 = ((signed int)(stru_5C6E00->uIntegerPi +
                        ((signed int)stru_5C6E00->uIntegerPi >>
                            3) +
                        pLevelDecorations[i].field_10_y_rot -
                        (signed int)v10) >>
                        8) &
                        7;
                    v37 = (unsigned __int16 *)v13;
                    if (frame->uFlags & 2) v38 = 2;
                    if ((256 << v13) & frame->uFlags) v38 |= 4;
                    if (frame->uFlags & 0x40000) v38 |= 0x40;
                    if (frame->uFlags & 0x20000) v38 |= 0x80;

                    // for light
                    if (frame->uGlowRadius) {
                        r = 255;
                        g = 255;
                        b_ = 255;
                        if (render->config->is_using_colored_lights) {
                            r = decor_desc->uColoredLightRed;
                            g = decor_desc->uColoredLightGreen;
                            b_ = decor_desc->uColoredLightBlue;
                        }
                        pStationaryLightsStack->AddLight(
                            pLevelDecorations[i].vPosition.x,
                            pLevelDecorations[i].vPosition.y,
                            pLevelDecorations[i].vPosition.z +
                            decor_desc->uDecorationHeight / 2,
                            frame->uGlowRadius, r, g, b_, _4E94D0_light_type);
                    }  // for light

                       // v17 = (pLevelDecorations[i].vPosition.x -
                       // pIndoorCameraD3D->vPartyPos.x) << 16; v40 =
                       // (pLevelDecorations[i].vPosition.y -
                       // pIndoorCameraD3D->vPartyPos.y) << 16;
                    int party_to_decor_x = pLevelDecorations[i].vPosition.x -
                        pIndoorCameraD3D->vPartyPos.x;
                    int party_to_decor_y = pLevelDecorations[i].vPosition.y -
                        pIndoorCameraD3D->vPartyPos.y;
                    int party_to_decor_z = pLevelDecorations[i].vPosition.z -
                        pIndoorCameraD3D->vPartyPos.z;

                    int view_x = 0;
                    int view_y = 0;
                    int view_z = 0;
                    bool visible = pIndoorCameraD3D->ViewClip(
                        pLevelDecorations[i].vPosition.x,
                        pLevelDecorations[i].vPosition.y,
                        pLevelDecorations[i].vPosition.z, &view_x, &view_y,
                        &view_z);

                    if (visible) {
                        if (2 * abs(view_x) >= abs(view_y)) {
                            int projected_x = 0;
                            int projected_y = 0;
                            pIndoorCameraD3D->Project(view_x, view_y, view_z,
                                &projected_x,
                                &projected_y);

                            auto _v41 =
                                frame->scale *
                                fixed::FromInt(pODMRenderParams->int_fov_rad) /
                                fixed::FromInt(view_x);

                            int screen_space_half_width = 0;
                            screen_space_half_width =
                                _v41.GetInt() *
                                frame->hw_sprites[(int)v37]->uBufferWidth / 2;

                            if (projected_x + screen_space_half_width >=
                                (signed int)pViewport->uViewportTL_X &&
                                projected_x - screen_space_half_width <=
                                (signed int)pViewport->uViewportBR_X) {
                                if (::uNumBillboardsToDraw >= 500) return;
                                ::uNumBillboardsToDraw++;
                                ++uNumDecorationsDrawnThisFrame;

                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .hwsprite = frame->hw_sprites[(int)v37];
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_x = pLevelDecorations[i].vPosition.x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_y = pLevelDecorations[i].vPosition.y;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_z = pLevelDecorations[i].vPosition.z;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_x = projected_x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_y = projected_y;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_z = view_x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screenspace_projection_factor_x = _v41.GetFloat();
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screenspace_projection_factor_y = _v41.GetFloat();
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .uPalette = frame->uPaletteIndex;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .field_1E = v38 | 0x200;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .uIndoorSectorID = 0;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .object_pid = PID(OBJECT_Decoration, i);
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .dimming_level = 0;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .pSpriteFrame = frame;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .sTintColor = 0;
                            }
                        }
                    }
                }
            } else {
                memset(&local_0, 0, 0x68);
                local_0.type = ParticleType_Bitmap | ParticleType_Rotating |
                    ParticleType_8;
                local_0.uDiffuse = 0xFF3C1E;
                local_0.x = (double)pLevelDecorations[i].vPosition.x;
                local_0.y = (double)pLevelDecorations[i].vPosition.y;
                local_0.z = (double)pLevelDecorations[i].vPosition.z;
                local_0.r = 0.0;
                local_0.g = 0.0;
                local_0.b = 0.0;
                local_0.particle_size = 1.0;
                local_0.timeToLive = (rand() & 0x80) + 128;
                local_0.texture = spell_fx_renderer->effpar01;
                particle_engine->AddParticle(&local_0);
            }
        }
    }
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

Texture *RenderOpenGL::CreateTexture_ColorKey(const String &name, uint16_t colorkey) {
    return TextureOpenGL::Create(new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey));
}

Texture *RenderOpenGL::CreateTexture_Solid(const String &name) {
    return TextureOpenGL::Create(new Image16bit_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_Alpha(const String &name) {
    return TextureOpenGL::Create(new Alpha_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromIconsLOD(const String &name) {
    return TextureOpenGL::Create(new PCX_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromNewLOD(const String &name) {
    return TextureOpenGL::Create(new PCX_LOD_Loader(pNew_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromFile(const String &name) {
    return TextureOpenGL::Create(new PCX_File_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_Blank(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels) {

    return TextureOpenGL::Create(width, height, format, pixels);
}


Texture *RenderOpenGL::CreateTexture(const String &name) {
    return TextureOpenGL::Create(new Bitmaps_LOD_Loader(pBitmaps_LOD, name));
}

Texture *RenderOpenGL::CreateSprite(const String &name, unsigned int palette_id,
                                    /*refactor*/ unsigned int lod_sprite_id) {
    return TextureOpenGL::Create(
        new Sprites_LOD_Loader(pSprites_LOD, palette_id, name, lod_sprite_id));
}

void RenderOpenGL::Update_Texture(Texture *texture) {
    // takes care of endian flip from literals here - hence BGRA

    auto t = (TextureOpenGL *)texture;
    glBindTexture(GL_TEXTURE_2D, t->GetOpenGlTexture());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->GetWidth(), t->GetHeight(), GL_BGRA, GL_UNSIGNED_BYTE, t->GetPixels(IMAGE_FORMAT_A8R8G8B8));
    glBindTexture(GL_TEXTURE_2D, NULL);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning(L"OpenGL error: (%u)", err);
    }
}

void RenderOpenGL::DeleteTexture(Texture *texture) {
    // crash here when assets not loaded as texture

    auto t = (TextureOpenGL *)texture;
    GLuint texid = t->GetOpenGlTexture();
    if (texid != -1) {
        glDeleteTextures(1, &texid);
    }
}

bool RenderOpenGL::MoveTextureToDevice(Texture *texture) {
    auto t = (TextureOpenGL *)texture;
    /*    return false;
    }

    bool RenderOpenGL::LoadTextureOpenGL(const String &name, bool mipmaps, int
    *out_texture)
    {*/
    /*String filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
    ::tolower);

    HWLTexture* pHWLTexture = pD3DBitmaps.LoadTexture(filename.c_str(),
    mipmaps); if (!pHWLTexture) return false;

    auto rgb = new unsigned char[pHWLTexture->uWidth * pHWLTexture->uHeight *
    4]; for (unsigned int i = 0; i < pHWLTexture->uWidth * pHWLTexture->uHeight;
    ++i)
    {
        rgb[i * 4 + 3] = pHWLTexture->pPixels[i] & 0x8000 ? 0xFF : 0x00;
        rgb[i * 4 + 2] = 8 * ((pHWLTexture->pPixels[i] >> 0) & 0x1F);
        rgb[i * 4 + 1] = 8 * ((pHWLTexture->pPixels[i] >> 5) & 0x1F);
        rgb[i * 4 + 0] = 8 * ((pHWLTexture->pPixels[i] >> 10) & 0x1F);
    }*/

    /*if (filename == "plansky3")
    {
    if (auto f = CreateTga((name + ".tga").c_str(), pHWLTexture->uWidth,
    pHWLTexture->uHeight))
    {
        for (unsigned int i = 0; i < pHWLTexture->uWidth * pHWLTexture->uHeight;
    ++i)
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
    int gl_format = GL_RGB;
        // native_format == IMAGE_FORMAT_A1R5G5B5 ? GL_RGBA : GL_RGB;

    unsigned __int8 *pixels = nullptr;
    if (native_format == IMAGE_FORMAT_R5G6B5) {
        pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8);
    } else if (native_format == IMAGE_FORMAT_A1R5G5B5 || IMAGE_FORMAT_A8R8G8B8) {
        pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8A8);  // rgba
        gl_format = GL_RGBA;
    } else {
        log->Warning(L"Image not loaded!");
    }

    if (pixels) {
        GLuint texid;
        glGenTextures(1, &texid);
        t->SetOpenGlTexture(texid);

        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, gl_format, t->GetWidth(), t->GetHeight(),
                     0, gl_format, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    return false;
}

void _set_3d_projection_matrix() {
    float near_clip = pIndoorCameraD3D->GetNearClip();
    float far_clip = pIndoorCameraD3D->GetFarClip();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    // outdoors 60 - should be 75?
    // indoors 65?/
    // something to do with ratio of screenwidth to viewport width


    gluPerspective(40, double(game_viewport_width/double(game_viewport_height))  // 65.0f
                   /*(GLfloat)window->GetWidth() / (GLfloat)window->GetHeight()*/,
                   near_clip, far_clip);
}

void _set_3d_modelview_matrix() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);

    int camera_x = pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationY / 2048.0);
    int camera_z = pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationY / 2048.0);
    int camera_y = pParty->vPosition.z + pParty->sEyelevel;

    gluLookAt(camera_x, camera_y, camera_z,

              camera_x - pParty->y_rotation_granularity *
                      cosf(2 * 3.14159 * pParty->sRotationY / 2048.0) /*- 5*/,
              camera_y - pParty->y_rotation_granularity *
                             sinf(2 * 3.14159 * (pParty->sRotationX/*-25*/) / 2048.0),
              camera_z - pParty->y_rotation_granularity *
                             sinf(2 * 3.14159 * pParty->sRotationY / 2048.0),

              0, 1, 0);
}

void _set_ortho_projection(bool gameviewport = false) {
    if (!gameviewport) {  // project over entire window
        glViewport(0, 0, window->GetWidth(), window->GetHeight());

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window->GetWidth(), window->GetHeight(), 0, -1, 1);
    } else {  // project to game viewport
        glViewport(game_viewport_x, window->GetHeight()-game_viewport_w-1, game_viewport_width, game_viewport_height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(game_viewport_x, game_viewport_z, game_viewport_w, game_viewport_y, 1, -1);  // far = 1 but ogl looks down -z
    }
}

void _set_ortho_modelview() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const int terrain_block_scale = 512;
const int terrain_height_scale = 32;
void RenderOpenGL::RenderTerrainD3D() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    GLint lasttex = -1;  // keeps track of which tile texture is loaded

    // tile culling maths
    int camx = WorldPosToGridCellX(pIndoorCameraD3D->vPartyPos.x);
    int camz = WorldPosToGridCellZ(pIndoorCameraD3D->vPartyPos.y);
    int tilerange = (pIndoorCameraD3D->GetFarClip() / terrain_block_scale)+1;

    int camfacing = 2048 - pIndoorCameraD3D->sRotationY;
    int right = int(camfacing - (stru_5C6E00->uIntegerPi / 2));
    int left = int(camfacing + (stru_5C6E00->uIntegerPi / 2));
    if (left > 2048) left -= 2048;
    if (right < 0) right += 2048;

    float Light_tile_dist;


    for (int z = 0; z < 128 - 1; ++z) {
        for (int x = 0; x < 128 - 1; ++x) {
            // tile culling
            int xdist = camx - x;
            int zdist = camz - z;

            if (xdist > tilerange || zdist > tilerange) continue;

            int dist = sqrt((xdist)*(xdist) + (zdist)*(zdist));
            if (dist > tilerange) continue;  // crude distance culling

            // could do further x + z culling by camera direction see dx

            int tiledir = stru_5C6E00->Atan2(xdist, zdist)+1024;
            if (tiledir > 2048) {
                tiledir -= 2048;
            }

            if (dist > 2) {  // dont cull near feet
                if (left > right) {  // crude fov culling
                    if ((tiledir > left) || (tiledir < right)) continue;
                } else {
                    if (!(tiledir < left || tiledir > right)) continue;
                }
            }

            // draw tile
            auto tile = pOutdoor->DoGetTile(x, z);
            if (!tile) continue;

            struct Polygon p;
            auto *poly = &p;

            poly->texture = tile->GetTexture();
            if (tile->IsWaterTile()) {
                poly->texture =
                    this->hd_water_tile_anim[this->hd_water_current_frame];
            }

            //Генерация местоположения
            //вершин-------------------------------------------------------------------------
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

            // terrain normal had problems
            /*uint norm_idx = pTerrainNormalIndices[2 * (x * 128 + z) + 1];
            assert(norm_idx < uNumTerrainNormals);

            Vec3_float_ *norm = &pTerrainNormals[norm_idx];
            float _f = ((norm->x * (float)pOutdoor->vSunlight.x / 65536.0) -
                (norm->y * (float)pOutdoor->vSunlight.y / 65536.0) -
                (norm->z * (float)pOutdoor->vSunlight.z / 65536.0));
            pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);*/

            uint norm_idx = pTerrainNormalIndices[2 * (x * 128 + z) + 1];
            assert(norm_idx < uNumTerrainNormals);

            auto norm = &pTerrainNormals[norm_idx];
            float dim =
                ((norm->x * (float)pOutdoor->vSunlight.x / 65536.0) -
                (norm->y * (float)pOutdoor->vSunlight.y / 65536.0) -
                (norm->z * (float)pOutdoor->vSunlight.z / 65536.0));

            // x start -65536 -> 65536
            // y always 0
            // z start -1 -> -65536 -> -1

            // poly->dimming_level = 20.0 - floorf(20.0 * dim + 0.5f);


            // adjusts sun dimming - needs better distance supplied
            poly->dimming_level = ::GetActorTintColor(
                (20.0 - floorf(20.0 * dim + 0.5f)), 0, /*lightdist*/((dist+1) * terrain_block_scale),
                0, 0);




            if (norm_idx < 0 || norm_idx > uNumTerrainNormals - 1)
                norm = 0;
            else
                norm = &pTerrainNormals[norm_idx];

            // if (false) {
            //    // if (engine->config->allow_lightmaps) {
            //    // Ritor1: significant fps slowdown
           //     lightmap_builder->StackLights_TerrainFace(
            //       norm, &Light_tile_dist, VertexRenderList, 4,
            //        1);
            //    decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm,
            //        &Light_tile_dist, VertexRenderList, 4, 1);
            //}


            poly->uEdgeList1Size = x;
            poly->uEdgeList2Size = z;

            // draw animated water under shore
            bool water_border_tile = false;
            if (tile->IsWaterBorderTile()) {
                glDepthMask(GL_FALSE);
                {
                    poly->texture =
                        this->hd_water_tile_anim[this->hd_water_current_frame];

                    auto texturew = (TextureOpenGL *)poly->texture;
                    GLint thistexw = texturew->GetOpenGlTexture();

                    // avoid rebinding same tex if we can
                    if (lasttex != thistexw) {
                        glBindTexture(GL_TEXTURE_2D, thistexw);
                        lasttex = thistexw;
                    }

                    this->DrawTerrainPolygon(poly, true, true);
                    poly->texture = tile->GetTexture();
                }
                glDepthMask(GL_TRUE);

                water_border_tile = true;
            }

            auto texture = (TextureOpenGL *)poly->texture;
            GLint thistex = texture->GetOpenGlTexture();

            // avoid rebinding same tex if we can
            if (lasttex != thistex) {
                glBindTexture(GL_TEXTURE_2D, thistex);
                lasttex = thistex;
            }

            this->DrawTerrainPolygon(poly, water_border_tile, true);
        }
    }
}

void RenderOpenGL::DrawTerrainPolygon(struct Polygon *poly, bool transparent,
                                      bool clampAtTextureBorders) {
    auto texture = (TextureOpenGL *)poly->texture;
    GLint thistex = texture->GetOpenGlTexture();

     // clamping doesnt really help here in opengl so had to alter texture
    // coordinates a bit
    float clamp_fix_u = 1.0f / texture->GetWidth();
    float clamp_fix_v = 1.0f / texture->GetHeight();

    // if (clampAtTextureBorders) {
    //    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // } else {
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // }

    if (transparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float dim = (poly->dimming_level & 0xFF)/255.;  // 1 - (poly->dimming_level / 20.0f);
    int x1 = poly->uEdgeList1Size;
    int z1 = poly->uEdgeList2Size;
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
        glVertex3f((x1 - 64) * terrain_block_scale, y11 * terrain_height_scale,
                   (64 - z1) * terrain_block_scale);

        glTexCoord2f(0.0f + clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x1 - 64) * terrain_block_scale, y12 * terrain_height_scale,
                   (64 - z2) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y22 * terrain_height_scale,
                   (64 - z2) * terrain_block_scale);

        // ---

        glTexCoord2f(0.0f + clamp_fix_u, 0.0f + clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x1 - 64) * terrain_block_scale, y11 * terrain_height_scale,
                   (64 - z1) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 1.0f - clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y22 * terrain_height_scale,
                   (64 - z2) * terrain_block_scale);

        glTexCoord2f(1.0f - clamp_fix_u, 0.0f + clamp_fix_v);
        glColor3f(dim, dim, dim);
        glVertex3f((x2 - 64) * terrain_block_scale, y21 * terrain_height_scale,
                   (64 - z1) * terrain_block_scale);
    }
    glEnd();

    if (transparent) {
        glDisable(GL_BLEND);
    }
}

void RenderOpenGL::DrawOutdoorSkyD3D() {
    int v9;                      // eax@4
    int top_y_proj;                     // ebx@4
    int v13;                     // edi@6
    int v14;                     // ecx@6
    int v15;                     // eax@8
    int v16;                     // eax@12
    signed __int64 v17;          // qtt@13
    signed int v18;              // ecx@13
    struct Polygon pSkyPolygon;  // [sp+14h] [bp-150h]@1
    int horizon_height_offset;                     // [sp+134h] [bp-30h]@1
    int v32;                     // [sp+13Ch] [bp-28h]@6
    int v33;                     // [sp+140h] [bp-24h]@2
    signed __int64 v34;          // [sp+144h] [bp-20h]@1
    int v35;                     // [sp+148h] [bp-1Ch]@4
    int v36;                     // [sp+14Ch] [bp-18h]@2
    int v37;                     // [sp+154h] [bp-10h]@8
    int bot_y_proj;                     // [sp+158h] [bp-Ch]@1
    int v39;                    // [sp+15Ch] [bp-8h]@4

    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    horizon_height_offset =
        (signed __int64)((double)(pODMRenderParams->int_fov_rad *
            pIndoorCameraD3D->vPartyPos.z) /
            ((double)pODMRenderParams->int_fov_rad + 8192.0) +
            (double)(pViewport->uScreenCenterY));

    // magnitude in up direction
    v34 = cos((double)pIndoorCameraD3D->sRotationX * rot_to_rads) *
        pIndoorCameraD3D->GetFarClip();

    bot_y_proj = (signed __int64)((double)(pViewport->uScreenCenterY) -
        (double)pODMRenderParams->int_fov_rad /
        (v34 + 0.0000001) *
        (sin((double)pIndoorCameraD3D->sRotationX * rot_to_rads)
            *
            -pIndoorCameraD3D->GetFarClip() -
            (double)pIndoorCameraD3D->vPartyPos.z));

    // pSkyPolygon.Create_48607B(&SkyBillboard);  //заполняется ptr_38
                                              // pSkyPolygon.ptr_38->_48694B_frustum_sky();
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.ptr_38 = &SkyBillboard;

                                              // if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
                                              // pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
                                              // else
                                              // pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
                                              // pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ?
                                              // (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);
    pSkyPolygon.texture = pOutdoor->sky_texture;
    if (pSkyPolygon.texture) {
        pSkyPolygon.dimming_level = 0;
        pSkyPolygon.uNumVertices = 4;
        // centering(центруем)-----------------------------------------------------------------
        pSkyPolygon.v_18.x =
            -stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX + 16);
        pSkyPolygon.v_18.y = 0;
        pSkyPolygon.v_18.z =
            -stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX + 16);

        // sky wiew position(положение неба на
        // экране)------------------------------------------
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
        VertexRenderList[0].vWorldViewProjX =
            (double)(signed int)pViewport->uViewportTL_X;  // 8
        VertexRenderList[0].vWorldViewProjY =
            (double)(signed int)pViewport->uViewportTL_Y;  // 8

        VertexRenderList[1].vWorldViewProjX =
            (double)(signed int)pViewport->uViewportTL_X;   // 8
        VertexRenderList[1].vWorldViewProjY = (double)bot_y_proj;  // 247

        VertexRenderList[2].vWorldViewProjX =
            (double)(signed int)pViewport->uViewportBR_X;   // 468
        VertexRenderList[2].vWorldViewProjY = (double)bot_y_proj;  // 247

        VertexRenderList[3].vWorldViewProjX =
            (double)(signed int)pViewport->uViewportBR_X;  // 468
        VertexRenderList[3].vWorldViewProjY =
            (double)(signed int)pViewport->uViewportTL_Y;  // 8

        pSkyPolygon.sTextureDeltaU =
            224 * pMiscTimer->uTotalGameTimeElapsed;  // 7168
        pSkyPolygon.sTextureDeltaV =
            224 * pMiscTimer->uTotalGameTimeElapsed;  // 7168

        pSkyPolygon.field_24 = 0x2000000;  // maybe attributes

        double half_fov_angle_rads = ((pODMRenderParams->uCameraFovInDegrees - 1) * pi_double) / 360;

        // far width per pixel??
        v33 = 65536 /
            (signed int)(signed __int64)(((double)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) / 2)
                / tan(half_fov_angle_rads) +
                0.5);
        for (uint i = 0; i < pSkyPolygon.uNumVertices; ++i) {
            // rotate skydome(вращение купола
            // неба)--------------------------------------
            // В игре принята своя система измерения углов. Полный угол (180).
            // Значению угла 0 соответствует направление на север и/или юг (либо
            // на восток и/или запад), значению 65536 еденицам(0х10000)
            // соответствует угол 90. две переменные хранят данные по углу
            // обзора. field_14 по западу и востоку. field_20 по югу и северу от
            // -25080 до 25080
            v39 = fixpoint_mul(
                pSkyPolygon.ptr_38->CamVecLeft_Y,
                v33 * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
            v35 = v39 + pSkyPolygon.ptr_38->CamVecLeft_Z;

            v39 = fixpoint_mul(
                pSkyPolygon.ptr_38->CamVecFront_Y,
                v33 * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.f)));
            v36 = v39 + pSkyPolygon.ptr_38->CamVecFront_Z;

            v9 = fixpoint_mul(
                pSkyPolygon.v_18.z,
                v33 * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
            top_y_proj = pSkyPolygon.v_18.x + v9;
            if (top_y_proj > 0) top_y_proj = 0;
            v13 = v33 * (pViewport->uScreenCenterX -
                (signed __int64)VertexRenderList[i].vWorldViewProjX);
            v34 = -pSkyPolygon.field_24;
            v32 = (signed __int64)VertexRenderList[i].vWorldViewProjY - 1.0;
            v14 = v33 * (horizon_height_offset - v32);
            while (1) {
                if (top_y_proj) {
                    v37 = abs((int)v34 >> 14);
                    v15 = abs(top_y_proj);
                    if (v37 <= v15 ||
                        v32 <= (signed int)pViewport->uViewportTL_Y) {
                        if (top_y_proj <= 0) break;
                    }
                }
                v16 = fixpoint_mul(pSkyPolygon.v_18.z, v14);
                --v32;
                v14 += v33;
                top_y_proj = pSkyPolygon.v_18.x + v16;
            }
            HEXRAYS_LODWORD(v17) = HEXRAYS_LODWORD(v34) << 16;
            HEXRAYS_HIDWORD(v17) = v34 >> 16;
            v18 = v17 / top_y_proj;
            if (v18 < 0) v18 = pIndoorCameraD3D->GetFarClip();
            v37 = v35 + fixpoint_mul(pSkyPolygon.ptr_38->CamVecLeft_X, v13);
            v35 = 224 * pMiscTimer->uTotalGameTimeElapsed +
                ((signed int)fixpoint_mul(v37, v18) >> 3);
            VertexRenderList[i].u =
                (double)v35 /
                ((double)pSkyPolygon.texture->GetWidth() * 65536.0);

            v36 = v36 + fixpoint_mul(pSkyPolygon.ptr_38->CamVecFront_X, v13);
            v35 = 224 * pMiscTimer->uTotalGameTimeElapsed +
                ((signed int)fixpoint_mul(v36, v18) >> 3);
            VertexRenderList[i].v =
                (double)v35 /
                ((double)pSkyPolygon.texture->GetHeight() * 65536.0);

            VertexRenderList[i].vWorldViewPosition.x = pIndoorCameraD3D->GetFarClip();
            VertexRenderList[i]._rhw = 1.0 / (double)(v18 >> 16);
        }

        _set_ortho_projection(1);
        _set_ortho_modelview();

        VertexRenderList[1].vWorldViewProjY =
            VertexRenderList[1].vWorldViewProjY + 80.0;
        VertexRenderList[2].vWorldViewProjY =
            VertexRenderList[2].vWorldViewProjY + 80.0;

        this->DrawOutdoorSkyPolygon(&pSkyPolygon);

        VertexRenderList[0].vWorldViewProjY = (double)top_y_proj;
        VertexRenderList[1].vWorldViewProjY =
            VertexRenderList[1].vWorldViewProjY + 30.0;
        VertexRenderList[2].vWorldViewProjY =
            VertexRenderList[2].vWorldViewProjY + 30.0;
        VertexRenderList[3].vWorldViewProjY = (double)top_y_proj;

        // this->DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}

//----- (004A2DA3) --------------------------------------------------------
void RenderOpenGL::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) {
    auto texture = (TextureOpenGL *)pSkyPolygon->texture;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    VertexRenderList[0].u = 0 - (float)pParty->sRotationY / 512;
    VertexRenderList[1].u = 0 - (float)pParty->sRotationY / 512;
    VertexRenderList[2].u = 1 - (float)pParty->sRotationY / 512;
    VertexRenderList[3].u = 1 - (float)pParty->sRotationY / 512;

    if (pParty->sRotationX > 0) {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationX / 1024;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationX / 1024;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationX / 1024;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationX / 1024;
    } else {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationX / 256;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationX / 256;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationX / 256;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationX / 256;
    }

    glBegin(GL_QUADS);
    {
        for (int i = 0; i < pSkyPolygon->uNumVertices; ++i) {
            unsigned int diffuse = ::GetActorTintColor(
                31, 0, VertexRenderList[i].vWorldViewPosition.x, 1, 0);

            glColor4f(((diffuse >> 16) & 0xFF) / 255.0f,
                      ((diffuse >> 8) & 0xFF) / 255.0f,
                      (diffuse & 0xFF) / 255.0f, 1.0f);

            glTexCoord2f(VertexRenderList[i].u,
                         /*max_v*/ -VertexRenderList[i].v);

            glVertex3f(VertexRenderList[i].vWorldViewProjX,
                VertexRenderList[i].vWorldViewProjY,
                0.99989998);  // z is negative in OpenGL
        }
    }
    glEnd();
}

void RenderOpenGL::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {
    engine->draw_debug_outlines();
    this->DoRenderBillboards_D3D();
    spell_fx_renderer->RenderSpecialEffects();
}

//----- (004A1C1E) --------------------------------------------------------
void RenderOpenGL::DoRenderBillboards_D3D() {
    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);  // some quads are reversed to reuse sprites opposite hand
    glEnable(GL_TEXTURE_2D);

    _set_ortho_projection(1);
    _set_ortho_modelview();

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i) {
        if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend) {
            SetBillboardBlendOptions(pBillboardRenderListD3D[i].opacity);
        }

        auto texture = (TextureOpenGL *)pBillboardRenderListD3D[i].texture;
        glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

        glBegin(GL_TRIANGLE_FAN);
        {
            auto billboard = &pBillboardRenderListD3D[i];
            auto b = &pBillboardRenderList[i];

            // since OpenGL 1.0 can't mirror texture borders, we should offset
            // UV to avoid black edges
            billboard->pQuads[0].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[0].texcoord.y += 0.5f / texture->GetHeight();
            billboard->pQuads[1].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[1].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[2].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[2].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[3].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[3].texcoord.y += 0.5f / texture->GetHeight();

            for (unsigned int j = 0; j < billboard->uNumVertices; ++j) {
                glColor4f(
                    ((billboard->pQuads[j].diffuse >> 16) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 8) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f);

                glTexCoord2f(billboard->pQuads[j].texcoord.x,
                             billboard->pQuads[j].texcoord.y);

                float oneoz = 1. / (billboard->screen_space_z);
                float oneon = 1. / (pIndoorCameraD3D->GetNearClip()+5);
                float oneof = 1. / pIndoorCameraD3D->GetFarClip();

                glVertex3f(
                    billboard->pQuads[j].pos.x,
                    billboard->pQuads[j].pos.y,
                    (oneoz - oneon)/(oneof - oneon) );  // depth is  non linear  proportional to reciprocal of distance
            }
        }
        glEnd();
    }

    // uNumBillboardsToDraw = 0;


    if (config->is_using_fog) {
        SetUsingFog(false);
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP);

        GLfloat fog_color[] = {((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
                               ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
                               ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f,
                               1.0f};
        glFogfv(GL_FOG_COLOR, fog_color);
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

//----- (004A1DA8) --------------------------------------------------------
void RenderOpenGL::SetBillboardBlendOptions(
    RenderBillboardD3D::OpacityType a1) {
    switch (a1) {
        case RenderBillboardD3D::Transparent: {
            if (config->is_using_fog) {
                SetUsingFog(false);
                glEnable(GL_FOG);
                glFogi(GL_FOG_MODE, GL_EXP);

                GLfloat fog_color[] = {
                    ((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f, 1.0f};
                glFogfv(GL_FOG_COLOR, fog_color);
            }

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } break;

        case RenderBillboardD3D::Opaque_1:
        case RenderBillboardD3D::Opaque_2:
        case RenderBillboardD3D::Opaque_3: {
            if (config->is_using_specular) {
                if (!config->is_using_fog) {
                    SetUsingFog(true);
                    glDisable(GL_FOG);
                }
            }

            glBlendFunc(GL_ONE, GL_ZERO);
        } break;

        default:
            log->Warning(
                L"SetBillboardBlendOptions: invalid opacity type (%u)", a1);
            assert(false);
            break;
    }
}

void RenderOpenGL::PackScreenshot(unsigned int width, unsigned int height,
                                  void *out_data, unsigned int data_size,
                                  unsigned int *screenshot_size) {
    /*auto pixels = MakeScreenshot(150, 112);
    PackPCXpicture(pixels, 150, 112, data, 1000000, out_screenshot_size);
    free(pixels);*/
}

void RenderOpenGL::SetUIClipRect(unsigned int x, unsigned int y, unsigned int z,
                                 unsigned int w) {
    this->clip_x = x;
    this->clip_y = y;
    this->clip_z = z;
    this->clip_w = w;
    glScissor(x, 480-w, z-x, w-y);  // invert glscissor co-ords 0,0 is BL
}

void RenderOpenGL::ResetUIClipRect() {
    this->SetUIClipRect(0, 0, this->window->GetWidth(), this->window->GetHeight());
}

void RenderOpenGL::PresentBlackScreen() {
    BeginScene();
    ClearBlack();
    EndScene();
    Present();
}

void RenderOpenGL::BeginScene() {
    // Setup for 2D

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    _set_ortho_projection();
    _set_ortho_modelview();
}

void RenderOpenGL::EndScene() {
    // blank in d3d
}



void RenderOpenGL::DrawTextureAlphaNew(float u, float v, Image *img) {
    DrawTextureNew(u, v, img);
    return;
}

void RenderOpenGL::DrawTextureNew(float u, float v, Image *tex) {
    if (!tex) __debugbreak();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto texture = (TextureOpenGL *)tex;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = tex->GetWidth();
    int height = tex->GetHeight();

    int x = u * 640;
    int y = v * 480;
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= 640 || x >= clipz || y >= 480 || y >= clipw) return;
    // check for overlap
    if ((clipx < z && clipz > x && clipy > w && clipw < y)) return;

    int drawx = max(x, clipx);
    int drawy = max(y, clipy);
    int draww = min(w, clipw);
    int drawz = min(z, clipz);

    float depth = 0;

    GLfloat Vertices[] = { (float)drawx, (float)drawy, depth,
        (float)drawz, (float)drawy, depth,
        (float)drawz, (float)draww, depth,
        (float)drawx, (float)draww, depth };

    float texx = (drawx - x) / float(width);
    float texy = (drawy - y) / float(height);
    float texz = (width - (z - drawz)) / float(width);
    float texw = (height - (w - draww)) / float(height);

    GLfloat TexCoord[] = { texx, texy,
        texz, texy,
        texz, texw,
        texx, texw,
    };

     GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning(L"OpenGL error: (%u)", err);
    }

    // blank over same bit of this render_target_rgb to stop text overlaps
    for (int ys = drawy; ys < draww; ys++) {
        memset(this->render_target_rgb +(ys * 640 + drawx), 0x00000000, (drawz - drawx) * 4);
    }
}

void RenderOpenGL::DrawTextureCustomHeight(float u, float v, class Image *img,
                                           int custom_height) {
    unsigned __int16 *v6;  // esi@3
    unsigned int v8;       // eax@5
    unsigned int v11;      // eax@7
    unsigned int v12;      // ebx@8
    unsigned int v15;      // eax@14
    int v19;               // [sp+10h] [bp-8h]@3

    if (!img) return;

    unsigned int uOutX = 640 * u;
    unsigned int uOutY = 480 * v;

    int width = img->GetWidth();
    int height = min(img->GetHeight(), custom_height);
    v6 = (unsigned __int16 *)img->GetPixels(IMAGE_FORMAT_R5G6B5);

    // v5 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v19 = width;
    // if (this->bClip)
    {
        if ((signed int)uOutX < (signed int)this->clip_x) {
            v8 = this->clip_x - uOutX;
            unsigned int v9 = uOutX - this->clip_x;
            v8 *= 2;
            width += v9;
            v6 = (unsigned __int16 *)((char *)v6 + v8);
            // v5 = (unsigned __int16 *)((char *)v5 + v8);
        }
        if ((signed int)uOutY < (signed int)this->clip_y) {
            v11 = this->clip_y - uOutY;
            v6 += v19 * v11;
            height += uOutY - this->clip_y;
            // v5 += this->uTargetSurfacePitch * v11;
        }
        v12 = max(this->clip_x, uOutX);
        if ((signed int)(width + v12) > (signed int)this->clip_z) {
            width = this->clip_z - max(this->clip_x, uOutX);
        }
        v15 = max(this->clip_y, uOutY);
        if ((signed int)(v15 + height) > (signed int)this->clip_w) {
            height = this->clip_w - max(this->clip_y, uOutY);
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            WritePixel16(uOutX + x, uOutY + y, *v6);
            // *v5 = *v6;
            // ++v5;
            ++v6;
        }
        v6 += v19 - width;
        // v5 += this->uTargetSurfacePitch - v4;
    }
}

void RenderOpenGL::DrawText(int uOutX, int uOutY, uint8_t *pFontPixels,
                            unsigned int uCharWidth, unsigned int uCharHeight,
                            uint8_t *pFontPalette, unsigned __int16 uFaceColor,
                            unsigned __int16 uShadowColor) {
    // needs limits checks adding

    // Image *fonttemp = Image::Create(uCharWidth, uCharHeight, IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *fontpix = (uint32_t*)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (uint y = 0; y < uCharHeight; ++y) {
        for (uint x = 0; x < uCharWidth; ++x) {
            if (*pFontPixels) {
                uint16_t color = uShadowColor;
                if (*pFontPixels != 1) {
                    color = uFaceColor;
                }
                // fontpix[x + y * uCharWidth] = Color32(color);
                this->render_target_rgb[(uOutX+x)+(uOutY+y)*640] = Color32(color);
            }
            ++pFontPixels;
        }
    }
    // render->DrawTextureAlphaNew(uOutX / 640., uOutY / 480., fonttemp);
    // fonttemp->Release();
}

void RenderOpenGL::DrawTextAlpha(int x, int y, unsigned char *font_pixels,
                                 int uCharWidth, unsigned int uFontHeight,
                                 uint8_t *pPalette,
                                 bool present_time_transparency) {
    // needs limits checks adding

    // Image *fonttemp = Image::Create(uCharWidth, uFontHeight, IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *fontpix = (uint32_t *)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (present_time_transparency) {
        for (unsigned int dy = 0; dy < uFontHeight; ++dy) {
            for (unsigned int dx = 0; dx < uCharWidth; ++dx) {
                uint16_t color = (*font_pixels)
                    ? pPalette[*font_pixels]
                    : 0x7FF;  // transparent color 16bit
                              // render->uTargetGMask |
                              // render->uTargetBMask;
                this->render_target_rgb[(x + dx) + (y + dy) * 640] = Color32(color);
                // fontpix[dx + dy * uCharWidth] = Color32(color);
                ++font_pixels;
            }
        }
    } else {
        for (unsigned int dy = 0; dy < uFontHeight; ++dy) {
            for (unsigned int dx = 0; dx < uCharWidth; ++dx) {
                if (*font_pixels) {
                    uint8_t index = *font_pixels;
                    uint8_t r = pPalette[index * 3 + 0];
                    uint8_t g = pPalette[index * 3 + 1];
                    uint8_t b = pPalette[index * 3 + 2];
                    this->render_target_rgb[(x + dx) + (y + dy) * 640] = Color32(r, g, b);
                    // fontpix[dx + dy * uCharWidth] = Color32(r, g, b);
                }
                ++font_pixels;
            }
        }
    }
    // render->DrawTextureAlphaNew(x / 640., y / 480., fonttemp);
    // fonttemp->Release();
}

void RenderOpenGL::Present() {
    // screen overlay holds all text and changing images at the moment

    static Texture *screen_text_overlay = 0;
    if (!screen_text_overlay) {
        screen_text_overlay = render->CreateTexture_Blank(640, 480, IMAGE_FORMAT_A8R8G8B8);
    }

    uint32_t *pix = (uint32_t*)screen_text_overlay->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    unsigned int num_pixels = screen_text_overlay->GetWidth() * screen_text_overlay->GetHeight();
    unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A8R8G8B8);

    // update pixels
    memcpy(pix, this->render_target_rgb, num_pixels_bytes);
    // update texture
    render->Update_Texture(screen_text_overlay);
    // draw
    render->DrawTextureAlphaNew(0, 0, screen_text_overlay);

    window->OpenGlSwapBuffers();
}

RenderVertexSoft ogl_draw_buildings_vertices[20];
void RenderOpenGL::DrawBuildingsD3D() {
    // int v27;  // eax@57
    int v49;  // [sp+2Ch] [bp-2Ch]@10
    int v50;  // [sp+30h] [bp-28h]@34
    int v51;  // [sp+34h] [bp-24h]@35
    int v52;  // [sp+38h] [bp-20h]@36
    int v53;  // [sp+3Ch] [bp-1Ch]@8

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    for (BSPModel &model : pOutdoor->pBModels) {
        int reachable;
        if (IsBModelVisible(&model, &reachable)) {
            model.field_40 |= 1;
            if (!model.pFaces.empty()) {
                for (ODMFace &face : model.pFaces) {
                    if (!face.Invisible()) {
                        v53 = 0;
                        auto poly =
                            &array_77EC08[pODMRenderParams->uNumPolygons];

                        poly->flags = 0;
                        poly->field_32 = 0;
                        poly->texture = face.GetTexture();

                        if (face.uAttributes & FACE_FLUID) poly->flags |= 2;
                        if (face.uAttributes & FACE_INDOOR_SKY)
                            poly->flags |= 0x400;

                        if (face.uAttributes & FACE_FLOW_DIAGONAL)
                            poly->flags |= 0x400;
                        else if (face.uAttributes & FACE_FLOW_VERTICAL)
                            poly->flags |= 0x800;

                        if (face.uAttributes & FACE_FLOW_HORIZONTAL)
                            poly->flags |= 0x2000;
                        else if (face.uAttributes & FACE_DONT_CACHE_TEXTURE)
                            poly->flags |= 0x1000;

                        poly->sTextureDeltaU = face.sTextureDeltaU;
                        poly->sTextureDeltaV = face.sTextureDeltaV;

                        unsigned int flow_anim_timer = OS_GetTime() >> 4;
                        unsigned int flow_u_mod = poly->texture->GetWidth() - 1;
                        unsigned int flow_v_mod =
                            poly->texture->GetHeight() - 1;

                        if (face.pFacePlane.vNormal.z &&
                            abs(face.pFacePlane.vNormal.z) >= 59082) {
                            if (poly->flags & 0x400)
                                poly->sTextureDeltaV +=
                                    flow_anim_timer & flow_v_mod;
                            if (poly->flags & 0x800)
                                poly->sTextureDeltaV -=
                                    flow_anim_timer & flow_v_mod;
                        } else {
                            if (poly->flags & 0x400)
                                poly->sTextureDeltaV -=
                                    flow_anim_timer & flow_v_mod;
                            if (poly->flags & 0x800)
                                poly->sTextureDeltaV +=
                                    flow_anim_timer & flow_v_mod;
                        }

                        if (poly->flags & 0x1000)
                            poly->sTextureDeltaU -=
                                flow_anim_timer & flow_u_mod;
                        else if (poly->flags & 0x2000)
                            poly->sTextureDeltaU +=
                                flow_anim_timer & flow_u_mod;

                        v50 = 0;
                        v49 = 0;

                        for (uint vertex_id = 1; vertex_id <= face.uNumVertices;
                             vertex_id++) {
                            array_73D150[vertex_id - 1].vWorldPosition.x =
                                model.pVertices
                                    .pVertices[face.pVertexIDs[vertex_id - 1]]
                                    .x;
                            array_73D150[vertex_id - 1].vWorldPosition.y =
                                model.pVertices
                                    .pVertices[face.pVertexIDs[vertex_id - 1]]
                                    .y;
                            array_73D150[vertex_id - 1].vWorldPosition.z =
                                model.pVertices
                                    .pVertices[face.pVertexIDs[vertex_id - 1]]
                                    .z;
                            array_73D150[vertex_id - 1].u =
                                (poly->sTextureDeltaU +
                                 (__int16)face.pTextureUIDs[vertex_id - 1]) *
                                (1.0 / (double)poly->texture->GetWidth());
                            array_73D150[vertex_id - 1].v =
                                (poly->sTextureDeltaV +
                                 (__int16)face.pTextureVIDs[vertex_id - 1]) *
                                (1.0 / (double)poly->texture->GetHeight());
                        }
                        memcpy(ogl_draw_buildings_vertices, array_73D150,
                               sizeof(array_73D150));

                        /*for (uint i = 1; i <= face.uNumVertices; i++) {
                            if (model.pVertices.pVertices[face.pVertexIDs[0]]
                                    .z == array_73D150[i - 1].vWorldPosition.z)
                                ++v53;
                            pIndoorCameraD3D->ViewTransform(
                                &array_73D150[i - 1], 1);
                            if (array_73D150[i - 1].vWorldViewPosition.x <
                                    pIndoorCameraD3D->GetNearClip() ||
                                array_73D150[i - 1].vWorldViewPosition.x >
                                    pIndoorCameraD3D->GetFarClip()) {
                                if (array_73D150[i - 1].vWorldViewPosition.x >=
                                    pIndoorCameraD3D->GetNearClip())
                                    v49 = 1;
                                else
                                    v50 = 1;
                            } else {
                                pIndoorCameraD3D->Project(&array_73D150[i - 1],
                                    1, 0);
                            }
                        }*/

                        if (v53 == face.uNumVertices) {
                            poly->field_32 |= 1;
                        }
                        poly->pODMFace = &face;
                        poly->uNumVertices = face.uNumVertices;
                        poly->field_59 = 5;
                        v51 = fixpoint_mul(-pOutdoor->vSunlight.x,
                                           face.pFacePlane.vNormal.x);
                        v53 = fixpoint_mul(-pOutdoor->vSunlight.y,
                                           face.pFacePlane.vNormal.y);
                        v52 = fixpoint_mul(-pOutdoor->vSunlight.z,
                                           face.pFacePlane.vNormal.z);
                        poly->dimming_level =
                            20 - fixpoint_mul(20, v51 + v53 + v52);
                        if (poly->dimming_level < 0) poly->dimming_level = 0;
                        if (poly->dimming_level > 31) poly->dimming_level = 31;
                        if (pODMRenderParams->uNumPolygons >= 1999 + 5000)
                            return;
                        if (ODMFace::IsBackfaceNotCulled(array_73D150, poly)) {
                            face.bVisible = 1;
                            poly->uBModelFaceID = face.index;
                            poly->uBModelID = model.index;
                            poly->pid = PID(OBJECT_BModel,
                                            face.index | (model.index << 6));
                            for (int vertex_id = 0;
                                 vertex_id < face.uNumVertices; ++vertex_id) {
                                memcpy(&VertexRenderList[vertex_id],
                                       &array_73D150[vertex_id],
                                       sizeof(VertexRenderList[vertex_id]));
                                VertexRenderList[vertex_id]._rhw =
                                    1.0 / (array_73D150[vertex_id]
                                               .vWorldViewPosition.x +
                                           0.0000001);
                            }


                            // this does not work for ogl - fix?
                            /*if (v50) {
                                poly->uNumVertices = ODM_NearClip(face.uNumVertices);
                                ODM_Project(poly->uNumVertices);
                            }
                            if (v49) {
                                poly->uNumVertices = ODM_FarClip(face.uNumVertices);
                                ODM_Project(poly->uNumVertices);
                            }*/

                            if (poly->uNumVertices) {
                                if (poly->IsWater()) {
                                    if (poly->IsWaterAnimDisabled())
                                        poly->texture =
                                            render->hd_water_tile_anim[0];
                                    else
                                        poly->texture =
                                            render->hd_water_tile_anim
                                                [render
                                                     ->hd_water_current_frame];
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

void RenderOpenGL::DrawPolygon(struct Polygon *poly) {
    if (poly->uNumVertices < 3) {
        return;
    }

    auto texture = (TextureOpenGL *)poly->texture;
    auto a4 = poly->pODMFace;
    auto uNumVertices = poly->uNumVertices;

    unsigned int a2 = 0xFFFFFFFF;
    engine->AlterGamma_ODM(a4, &a2);

    if (!lightmap_builder->StationaryLightsCount ||
        _4D864C_force_sw_render_rules && engine->config->Flag1_2()) {
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

        glBegin(GL_TRIANGLE_FAN);  // GL_TRIANGLE_FAN

        int outline_color;
        if (OS_GetTime() % 300 >= 150)
            outline_color = 0xFFFF2020;
        else
            outline_color = 0xFF901010;

        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
            d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
            d3d_vertex_buffer[i].pos.z =
                1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) /
                             pIndoorCameraD3D->GetFarClip());
            d3d_vertex_buffer[i].rhw =
                1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
            d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(
                poly->dimming_level, 0,
                VertexRenderList[i].vWorldViewPosition.x, 0, 0);
            engine->AlterGamma_ODM(a4, &d3d_vertex_buffer[i].diffuse);

            if (a4->uAttributes & FACE_OUTLINED) {
                d3d_vertex_buffer[i].diffuse = outline_color;
            }

            if (config->is_using_specular)
                d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                    0, 0, VertexRenderList[i].vWorldViewPosition.x);
            else
                d3d_vertex_buffer[i].specular = 0;
            d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
            d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;

            glTexCoord2f(ogl_draw_buildings_vertices[i].u,
                         ogl_draw_buildings_vertices[i].v);

            glColor4f(
                ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                config->is_using_specular
                    ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                    : 1.0f);

            glVertex3f(ogl_draw_buildings_vertices[i].vWorldPosition.x,
                       ogl_draw_buildings_vertices[i].vWorldPosition.z,
                       ogl_draw_buildings_vertices[i].vWorldPosition.y);
        }

        glEnd();

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            log->Warning(L"OpenGL error: (%u)", err);
        }

    } else {
        /*for (uint i = 0; i < uNumVertices; ++i)
        {

        d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 /
        ((VertexRenderList[i].vWorldViewPosition.x * 1000) /
        (double)pODMRenderParams->shading_dist_mist); d3d_vertex_buffer[i].rhw
        = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
        d3d_vertex_buffer[i].diffuse = GetActorTintColor(a3->dimming_level, 0,
        VertexRenderList[i].vWorldViewPosition.x, 0, 0); if
        (this->bUsingSpecular) d3d_vertex_buffer[i].specular =
        sub_47C3D7_get_fog_specular(0, 0,
        VertexRenderList[i].vWorldViewPosition.x); else
        d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
        d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;

        }

        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
        FALSE)); ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0,
        D3DTSS_ADDRESS, D3DTADDRESS_WRAP)); if (bUsingSpecular)
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,
        FALSE));

        ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
        d3d_vertex_buffer,
        uNumVertices,
        D3DDP_DONOTLIGHT));
        //v50 = (const char *)v5->pRenderD3D->pDevice;
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
        D3DCULL_NONE));
        //(*(void (**)(void))(*(int *)v50 + 88))();
        lightmap_builder->DrawLightmaps(-1);
        for (uint i = 0; i < uNumVertices; ++i)
        {
        d3d_vertex_buffer[i].diffuse = a2;
        }
        ErrD3D(pRenderD3D->pDevice->SetTexture(0,
        texture->GetDirect3DTexture()));
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
        D3DTADDRESS_WRAP)); if (!render->bUsingSpecular)
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
        TRUE));

        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
        TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
        D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
        D3DBLEND_SRCCOLOR));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
        d3d_vertex_buffer,
        uNumVertices,
        D3DDP_DONOTLIGHT));
        if (bUsingSpecular)
        {
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
        TRUE));

        for (uint i = 0; i < uNumVertices; ++i)
        {
        d3d_vertex_buffer[i].diffuse = render->uFogColor |
        d3d_vertex_buffer[i].specular & 0xFF000000;
        d3d_vertex_buffer[i].specular = 0;
        }

        ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
        D3DBLEND_INVSRCALPHA));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
        D3DBLEND_SRCALPHA));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
        d3d_vertex_buffer,
        uNumVertices,
        D3DDP_DONOTLIGHT));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,
        TRUE));
        //v40 = render->pRenderD3D->pDevice->lpVtbl;
        v41 = GetLevelFogColor();
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
        GetLevelFogColor() & 0xFFFFFF); v6 = 0;
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0);
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
        D3DBLEND_ONE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
        D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
        v6));*/
    }
}

void RenderOpenGL::DrawIndoorPolygon(unsigned int uNumVertices, BLVFace *pFace,
    int uPackedID, unsigned int uColor,
    int a8) {
    if (uNumVertices < 3) {
        return;
    }

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    unsigned int sCorrectedColor = uColor;

    TextureOpenGL *texture = (TextureOpenGL *)pFace->GetTexture();

    if (lightmap_builder->StationaryLightsCount) sCorrectedColor = -1;
    engine->AlterGamma_BLV(pFace, &sCorrectedColor);

    if (pFace->uAttributes & FACE_OUTLINED) {
        if (OS_GetTime() % 300 >= 150)
            uColor = sCorrectedColor = 0xFF20FF20;
        else
            uColor = sCorrectedColor = 0xFF109010;
    }

    if (_4D864C_force_sw_render_rules && engine->config->Flag1_1()) {
        /*
        __debugbreak();
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
        false)); ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0,
        D3DTSS_ADDRESS, D3DTADDRESS_WRAP)); for (uint i = 0; i <
        uNumVertices; ++i)
        {
        d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 /
        (array_507D30[i].vWorldViewPosition.x * 0.061758894);
        d3d_vertex_buffer[i].rhw = 1.0 /
        array_507D30[i].vWorldViewPosition.x; d3d_vertex_buffer[i].diffuse =
        sCorrectedColor; d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u /
        (double)pFace->GetTexture()->GetWidth();
        d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v /
        (double)pFace->GetTexture()->GetHeight();
        }

        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
        D3DTADDRESS_WRAP)); ErrD3D(pRenderD3D->pDevice->SetTexture(0,
        nullptr));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        d3d_vertex_buffer, uNumVertices, 28));
        lightmap_builder->DrawLightmaps(-1);
        */
    } else {
        if (!lightmap_builder->StationaryLightsCount ||
            _4D864C_force_sw_render_rules && engine->config->Flag1_2()) {
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

            // glDisable(GL_CULL_FACE);  // testing
            // glDisable(GL_DEPTH_TEST);

            // if (uNumVertices != 3 ) return; //3 ,4, 5 ,6

            glBegin(GL_TRIANGLE_FAN);

            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z =
                    1.0 -
                    1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);

                d3d_vertex_buffer[i].rhw =
                    1.0 / array_507D30[i].vWorldViewPosition.x;
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
                d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x =
                    array_507D30[i].u / (double)pFace->GetTexture()->GetWidth();
                d3d_vertex_buffer[i].texcoord.y =
                    array_507D30[i].v /
                    (double)pFace->GetTexture()->GetHeight();


                /*glTexCoord2f(d3d_vertex_buffer[i].texcoord.x,
                    d3d_vertex_buffer[i].texcoord.y);*/

                glTexCoord2f((pFace->pVertexUIDs[i]/ (double)pFace->GetTexture()->GetWidth()), (pFace->pVertexVIDs[i]/ (double)pFace->GetTexture()->GetHeight()));


                 /*glColor4f(
                ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                config->is_using_specular
                ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                : 1.0f);*/

                glColor4f(1, 1, 1, 1);

                /*glVertex3f(d3d_vertex_buffer[i].pos.x,
                d3d_vertex_buffer[i].pos.z,
                d3d_vertex_buffer[i].pos.y);*/



                glVertex3f(pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].z,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].y);
            }

            glEnd();
        } else {
            /*
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

            glBegin(GL_TRIANGLE_FAN);

            for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x =
            array_507D30[i].vWorldViewProjX; d3d_vertex_buffer[i].pos.y =
            array_507D30[i].vWorldViewProjY; d3d_vertex_buffer[i].pos.z = 1.0
            - 1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
            d3d_vertex_buffer[i].rhw = 1.0 /
            array_507D30[i].vWorldViewPosition.x;
            d3d_vertex_buffer[i].diffuse = uColor;
            d3d_vertex_buffer[i].specular = 0;
            d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u /
            (double)pFace->GetTexture()->GetWidth();
            d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v /
            (double)pFace->GetTexture()->GetHeight();
            }
            glBindTexture(GL_TEXTURE_2D,
            face_texture->GetOpenGlTexture());
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR |
            D3DFVF_TEX1, d3d_vertex_buffer, uNumVertices, 28));

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
            D3DCULL_NONE)); lightmap_builder->DrawLightmaps(-1);

            for (uint i = 0; i < uNumVertices; ++i)
            d3d_vertex_buffer[i].diffuse = sCorrectedColor;

            glBindTexture(GL_TEXTURE_2D,
            face_texture->GetOpenGlTexture());
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
            TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
            TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
            D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
            D3DBLEND_SRCCOLOR));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR |
            D3DFVF_TEX1, d3d_vertex_buffer, uNumVertices, 28));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
            D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
            D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
            FALSE));
            */
        }
    }
}

bool RenderOpenGL::SwitchToWindow() {
    // pParty->uFlags |= PARTY_FLAGS_1_0002;
    pViewport->SetFOV(_6BE3A0_fov);
    CreateZBuffer();

    return true;
}


bool RenderOpenGL::Initialize(OSWindow *window_) {
    if (!RenderBase::Initialize(window_)) {
        return false;
    }

    if (window != nullptr) {
        window->OpenGlCreate();


        glShadeModel(GL_SMOOTH);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // Black Background
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, window->GetWidth(), window->GetHeight());
        glScissor(0, 0, window->GetWidth(), window->GetHeight());

        glEnable(GL_SCISSOR_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Calculate The Aspect Ratio Of The Window
        gluPerspective(45.0f,
            (GLfloat)window->GetWidth() / (GLfloat)window->GetHeight(),
            0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Swap Buffers (Double Buffering)
        window->OpenGlSwapBuffers();

        this->clip_x = this->clip_y = 0;
        this->clip_z = window->GetWidth();
        this->clip_w = window->GetHeight();
        this->render_target_rgb =
            new uint32_t[window->GetWidth() *
            window->GetHeight()];

        PostInitialization();

        return true;
    }

    return false;
}

void RenderOpenGL::WritePixel16(int x, int y, uint16_t color) {
    // render target now 32 bit - format A8R8G8B8
    render_target_rgb[x + window->GetWidth() * y] = Color32(color);
}

void RenderOpenGL::FillRectFast(unsigned int uX, unsigned int uY,
                                unsigned int uWidth, unsigned int uHeight,
                                unsigned int uColor16) {
    // uint32_t col = Color32(uColor16);
    // for (unsigned int dy = 0; dy < uHeight; ++dy) {
    //    memset32(this->render_target_rgb + ((uY+dy) * window->GetWidth() + uX), col, uWidth);
    // }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    unsigned int b = (uColor16 & 0x1F) * 8;
    unsigned int g = ((uColor16 >> 5) & 0x3F) * 4;
    unsigned int r = ((uColor16 >> 11) & 0x1F) * 8;
    glColor3ub(r, g, b);

    float depth = 0;

    GLfloat Vertices[] = { (float)uX, (float)uY, depth,
        (float)(uX+uWidth), (float)uY, depth,
        (float)(uX + uWidth), (float)(uY+uHeight), depth,
        (float)uX, (float)(uY + uHeight), depth };

    GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    glDisableClientState(GL_VERTEX_ARRAY);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning(L"OpenGL error: (%u)", err);
    }
}


