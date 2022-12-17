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

#include "glad/gl.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <algorithm>
#include <memory>
#include <utility>
#include <map>
#include <filesystem>
#include <chrono>

#include <glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/OpenGL/GLShaderLoader.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Arcomage/Arcomage.h"


#include "Utility/Geometry/Size.h"
#include "Utility/Memory.h"
#include "Utility/Math/TrigLut.h"

#ifndef LOWORD
    #define LOWORD(l) ((unsigned short)(((std::uintptr_t)(l)) & 0xFFFF))
#endif

// globals
//TODO(pskelton): Combine and contain
std::shared_ptr<IRender> render;
int uNumDecorationsDrawnThisFrame;
RenderBillboard pBillboardRenderList[500];
unsigned int uNumBillboardsToDraw;
int uNumSpritesDrawnThisFrame;
RenderVertexSoft array_73D150[20];
RenderVertexSoft VertexRenderList[50];
RenderVertexD3D3 d3d_vertex_buffer[50];
RenderVertexSoft array_507D30[50];
Sizei output = {0, 0};

// improved error check - using glad post call back
void GL_Check_Errors(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLenum err = glad_glGetError();

    while (err != GL_NO_ERROR) {
        static std::string error;

        switch (err) {
            case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
            default:                        error = "Unknown Error";  break;
        }

        logger->Warning("OpenGL error (%u): %s from function %s", err, error.c_str(), name);

        err = glad_glGetError();
    }
}

// sky billboard stuff

void SkyBillboardStruct::CalcSkyFrustumVec(int x1, int y1, int z1, int x2, int y2, int z2) {
    // 6 0 0 0 6 0

    // TODO(pskelton): clean up

    float cosz = pCamera3D->fRotationZCosine;
    float cosx = pCamera3D->fRotationYCosine;
    float sinz = pCamera3D->fRotationZSine;
    float sinx = pCamera3D->fRotationYSine;

    // positions all minus ?
    float v11 = cosz * -pCamera3D->vCameraPos.x + sinz * -pCamera3D->vCameraPos.y;
    float v24 = cosz * -pCamera3D->vCameraPos.y - sinz * -pCamera3D->vCameraPos.x;

    // cam position transform
    if (pCamera3D->sRotationY) {
        this->field_0_party_dir_x = (v11 * cosx) + (-pCamera3D->vCameraPos.z * sinx);
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z * cosx) /*-*/ + (v11 * sinx);
    } else {
        this->field_0_party_dir_x = v11;
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z);
    }

    // set 1 position transfrom (6 0 0) looks like cam left vector
    if (pCamera3D->sRotationY) {
        float v17 = (x1 * cosz) + (y1 * sinz);

        this->CamVecLeft_Y = (v17 * cosx) + (z1 * sinx);  // dz
        this->CamVecLeft_X = (y1 * cosz) - (x1 * sinz);  // dx
        this->CamVecLeft_Z = (z1 * cosx) /*-*/ + (v17 * sinx);  // dy
    } else {
        this->CamVecLeft_Y = (x1 * cosz) + (y1 * sinz);  // dz
        this->CamVecLeft_X = (y1 * cosz) - (x1 * sinz);  // dx
        this->CamVecLeft_Z = static_cast<float>(z1);  // dy
    }

    // set 2 position transfrom (0 1 0) looks like cam front vector
    if (pCamera3D->sRotationY) {
        float v19 = (x2 * cosz) + (y2 * sinz);

        this->CamVecFront_Y = (v19 * cosx) + (z2 * sinx);  // dz
        this->CamVecFront_X = (y2 * cosz) - (x2 * sinz);  // dx
        this->CamVecFront_Z = (z2 * cosx) /*-*/ + (v19 * sinx);  // dy
    } else {
        this->CamVecFront_Y = (x2 * cosz) + (y2 * sinz);  // dz
        this->CamVecFront_X = (y2 * cosz) - (x2 * sinz);  // dx
        this->CamVecFront_Z = static_cast<float>(z2);  // dy
    }

    this->CamLeftDot =
        (this->CamVecLeft_X * this->field_0_party_dir_x) +
        (this->CamVecLeft_Y * this->field_4_party_dir_y) +
        (this->CamVecLeft_Z * this->field_8_party_dir_z);
    this->CamFrontDot =
        (this->CamVecFront_X * this->field_0_party_dir_x) +
        (this->CamVecFront_Y * this->field_4_party_dir_y) +
        (this->CamVecFront_Z * this->field_8_party_dir_z);
}

RenderOpenGL::RenderOpenGL(
    std::shared_ptr<Application::GameConfig> config,
    DecalBuilder* decal_builder,
    LightmapBuilder* lightmap_builder,
    SpellFxRenderer* spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis* vis,
    Log* logger
) : RenderBase(config, decal_builder, lightmap_builder, spellfx, particle_engine, vis, logger) {
    clip_w = 0;
    clip_x = 0;
    clip_y = 0;
    clip_z = 0;
}

RenderOpenGL::~RenderOpenGL() { logger->Info("RenderGl - Destructor"); }

void RenderOpenGL::Release() { logger->Info("RenderGL - Release"); }

void RenderOpenGL::MaskGameViewport() {
    // do not want in opengl mode
}

void RenderOpenGL::SaveWinnersCertificate(const char *a1) {
    GLubyte *sPixels = new GLubyte[3 * output.w * output.h];
    glReadPixels(0, 0, output.w, output.h, GL_RGB, GL_UNSIGNED_BYTE, sPixels);

    uint16_t *pPixels = (uint16_t *)malloc(sizeof(uint16_t) * output.h * output.w);
    memset(pPixels, 0, sizeof(uint16_t) * output.h * output.w);

    // reverse pixels from ogl (uses BL as 0,0)
    uint16_t *for_pixels = pPixels;
    uint8_t *p = sPixels;
    for (uint y = 0; y < (unsigned int)output.h; ++y) {
        for (uint x = 0; x < (unsigned int)output.w; ++x) {
            p = sPixels + 3 * (int)(x) + 3 * (int)(output.h -1 - y) * output.w;

            *for_pixels = Color16(*p & 255, *(p + 1) & 255, *(p + 2) & 255);
            ++for_pixels;
        }
    }

    delete[] sPixels;

    SavePCXImage16(a1, (uint16_t *)pPixels, render->GetRenderWidth(), render->GetRenderHeight());
    free(pPixels);
}

void RenderOpenGL::SavePCXImage16(const std::string &filename, uint16_t *picture_data, int width, int height) {
    // TODO(pskelton): add "Screenshots" folder?
    std::string thispath = MakeDataPath(filename);
    FILE *result = fopen(thispath.c_str(), "wb");
    if (result == nullptr) {
        return;
    }

    unsigned int pcx_data_size = width * height * 5;
    uint8_t *pcx_data = new uint8_t[pcx_data_size];
    unsigned int pcx_data_real_size = 0;
    PCX::Encode16(picture_data, width, height, pcx_data, pcx_data_size,
        &pcx_data_real_size);
    fwrite(pcx_data, pcx_data_real_size, 1, result);
    delete[] pcx_data;
    fclose(result);
}


bool RenderOpenGL::InitializeFullscreen() {
    // pViewport->ResetScreen();
    // CreateZBuffer();

    return true;
}

unsigned int RenderOpenGL::GetActorTintColor(int DimLevel, int tint, float WorldViewX, int a5, RenderBillboard *Billboard) {
    // GetActorTintColor(int max_dimm, int min_dimm, float distance, int a4, RenderBillboard *a5)
    return ::GetActorTintColor(DimLevel, tint, WorldViewX, a5, Billboard);
}


// when losing and regaining window focus - not required for OGL??
void RenderOpenGL::RestoreFrontBuffer() { logger->Info("RenderGl - RestoreFrontBuffer"); }
void RenderOpenGL::RestoreBackBuffer() { logger->Info("RenderGl - RestoreBackBuffer"); }

void RenderOpenGL::BltBackToFontFast(int a2, int a3, Recti *a4) {
    logger->Info("RenderGl - BltBackToFontFast");
    // never called anywhere
}



unsigned int RenderOpenGL::GetRenderWidth() const { return output.w; }
unsigned int RenderOpenGL::GetRenderHeight() const { return output.h; }

void RenderOpenGL::ClearBlack() {  // used only at start and in game over win
    ClearZBuffer();
    ClearTarget(0);
}

void RenderOpenGL::ClearTarget(unsigned int uColor) {
    glClearColor(0, 0, 0, 0/*0.9f, 0.5f, 0.1f, 1.0f*/);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}



void RenderOpenGL::CreateZBuffer() {
    if (pActiveZBuffer)
        free(pActiveZBuffer);

    pActiveZBuffer = (int*)malloc(output.w * output.h * sizeof(int));
    if (!pActiveZBuffer)
        Error("Failed to create zbuffer");

    ClearZBuffer();
}

void RenderOpenGL::ClearZBuffer() {
    memset32(this->pActiveZBuffer, 0xFFFF0000, output.w * output.h);
}

struct linesverts {
    GLfloat x;
    GLfloat y;
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

linesverts lineshaderstore[2000] = {};
int linevertscnt = 0;

void RenderOpenGL::BeginLines2D() {
    if (linevertscnt && engine->config->debug.VerboseLogging.Get())
        logger->Warning("BeginLines with points still stored in buffer");

    DrawTwodVerts();

    if (lineVAO == 0) {
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);

        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(lineshaderstore), NULL, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(linesverts), (void *)offsetof(linesverts, x));
        glEnableVertexAttribArray(0);
        // colour attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(linesverts), (void *)offsetof(linesverts, r));
        glEnableVertexAttribArray(1);
    }
}

void RenderOpenGL::EndLines2D() {
    if (!linevertscnt) return;

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineshaderstore), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(linesverts) * linevertscnt, lineshaderstore);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(lineVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glUseProgram(lineshader.ID);

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(lineshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(lineshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    glDrawArrays(GL_LINES, 0, (linevertscnt));
    drawcalls++;

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    linevertscnt = 0;
}

void RenderOpenGL::RasterLine2D(signed int uX, signed int uY, signed int uZ,
                                signed int uW, uint16_t uColor) {
    float b = ((uColor & 0x1F)*8) / 255.0f;
    float g = (((uColor >> 5) & 0x3F)*4) / 255.0f;
    float r = (((uColor >> 11) & 0x1F)*8) / 255.0f;

    lineshaderstore[linevertscnt].x = static_cast<float>(uX);
    lineshaderstore[linevertscnt].y = static_cast<float>(uY);
    lineshaderstore[linevertscnt].r = r;
    lineshaderstore[linevertscnt].g = g;
    lineshaderstore[linevertscnt].b = b;
    linevertscnt++;

    lineshaderstore[linevertscnt].x = static_cast<float>(uZ);
    lineshaderstore[linevertscnt].y = static_cast<float>(uW);
    lineshaderstore[linevertscnt].r = r;
    lineshaderstore[linevertscnt].g = g;
    lineshaderstore[linevertscnt].b = b;
    linevertscnt++;

    // draw if buffer full
    if (linevertscnt == 2000) EndLines2D();
}

void RenderOpenGL::BeginSceneD3D() {
    // Setup for 3D

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 0/*0.9f, 0.5f, 0.1f, 1.0f*/);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render->uNumBillboardsToDraw = 0;  // moved from drawbillboards - cant reset this until mouse picking finished

    SetFogParametersGL();
    gamma = GetGamma();
}

extern unsigned int BlendColors(unsigned int a1, unsigned int a2);

//----- (004A4CC9) ---------------------------------------
void RenderOpenGL::BillboardSphereSpellFX(struct SpellFX_Billboard *a1, int diffuse) {
    // fireball / implosion sphere
    // TODO(pskelton): could draw in 3d rather than convert to billboard for ogl

    if (a1->uNumVertices < 3) {
        return;
    }

    float depth = 1000000.0;
    for (uint i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        if (a1->field_104[i].z < depth) {
            depth = a1->field_104[i].z;
        }
    }

    unsigned int v5 = Billboard_ProbablyAddToListAndSortByZOrder(depth);
    pBillboardRenderListD3D[v5].field_90 = 0;
    pBillboardRenderListD3D[v5].sParentBillboardID = -1;
    pBillboardRenderListD3D[v5].opacity = RenderBillboardD3D::Opaque_2;
    pBillboardRenderListD3D[v5].texture = 0;
    pBillboardRenderListD3D[v5].uNumVertices = a1->uNumVertices;
    pBillboardRenderListD3D[v5].z_order = depth;

    pBillboardRenderListD3D[v5].pQuads[3].pos.x = 0.0f;
    pBillboardRenderListD3D[v5].pQuads[3].pos.y = 0.0f;
    pBillboardRenderListD3D[v5].pQuads[3].pos.z = 0.0f;

    for (unsigned int i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        pBillboardRenderListD3D[v5].pQuads[i].pos.x = a1->field_104[i].x;
        pBillboardRenderListD3D[v5].pQuads[i].pos.y = a1->field_104[i].y;
        pBillboardRenderListD3D[v5].pQuads[i].pos.z = a1->field_104[i].z;

        float rhw = 1.f / a1->field_104[i].z;
        float z = 1.f - 1.f / (a1->field_104[i].z * 1000.f / pCamera3D->GetFarClip());

        double v10 = a1->field_104[i].z;
        v10 *= 1000.f / pCamera3D->GetFarClip();

        pBillboardRenderListD3D[v5].pQuads[i].rhw = rhw;

        int v12;
        if (diffuse & 0xFF000000) {
            v12 = a1->field_104[i].diffuse;
        } else {
            v12 = diffuse;
        }
        pBillboardRenderListD3D[v5].pQuads[i].diffuse = v12;
        pBillboardRenderListD3D[v5].pQuads[i].specular = 0;

        pBillboardRenderListD3D[v5].pQuads[i].texcoord.x = 0.5;
        pBillboardRenderListD3D[v5].pQuads[i].texcoord.y = 0.5;
    }
}

struct forcepersverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
    GLfloat u;
    GLfloat v;
    GLfloat q;  // rhw
    GLfloat screenspace;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    GLfloat texid;
};

const int MAX_FORCEPERSTORECNT{ 498 };
forcepersverts forceperstore[MAX_FORCEPERSTORECNT]{};
int forceperstorecnt{ 0 };


void RenderOpenGL::DrawProjectile(float srcX, float srcY, float srcworldview, float srcfovoworldview,
                                  float dstX, float dstY, float dstworldview, float dstfovoworldview,
                                  Texture *texture) {
    // billboards projectile - lightning bolt

    TextureOpenGL *textured3d = (TextureOpenGL *)texture;

    int xDifference = bankersRounding(dstX - srcX);
    int yDifference = bankersRounding(dstY - srcY);
    int absYDifference = abs(yDifference);
    int absXDifference = abs(xDifference);
    unsigned int smallerabsdiff = std::min(absXDifference, absYDifference);
    unsigned int largerabsdiff = std::max(absXDifference, absYDifference);

    // distance approx
    int distapprox = (11 * smallerabsdiff >> 5) + largerabsdiff;

    float v16 = 1.0f / (float)distapprox;
    float srcxmod = (float)yDifference * v16 * srcfovoworldview;
    float srcymod = (float)xDifference * v16 * srcfovoworldview;

    float v20 = srcworldview * 4000.0f / pCamera3D->GetNearClip() / pCamera3D->GetFarClip();
    float v25 = dstworldview * 4000.0f / pCamera3D->GetNearClip() / pCamera3D->GetFarClip();
    float srcrhw = 1.0f / srcworldview;
    float dstxmod = (float)yDifference * v16 * dstfovoworldview;
    float dstymod = (float)xDifference * v16 * dstfovoworldview;
    float srcz = 1.0f - 1.0f / v20;
    float dstz = 1.0f - 1.0f / v25;
    float dstrhw = 1.0f / dstworldview;


    RenderVertexD3D3 v29[4];
    v29[0].pos.x = srcX + srcxmod;
    v29[0].pos.y = srcY - srcymod;
    v29[0].pos.z = srcz;
    v29[0].rhw = srcrhw;
    v29[0].diffuse = -1;
    v29[0].specular = 0;
    v29[0].texcoord.x = 1.0;
    v29[0].texcoord.y = 0.0;

    v29[1].pos.x = dstxmod + dstX;
    v29[1].pos.y = dstY - dstymod;
    v29[1].pos.z = dstz;
    v29[1].rhw = dstrhw;
    v29[1].diffuse = -16711936;
    v29[1].specular = 0;
    v29[1].texcoord.x = 1.0;
    v29[1].texcoord.y = 1.0;

    v29[2].pos.x = dstX - dstxmod;
    v29[2].pos.y = dstymod + dstY;
    v29[2].pos.z = dstz;
    v29[2].rhw = dstrhw;
    v29[2].diffuse = -1;
    v29[2].specular = 0;
    v29[2].texcoord.x = 0.0;
    v29[2].texcoord.y = 1.0;

    v29[3].pos.x = srcX - srcxmod;
    v29[3].pos.y = srcymod + srcY;
    v29[3].pos.z = srcz;
    v29[3].rhw = srcrhw;
    v29[3].diffuse = -1;
    v29[3].specular = 0;
    v29[3].texcoord.x = 0.0;
    v29[3].texcoord.y = 0.0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    auto texid{ 0 };

    if (textured3d) {
        texid = textured3d->GetOpenGlTexture();
    }

    // load up poly
    for (int z = 0; z < 2; z++) {
        // 123, 134, 145, 156..
        forcepersverts *thisvert = &forceperstore[forceperstorecnt];

        // copy first
        thisvert->x = v29[0].pos.x;
        thisvert->y = v29[0].pos.y;
        thisvert->z = v29[0].pos.z;
        thisvert->w = 1.0f;
        thisvert->u = v29[0].texcoord.x;
        thisvert->v = v29[0].texcoord.y;
        thisvert->q = v29[0].rhw;
        thisvert->screenspace = srcworldview;
        thisvert->r = 1.0f;
        thisvert->g = 1.0f;
        thisvert->b = 1.0f;
        thisvert->a = 1.0f;
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (uint i = 1; i < 3; ++i) {
            thisvert->x = v29[z + i].pos.x;
            thisvert->y = v29[z + i].pos.y;
            thisvert->z = v29[z + i].pos.z;
            thisvert->w = 1.0f;
            thisvert->u = v29[z + i].texcoord.x;
            thisvert->v = v29[z + i].texcoord.y;
            thisvert->q = v29[z + i].rhw;
            thisvert->screenspace = (z + i == 3) ? srcworldview: dstworldview;
            thisvert->r = 1.0f;
            thisvert->g = 1.0f;
            thisvert->b = 1.0f;
            thisvert->a = 1.0f;
            thisvert->texid = texid;
            thisvert++;
        }

        forceperstorecnt += 3;
        assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
    }

    // TODO(pskelton): do these need batching?
    DrawForcePerVerts();

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    //ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

struct twodverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat u;
    GLfloat v;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    GLfloat texid;
    GLfloat paletteid;
};

twodverts twodshaderstore[500] = {};
int twodvertscnt = 0;

void RenderOpenGL::ScreenFade(unsigned int color, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    //color is 24bits r8g8b8?
    float red = ((color & 0xFF0000) >> 16) / 255.0f;
    float green = ((color & 0xFF00) >> 8) / 255.0f;
    float blue = ((color & 0xFF)) / 255.0f;

    float drawx = static_cast<float>(pViewport->uViewportTL_X);
    float drawy = static_cast<float>(pViewport->uViewportTL_Y);
    float drawz = static_cast<float>(pViewport->uViewportBR_X);
    float draww = static_cast<float>(pViewport->uViewportBR_Y);

    static Texture *effpar03 = assets->GetBitmap("effpar03");
    auto texture = (TextureOpenGL *)effpar03;
    float gltexid = static_cast<float>(texture->GetOpenGlTexture());

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = 0;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].r = red;
    twodshaderstore[twodvertscnt].g = green;
    twodshaderstore[twodvertscnt].b = blue;
    twodshaderstore[twodvertscnt].a = t;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


void RenderOpenGL::DrawTextureOffset(int pX, int pY, int move_X, int move_Y,
                                     Image *pTexture) {
    DrawTextureNew((float)(pX - move_X)/output.w, (float)(pY - move_Y)/output.h, pTexture);
}



void RenderOpenGL::DrawImage(Image *img, const Recti &rect, uint paletteid) {
    if (!img) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Warning("Null img passed to DrawImage");
        return;
    }

    int width = img->GetWidth();
    int height = img->GetHeight();

    int x = rect.x;
    int y = rect.y;
    int z = rect.x + rect.w;
    int w = rect.y + rect.h;

    // check bounds
    if (x >= output.w || x >= this->clip_z || y >= output.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    auto texture = (TextureOpenGL*)img;
    float gltexid = static_cast<float>(texture->GetOpenGlTexture());

    float drawx = static_cast<float>(std::max(x, this->clip_x));
    float drawy = static_cast<float>(std::max(y, this->clip_y));
    float draww = static_cast<float>(std::min(w, this->clip_w));
    float drawz = static_cast<float>(std::min(z, this->clip_z));

    float texx = (drawx - x) / float(z - x);
    float texy = (drawy - y) / float(w - y);
    float texz = (drawz - x) / float(z - x);
    float texw = (draww - y) / float(w - y);

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = 1;
    twodshaderstore[twodvertscnt].g = 1;
    twodshaderstore[twodvertscnt].b = 1;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


void RenderOpenGL::ZDrawTextureAlpha(float u, float v, Image *img, int zVal) {
    if (!img) return;

    int uOutX = static_cast<int>(u * output.w);
    int uOutY = static_cast<int>(v * output.h);
    int imgheight = img->GetHeight();
    int imgwidth = img->GetWidth();
    auto pixels = (uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (uOutX < 0)
        uOutX = 0;
    if (uOutY < 0)
        uOutY = 0;

    for (int xs = 0; xs < imgwidth; xs++) {
        for (int ys = 0; ys < imgheight; ys++) {
            if (pixels[xs + imgwidth * ys] & 0xFF000000) {
                this->pActiveZBuffer[uOutX + xs + output.w * (uOutY + ys)] = zVal;
            }
        }
    }
}


// TODO(pskelton): sort this - forcing the draw is slow
// TODO(pskelton): stencil masking with opacity would be a better way to do this
void RenderOpenGL::BlendTextures(int x, int y, Image* imgin, Image* imgblend, int time, int start_opacity,
    int end_opacity) {
    // thrown together as a crude estimate of the enchaintg effects
    // leaves gap where it shouldnt on dark pixels currently
    // doesnt use opacity params

    const uint32_t* pixelpoint;
    const uint32_t* pixelpointblend;

    if (imgin && imgblend) {  // 2 images to blend
        pixelpoint = (const uint32_t*)imgin->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        pixelpointblend =
            (const uint32_t*)imgblend->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        int Width = imgin->GetWidth();
        int Height = imgin->GetHeight();
        Texture *temp = render->CreateTexture_Blank(Width, Height, IMAGE_FORMAT_A8R8G8B8);
        //Image* temp = Image::Create(Width, Height, IMAGE_FORMAT_A8R8G8B8);
        uint32_t* temppix = (uint32_t*)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        uint32_t c = *(pixelpointblend + 2700);  // guess at brightest pixel
        unsigned int bmax = (c & 0xFF);
        unsigned int gmax = ((c >> 8) & 0xFF);
        unsigned int rmax = ((c >> 16) & 0xFF);

        unsigned int bmin = bmax / 10;
        unsigned int gmin = gmax / 10;
        unsigned int rmin = rmax / 10;

        unsigned int bstep = (bmax - bmin) / 128;
        unsigned int gstep = (gmax - gmin) / 128;
        unsigned int rstep = (rmax - rmin) / 128;

        for (int ydraw = 0; ydraw < Height; ++ydraw) {
            for (int xdraw = 0; xdraw < Width; ++xdraw) {
                // should go blue -> black -> blue reverse
                // patchy -> solid -> patchy

                if (*pixelpoint) {  // check orig item not got blakc pixel
                    uint32_t nudge =
                        (xdraw % imgblend->GetWidth()) +
                        (ydraw % imgblend->GetHeight()) * imgblend->GetWidth();
                    uint32_t pixcol = *(pixelpointblend + nudge);

                    unsigned int bcur = (pixcol & 0xFF);
                    unsigned int gcur = ((pixcol >> 8) & 0xFF);
                    unsigned int rcur = ((pixcol >> 16) & 0xFF);

                    int steps = (time) % 128;

                    if ((time) % 256 >= 128) {  // step down
                        bcur += bstep * (128 - steps);
                        gcur += gstep * (128 - steps);
                        rcur += rstep * (128 - steps);
                    } else {  // step up
                        bcur += bstep * steps;
                        gcur += gstep * steps;
                        rcur += rstep * steps;
                    }

                    if (bcur > bmax) bcur = bmax;  // limit check
                    if (gcur > gmax) gcur = gmax;
                    if (rcur > rmax) rcur = rmax;
                    if (bcur < bmin) bcur = bmin;
                    if (gcur < gmin) gcur = gmin;
                    if (rcur < rmin) rcur = rmin;

                    temppix[xdraw + ydraw * Width] = Color32(rcur, gcur, bcur);
                }

                pixelpoint++;
            }

            pixelpoint += imgin->GetWidth() - Width;
        }
        // draw image
        render->Update_Texture(temp);
        render->DrawTextureAlphaNew(x / float(output.w), y / float(output.h), temp);

        render->DrawTwodVerts();

        temp->Release();
    }
}

//----- (004A65CC) --------------------------------------------------------
//_4A65CC(unsigned int x, unsigned int y, Texture_MM7 *a4, Texture_MM7 *a5, int a6, int a7, int a8)
// a6 is time, a7 is 0, a8 is 63
void RenderOpenGL::TexturePixelRotateDraw(float u, float v, Image *img, int time) {
    // TODO(pskelton): sort this - precalculate/ shader
    static std::array<Texture *, 14> cachedtemp {};
    static std::array<int, 14> cachetime { -1 };

    if (img) {
        std::string_view tempstr{ *img->GetName() };
        int number = tempstr[4] - 48;
        int number2 = tempstr[5] - 48;

        int thisslot = 10 * number + number2 - 1;
        if (cachetime[thisslot] != time) {
            int width = img->GetWidth();
            int height = img->GetHeight();
            if (!cachedtemp[thisslot]) {
                cachedtemp[thisslot] = CreateTexture_Blank(width, height, IMAGE_FORMAT_A8R8G8B8);
            }

            uint8_t *palpoint24 = (uint8_t *)img->GetPalette();
            uint32_t *temppix = (uint32_t *)cachedtemp[thisslot]->GetPixels(IMAGE_FORMAT_A8R8G8B8);
            uint8_t *texpix24 = (uint8_t *)img->GetPalettePixels();
            uint8_t thispix;
            int palindex;

            for (int dy = 0; dy < height; ++dy) {
                for (int dx = 0; dx < width; ++dx) {
                    thispix = *texpix24;
                    if (thispix >= 0 && thispix <= 63) {
                        palindex = (time + thispix) % (2 * 63);
                        if (palindex >= 63)
                            palindex = (2 * 63) - palindex;
                        temppix[dx + dy * width] = Color32(palpoint24[palindex * 3], palpoint24[palindex * 3 + 1], palpoint24[palindex * 3 + 2]);
                    }
                    ++texpix24;
                }
            }
            cachetime[thisslot] = time;
            render->Update_Texture(cachedtemp[thisslot]);
        }

        render->DrawTextureAlphaNew(u, v, cachedtemp[thisslot]);
    }
}

void RenderOpenGL::DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait, int Y_Offset) {
    Recti rct;
    rct.x = rc.x + 64 + Portrait->hw_sprites[0]->uAreaX - Portrait->hw_sprites[0]->uBufferWidth / 2;
    rct.y = rc.y + Y_Offset + Portrait->hw_sprites[0]->uAreaY;
    rct.w = Portrait->hw_sprites[0]->uAreaWidth;
    rct.h = Portrait->hw_sprites[0]->uAreaHeight;

    render->SetUIClipRect(rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
    render->DrawImage(Portrait->hw_sprites[0]->texture, rct, Portrait->GetPaletteIndex());
    render->ResetUIClipRect();
}

void RenderOpenGL::DrawTransparentRedShade(float u, float v, Image *a4) {
    DrawMasked(u, v, a4, 0, 0xF800);
}

void RenderOpenGL::DrawTransparentGreenShade(float u, float v, Image *pTexture) {
    DrawMasked(u, v, pTexture, 0, 0x07E0);
}

void RenderOpenGL::DrawMasked(float u, float v, Image *pTexture, unsigned int color_dimming_level, uint16_t mask) {
    uint32_t col = colorTable.White.C32();

    if (mask)
        col = Color32(mask);

    int r = ((col >> 16) & 0xFF) & (0xFF>> color_dimming_level);
    int g = ((col >> 8) & 0xFF) & (0xFF >> color_dimming_level);
    int b = ((col) & 0xFF) & (0xFF >> color_dimming_level);

    col = Color32(r, g, b);

    DrawTextureNew(u, v, pTexture, col);
    return;
}



void RenderOpenGL::DrawTextureGrayShade(float a2, float a3, Image *a4) {
    DrawMasked(a2, a3, a4, 1, 0x7BEF);
}

void RenderOpenGL::DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID) {
    BLVFace *pFace = &pIndoor->pFaces[uFaceID];
    if (pFace->uNumVertices <= 0) return;

    struct Polygon pSkyPolygon;
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.texture = pFace->GetTexture();
    if (!pSkyPolygon.texture) return;

    pSkyPolygon.ptr_38 = &SkyBillboard;
    pSkyPolygon.dimming_level = 0;
    pSkyPolygon.uNumVertices = pFace->uNumVertices;


    // TODO(pskelton): repeated maths could be saved when calculating sky planes
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  blv_horizon_height_offset = ((pCamera3D->ViewPlaneDist_X * pCamera3D->vCameraPos.z)
        / (pCamera3D->ViewPlaneDist_X + pCamera3D->GetFarClip())
        + (pBLVRenderParams->uViewportCenterY));

    double cam_y_rot_rad = (double)pCamera3D->sRotationY * rot_to_rads;

    float depth_to_far_clip = static_cast<float>(cos(pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip());
    float height_to_far_clip = static_cast<float>(sin(pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip());

    float blv_bottom_y_proj = ((pBLVRenderParams->uViewportCenterY) -
        pCamera3D->ViewPlaneDist_X /
        (depth_to_far_clip + 0.0000001f) *
        (height_to_far_clip - pCamera3D->vCameraPos.z));

    // rotation vec for sky plane - pitch
    float v_18x = static_cast<float>(-sin((-pCamera3D->sRotationY + 16) * rot_to_rads));
    float v_18y = 0.0f;
    float v_18z = static_cast<float>(-cos((pCamera3D->sRotationY + 16) * rot_to_rads));

    float inv_viewplanedist = 1.0f / pCamera3D->ViewPlaneDist_X;

    // copy to buff in
    for (uint i = 0; i < pFace->uNumVertices; ++i) {
        array_507D30[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
        array_507D30[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
        array_507D30[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
        array_507D30[i].u = (signed short)pFace->pVertexUIDs[i];
        array_507D30[i].v = (signed short)pFace->pVertexVIDs[i];
    }

    // clip accurately to camera
    pCamera3D->ClipFaceToFrustum(array_507D30, &pSkyPolygon.uNumVertices, VertexRenderList, pBspRenderer->nodes[0].ViewportNodeFrustum, 4, 0, 0);
    if (!pSkyPolygon.uNumVertices) return;

    pCamera3D->ViewTransform(VertexRenderList, pSkyPolygon.uNumVertices);
    pCamera3D->Project(VertexRenderList, pSkyPolygon.uNumVertices, false);

    uint _507D30_idx = 0;
    for (; _507D30_idx < pSkyPolygon.uNumVertices; _507D30_idx++) {
        // outbound screen x dist
        float x_dist = inv_viewplanedist * (pBLVRenderParams->uViewportCenterX - VertexRenderList[_507D30_idx].vWorldViewProjX);
        // outbound screen y dist
        float y_dist = inv_viewplanedist * (blv_horizon_height_offset - VertexRenderList[_507D30_idx].vWorldViewProjY);

        // rotate vectors to cam facing
        float skyfinalleft = (pSkyPolygon.ptr_38->CamVecLeft_X * x_dist) + (pSkyPolygon.ptr_38->CamVecLeft_Z * y_dist) + pSkyPolygon.ptr_38->CamVecLeft_Y;
        float skyfinalfront = (pSkyPolygon.ptr_38->CamVecFront_X * x_dist) + (pSkyPolygon.ptr_38->CamVecFront_Z * y_dist) + pSkyPolygon.ptr_38->CamVecFront_Y;

        // pitch rotate sky to get top projection
        float newX = v_18x + v_18y + (v_18z * y_dist);
        float worldviewdepth = -512.0f / newX;

        // offset tex coords
        float texoffset_U = ((pMiscTimer->uTotalGameTimeElapsed) / 128.0f) + ((skyfinalleft * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].u = texoffset_U / (pSkyPolygon.texture->GetWidth());
        float texoffset_V = ((pMiscTimer->uTotalGameTimeElapsed) / 128.0f) + ((skyfinalfront * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].v = texoffset_V / (pSkyPolygon.texture->GetHeight());

        // this basically acts as texture perspective correction
        VertexRenderList[_507D30_idx]._rhw = worldviewdepth;
    }

    // no clipped polygon so draw and return??
    if (_507D30_idx >= pSkyPolygon.uNumVertices) {
        DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon);
        return;
    }

    logger->Info("past normal section");
    __debugbreak();
    // please provide save game / details if you get here
}

void RenderOpenGL::DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon) {
    auto texture = (TextureOpenGL*)pSkyPolygon->texture;
    auto texid = texture->GetOpenGlTexture();

    uint uTint = GetActorTintColor(pSkyPolygon->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0);
    uint uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF, uTintB = uTint & 0xFF;
    float scrspace{ pCamera3D->GetFarClip() };

    float oneon = 1.0f / (pCamera3D->GetNearClip() * 2.0f);
    float oneof = 1.0f / (pCamera3D->GetFarClip());

    // load up poly
    for (int z = 0; z < (pSkyPolygon->uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        forcepersverts* thisvert = &forceperstore[forceperstorecnt];
        float oneoz = 1.0f / VertexRenderList[0].vWorldViewPosition.x;
        float thisdepth = (oneoz - oneon) / (oneof - oneon);
        // copy first
        thisvert->x = VertexRenderList[0].vWorldViewProjX;
        thisvert->y = VertexRenderList[0].vWorldViewProjY;
        thisvert->z = thisdepth;
        thisvert->w = VertexRenderList[0]._rhw;
        thisvert->u = VertexRenderList[0].u;
        thisvert->v = VertexRenderList[0].v;
        thisvert->q = 1.0f;
        thisvert->screenspace = scrspace;
        thisvert->r = (uTintR) / 255.0f;
        thisvert->g = (uTintG) / 255.0f;
        thisvert->b = (uTintB) / 255.0f;
        thisvert->a = 1.0f;
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (uint i = 1; i < 3; ++i) {
            oneoz = 1.0f / VertexRenderList[z + i].vWorldViewPosition.x;
            thisdepth = (oneoz - oneon) / (oneof - oneon);
            thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
            thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
            thisvert->z = thisdepth;
            thisvert->w = VertexRenderList[z + i]._rhw;
            thisvert->u = VertexRenderList[z + i].u;
            thisvert->v = VertexRenderList[z + i].v;
            thisvert->q = 1.0f;
            thisvert->screenspace = scrspace;
            thisvert->r = (uTintR) / 255.0f;
            thisvert->g = (uTintG) / 255.0f;
            thisvert->b = (uTintB) / 255.0f;
            thisvert->a = 1.0f;
            thisvert->texid = texid;
            thisvert++;
        }

        forceperstorecnt += 3;
        // TODO (pskelton): should force drawing if buffer is full
        assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
    }
}

bool RenderOpenGL::AreRenderSurfacesOk() {
    logger->Info("RenderGl - AreRenderSurfacesOk");
    return true;
}

unsigned short *RenderOpenGL::MakeScreenshot16(int width, int height) {
    BeginSceneD3D();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        pOutdoor->Draw();
    }

    DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();

    GLubyte *sPixels = new GLubyte[3 * output.w * output.h];
    glReadPixels(0, 0, output.w, output.h, GL_RGB, GL_UNSIGNED_BYTE, sPixels);

    int interval_x = static_cast<int>(game_viewport_width / (double)width);
    int interval_y = static_cast<int>(game_viewport_height / (double)height);

    uint16_t *pPixels = (uint16_t *)malloc(sizeof(uint16_t) * height * width);
    memset(pPixels, 0, sizeof(uint16_t) * height * width);

    uint16_t *for_pixels = pPixels;

    if (uCurrentlyLoadedLevelType == LEVEL_null) {
        memset(&for_pixels, 0, sizeof(for_pixels));
    } else {
        for (uint y = 0; y < (unsigned int)height; ++y) {
            for (uint x = 0; x < (unsigned int)width; ++x) {
                uint8_t *p;

                p = sPixels + 3 * (int)(x * interval_x + 8.0) + 3 * (int)(output.h - (y * interval_y) - 8.0) * output.w;

                *for_pixels = Color16(*p & 255, *(p + 1) & 255, *(p + 2) & 255);
                ++for_pixels;
            }
        }
    }

    delete [] sPixels;
    return pPixels;
}

Image *RenderOpenGL::TakeScreenshot(unsigned int width, unsigned int height) {
    auto pixels = MakeScreenshot16(width, height);
    Image *image = Image::Create(width, height, IMAGE_FORMAT_R5G6B5, pixels);
    free(pixels);
    return image;
}

void RenderOpenGL::SaveScreenshot(const std::string &filename, unsigned int width, unsigned int height) {
    auto pixels = MakeScreenshot16(width, height);

    std::string thispath = MakeDataPath(filename);
    FILE *result = fopen(thispath.c_str(), "wb");
    if (result == nullptr) {
        return;
    }

    unsigned int pcx_data_size = width * height * 5;
    uint8_t *pcx_data = new uint8_t[pcx_data_size];
    unsigned int pcx_data_real_size = 0;
    PCX::Encode16(pixels, width, height, pcx_data, pcx_data_size, &pcx_data_real_size);
    fwrite(pcx_data, pcx_data_real_size, 1, result);
    delete[] pcx_data;
    fclose(result);
}

void RenderOpenGL::PackScreenshot(unsigned int width, unsigned int height,
                                  void *out_data, unsigned int data_size,
                                  unsigned int *screenshot_size) {
    auto pixels = MakeScreenshot16(width, height);
    SaveScreenshot("save.pcx", width, height);
    PCX::Encode16(pixels, 150, 112, out_data, 1000000, screenshot_size);
    free(pixels);
}

void RenderOpenGL::SavePCXScreenshot() {
    size_t zeros_number = 5;
    std::string screenshot_number = std::to_string(engine->config->settings.ScreenshotNumber.Increment());
    std::string file_name = "screenshot_" + std::string(zeros_number - std::min(zeros_number, screenshot_number.length()), '0') + screenshot_number + ".pcx";

    SaveWinnersCertificate(file_name.c_str());
}


// TODO: should this be combined / moved out of render
int RenderOpenGL::GetActorsInViewport(int pDepth) {
    int
        v3;  // eax@2 применяется в закле Жар печи для подсчёта кол-ва монстров
             // видимых группе и заполнения массива id видимых монстров
    unsigned int v5;   // eax@2
    unsigned int v6;   // eax@4
    unsigned int v12;  // [sp+10h] [bp-14h]@1
    int mon_num;       // [sp+1Ch] [bp-8h]@1
    unsigned int a1a;  // [sp+20h] [bp-4h]@1

    mon_num = 0;
    v12 = render->uNumBillboardsToDraw;
    if ((signed int)render->uNumBillboardsToDraw > 0) {
        for (a1a = 0; (signed int)a1a < (signed int)v12; ++a1a) {
            v3 = render->pBillboardRenderListD3D[a1a].sParentBillboardID;
            if(v3 == -1)
                continue; // E.g. spell particle.

            v5 = (uint16_t)pBillboardRenderList[v3].object_pid;
            if (PID_TYPE(v5) == OBJECT_Actor) {
                if (pBillboardRenderList[v3].screen_space_z <= pDepth) {
                    v6 = PID_ID(v5);
                    if (pActors[v6].uAIState != Dead &&
                        pActors[v6].uAIState != Dying &&
                        pActors[v6].uAIState != Removed &&
                        pActors[v6].uAIState != Disabled &&
                        pActors[v6].uAIState != Summoned) {
                        if (vis->DoesRayIntersectBillboard(static_cast<float>(pDepth), a1a)) {
                            if (mon_num < 100) {
                                _50BF30_actors_in_viewport_ids[mon_num] = v6;
                                mon_num++;
                            }
                        }
                    }
                }
            }
        }
    }
    return mon_num;
}

// not required in gl renderer now
void RenderOpenGL::BeginLightmaps() { return; }
void RenderOpenGL::EndLightmaps() { return; }
void RenderOpenGL::BeginLightmaps2() { return; }
void RenderOpenGL::EndLightmaps2() { return; }
bool RenderOpenGL::DrawLightmap(struct Lightmap *pLightmap, Vec3f *pColorMult, float z_bias) {
    return true;
}

struct GLdecalverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat u;
    GLfloat v;
    GLfloat texunit;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GLfloat attribs;
};

GLdecalverts decalshaderstore[10000] = {};
int numdecalverts{ 0 };


void RenderOpenGL::BeginDecals() {
    auto texture = (TextureOpenGL*)assets->GetBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // gen buffers

    if (decalVAO == 0) {
        glGenVertexArrays(1, &decalVAO);
        glGenBuffers(1, &decalVBO);

        glBindVertexArray(decalVAO);
        glBindBuffer(GL_ARRAY_BUFFER, decalVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(GLdecalverts) * 10000, decalshaderstore, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLdecalverts), (void *)offsetof(GLdecalverts, x));
        glEnableVertexAttribArray(0);
        // tex uv attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLdecalverts), (void *)offsetof(GLdecalverts, u));
        glEnableVertexAttribArray(1);
        // tex unit attribute
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLdecalverts), (void *)offsetof(GLdecalverts, texunit));
        glEnableVertexAttribArray(2);
        // colours
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLdecalverts), (void *)offsetof(GLdecalverts, red));
        glEnableVertexAttribArray(3);
        // attribs - not used here yet
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GLdecalverts), (void *)offsetof(GLdecalverts, attribs));
        glEnableVertexAttribArray(4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    numdecalverts = 0;
}

void RenderOpenGL::EndDecals() {
    // draw here

    if (numdecalverts) {
            glBindBuffer(GL_ARRAY_BUFFER, decalVBO);
            // orphan buffer
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLdecalverts) * 10000, NULL, GL_DYNAMIC_DRAW);
            // update buffer
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLdecalverts) * numdecalverts, decalshaderstore);
    } else {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // ?
    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    glUseProgram(decalshader.ID);
    // set projection
    glUniformMatrix4fv(glGetUniformLocation(decalshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    // set view
    glUniformMatrix4fv(glGetUniformLocation(decalshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    // set fog uniforms
    glUniform3f(glGetUniformLocation(decalshader.ID, "fog.color"), fogr, fogg, fogb);
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogend"), GLfloat(fogend));

    // set bias
    glUniform1f(glGetUniformLocation(decalshader.ID, "decalbias"), GLfloat(0.002f));

    // set texture unit location
    glUniform1i(glGetUniformLocation(decalshader.ID, "texture0"), GLint(0));
    glActiveTexture(GL_TEXTURE0);

    auto texture = (TextureOpenGL *)assets->GetBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    glBindVertexArray(decalVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glDrawArrays(GL_TRIANGLES, 0, numdecalverts);
    drawcalls++;

    // unload
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindVertexArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NULL);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}



void RenderOpenGL::DrawDecal(struct Decal *pDecal, float z_bias) {
    if (pDecal->uNumVertices < 3) {
        log->Warning("Decal has < 3 vertices");
        return;
    }

    float color_mult = pDecal->Fade_by_time();
    if (color_mult == 0.0f) return;

    // temp - bloodsplat persistance
    // color_mult = 1;

    // load into buffer
    uint uDecalColorMultR = (pDecal->uColorMultiplier >> 16) & 0xFF,
        uDecalColorMultG = (pDecal->uColorMultiplier >> 8) & 0xFF,
        uDecalColorMultB = pDecal->uColorMultiplier & 0xFF;

    for (int z = 0; z < (pDecal->uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        GLdecalverts *thisvert = &decalshaderstore[numdecalverts];
        uint uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[0].vWorldViewPosition.x, 0, nullptr);
        uint uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF, uTintB = uTint & 0xFF;

        float uFinalR = floorf(uTintR / 255.0f * color_mult * uDecalColorMultR + 0.0f),
             uFinalG = floorf(uTintG / 255.0f * color_mult * uDecalColorMultG + 0.0f),
             uFinalB = floorf(uTintB / 255.0f * color_mult * uDecalColorMultB + 0.0f);

        // copy first
        thisvert->x = pDecal->pVertices[0].vWorldPosition.x;
        thisvert->y = pDecal->pVertices[0].vWorldPosition.y;
        thisvert->z = pDecal->pVertices[0].vWorldPosition.z;
        thisvert->u = pDecal->pVertices[0].u;
        thisvert->v = pDecal->pVertices[0].v;
        thisvert->texunit = 0;
        thisvert->red = (uFinalR) / 255.0f;
        thisvert->green = (uFinalG) / 255.0f;
        thisvert->blue = (uFinalB) / 255.0f;
        thisvert->attribs = 0;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (uint i = 1; i < 3; ++i) {
            uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[z + i].vWorldViewPosition.x, 0, nullptr);
            uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF, uTintB = uTint & 0xFF;
            uFinalR = floorf(uTintR / 255.0f * color_mult * uDecalColorMultR + 0.0f),
            uFinalG = floorf(uTintG / 255.0f * color_mult * uDecalColorMultG + 0.0f),
            uFinalB = floorf(uTintB / 255.0f * color_mult * uDecalColorMultB + 0.0f);

            thisvert->x = pDecal->pVertices[z + i].vWorldPosition.x;
            thisvert->y = pDecal->pVertices[z + i].vWorldPosition.y;
            thisvert->z = pDecal->pVertices[z + i].vWorldPosition.z;
            thisvert->u = pDecal->pVertices[z + i].u;
            thisvert->v = pDecal->pVertices[z + i].v;
            thisvert->texunit = 0;
            thisvert->red = (uFinalR) / 255.0f;
            thisvert->green = (uFinalG) / 255.0f;
            thisvert->blue = (uFinalB) / 255.0f;
            thisvert->attribs = 0;
            thisvert++;
        }

        numdecalverts += 3;
        assert(numdecalverts <= 9999);
    }
}

void RenderOpenGL::Do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin,
                                          signed int sDiffuseBegin,
                                          const RenderVertexD3D3 *pLineEnd,
                                          signed int sDiffuseEnd,
                                          float z_stuff) {
    // not required - using wireframe drawing for gl debug lines
}

// used for debug protal lines
void RenderOpenGL::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {
    BeginLines2D();
    for (uint i = 0; i < num_vertices - 1; ++i) {
        uint uColor = vertices[i].diffuse;
        float b1 = ((uColor & 0x1F) * 8) / 255.0f;
        float g1 = (((uColor >> 5) & 0x3F) * 4) / 255.0f;
        float r1 = (((uColor >> 11) & 0x1F) * 8) / 255.0f;

        uint uColor2 = vertices[i+1].diffuse;
        float b2 = ((uColor2 & 0x1F) * 8) / 255.0f;
        float g2 = (((uColor2 >> 5) & 0x3F) * 4) / 255.0f;
        float r2 = (((uColor2 >> 11) & 0x1F) * 8) / 255.0f;

        lineshaderstore[linevertscnt].x = vertices[i].pos.x;
        lineshaderstore[linevertscnt].y = vertices[i].pos.y;
        lineshaderstore[linevertscnt].r = r1;
        lineshaderstore[linevertscnt].g = g1;
        lineshaderstore[linevertscnt].b = b1;
        linevertscnt++;

        lineshaderstore[linevertscnt].x = vertices[i + 1].pos.x + 0.5f;
        lineshaderstore[linevertscnt].y = vertices[i + 1].pos.y + 0.5f;
        lineshaderstore[linevertscnt].r = r2;
        lineshaderstore[linevertscnt].g = g2;
        lineshaderstore[linevertscnt].b = b2;
        linevertscnt++;

        // draw if buffer full
        if (linevertscnt == 2000) EndLines2D();
    }
    EndLines2D();
}

void RenderOpenGL::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, Texture *texture) {
    //TODO(pskelton): need to add dimming  0x7F7F7F
    Recti targetrect{};
    targetrect.x = pViewport->uViewportTL_X;
    targetrect.y = pViewport->uViewportTL_Y;
    targetrect.w = pViewport->uViewportBR_X - pViewport->uViewportTL_X;
    targetrect.h = pViewport->uViewportBR_Y - pViewport->uViewportTL_Y;

    DrawImage(texture, targetrect);
}

void RenderOpenGL::DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3, int blend_mode) {
    // want to draw psrcrect section @ point

    TextureOpenGL *texture = (TextureOpenGL*)pArcomageGame->pSprites;

    if (!texture) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Warning("Missing Arcomage Sprite Sheet");
        return;
    }

    float col = (blend_mode == 2) ? 1.0f : 0.5f;
    float r = col;
    float g = col;
    float b = col;

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = pSrcRect->w;
    int height = pSrcRect->h;

    int x = pTargetPoint->x;
    int y = pTargetPoint->y;
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= output.w || x >= this->clip_z || y >= output.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = static_cast<float>(texture->GetOpenGlTexture());
    int texwidth = texture->GetWidth();
    int texheight = texture->GetHeight();

    float drawx = static_cast<float>(x);
    float drawy = static_cast<float>(y);
    float draww = static_cast<float>(w);
    float drawz = static_cast<float>(z);

    float texx = pSrcRect->x / float(texwidth);
    float texy = pSrcRect->y / float(texheight);
    float texz = (pSrcRect->x + pSrcRect->w) / float(texwidth);
    float texw = (pSrcRect->y + pSrcRect->h) / float(texheight);

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


void RenderOpenGL::PrepareDecorationsRenderList_ODM() {
    unsigned int v6;        // edi@9
    int v7;                 // eax@9
    SpriteFrame *frame;     // eax@9
    int v13;                // ecx@9
    int r;                 // ecx@20
    int g;                 // dl@20
    int b_;                // eax@20
    Particle_sw local_0;    // [sp+Ch] [bp-98h]@7
    int v38;                // [sp+88h] [bp-1Ch]@9

    for (unsigned int i = 0; i < pLevelDecorations.size(); ++i) {
        if (::uNumBillboardsToDraw >= 500) return;

        // view cull
        if (!IsCylinderInFrustum(pLevelDecorations[i].vPosition.ToFloat(), 512.0f)) continue;

        // LevelDecoration* decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST) ||
            pLevelDecorations[i].IsObeliskChestActive()) &&
            !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            const DecorationDesc *decor_desc = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);
            if (!(decor_desc->uFlags & DECORATION_DESC_EMITS_FIRE)) {
                if (!(decor_desc->uFlags & (DECORATION_DESC_MARKER | DECORATION_DESC_DONT_DRAW))) {
                    v6 = pMiscTimer->uTotalGameTimeElapsed;
                    v7 = abs(pLevelDecorations[i].vPosition.x +
                        pLevelDecorations[i].vPosition.y);

                    frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                        v6 + v7);

                    if (engine->config->graphics.SeasonsChange.Get()) {
                        frame = LevelDecorationChangeSeason(decor_desc, v6 + v7, pParty->uCurrentMonth);
                    }

                    if (!frame || frame->texture_name == "null" || frame->hw_sprites[0] == NULL) {
                        continue;
                    }

                    // v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                    // v6 + v7);

                    int v10 = TrigLUT.Atan2(
                        pLevelDecorations[i].vPosition.x -
                        pCamera3D->vCameraPos.x,
                        pLevelDecorations[i].vPosition.y -
                        pCamera3D->vCameraPos.y);
                    v38 = 0;
                    v13 = ((signed int)(TrigLUT.uIntegerPi +
                        ((signed int)TrigLUT.uIntegerPi >>
                            3) +
                        pLevelDecorations[i].field_10_y_rot -
                        (int64_t)v10) >>
                        8) &
                        7;
                    int v37 = v13;
                    if (frame->uFlags & 2) v38 = 2;
                    if ((256 << v13) & frame->uFlags) v38 |= 4;
                    if (frame->uFlags & 0x40000) v38 |= 0x40;
                    if (frame->uFlags & 0x20000) v38 |= 0x80;

                    // for light
                    if (frame->uGlowRadius) {
                        r = 255;
                        g = 255;
                        b_ = 255;
                        if (render->config->graphics.ColoredLights.Get()) {
                            r = decor_desc->uColoredLightRed;
                            g = decor_desc->uColoredLightGreen;
                            b_ = decor_desc->uColoredLightBlue;
                            // to avoid blank lights
                            if (!r && !g && !b_) {
                                r = g = b_ = 255;
                            }
                        }
                        pStationaryLightsStack->AddLight(
                                pLevelDecorations[i].vPosition.ToFloat() +
                                Vec3f(0, 0, decor_desc->uDecorationHeight / 2),
                            frame->uGlowRadius, r, g, b_, _4E94D0_light_type);
                    }  // for light

                       // v17 = (pLevelDecorations[i].vPosition.x -
                       // pCamera3D->vCameraPos.x) << 16; v40 =
                       // (pLevelDecorations[i].vPosition.y -
                       // pCamera3D->vCameraPos.y) << 16;
                    int party_to_decor_x = static_cast<int>(pLevelDecorations[i].vPosition.x - pCamera3D->vCameraPos.x);
                    int party_to_decor_y = static_cast<int>(pLevelDecorations[i].vPosition.y - pCamera3D->vCameraPos.y);
                    int party_to_decor_z = static_cast<int>(pLevelDecorations[i].vPosition.z - pCamera3D->vCameraPos.z);

                    int view_x = 0;
                    int view_y = 0;
                    int view_z = 0;
                    bool visible = pCamera3D->ViewClip(
                        pLevelDecorations[i].vPosition.x,
                        pLevelDecorations[i].vPosition.y,
                        pLevelDecorations[i].vPosition.z, &view_x, &view_y,
                        &view_z);

                    if (visible) {
                        if (2 * abs(view_x) >= abs(view_y)) {
                            int projected_x = 0;
                            int projected_y = 0;
                            pCamera3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

                            float _v41 = frame->scale * (pCamera3D->ViewPlaneDist_X) / (view_x);

                            int screen_space_half_width = static_cast<int>(_v41 * frame->hw_sprites[(int64_t)v37]->uBufferWidth / 2.0f);
                            int screen_space_height = static_cast<int>(_v41 * frame->hw_sprites[(int64_t)v37]->uBufferHeight);

                            if (projected_x + screen_space_half_width >= (signed int)pViewport->uViewportTL_X &&
                                projected_x - screen_space_half_width <= (signed int)pViewport->uViewportBR_X) {
                                if (projected_y >= pViewport->uViewportTL_Y && (projected_y - screen_space_height) <= pViewport->uViewportBR_Y) {
                                    ::uNumBillboardsToDraw++;
                                    ++uNumDecorationsDrawnThisFrame;

                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].hwsprite = frame->hw_sprites[(int64_t)v37];
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].world_x = pLevelDecorations[i].vPosition.x;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].world_y = pLevelDecorations[i].vPosition.y;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].world_z = pLevelDecorations[i].vPosition.z;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].screen_space_z = view_x;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].screenspace_projection_factor_x = _v41;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].screenspace_projection_factor_y = _v41;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].uPaletteIndex = frame->GetPaletteIndex();
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].field_1E = v38 | 0x200;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].uIndoorSectorID = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Decoration, i);
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].dimming_level = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].pSpriteFrame = frame;
                                    pBillboardRenderList[::uNumBillboardsToDraw - 1].sTintColor = 0;
                                }
                            }
                        }
                    }
                }
            } else {
                memset(&local_0, 0, sizeof(Particle_sw));
                local_0.type = ParticleType_Bitmap | ParticleType_Rotating |
                    ParticleType_8;
                local_0.uDiffuse = 0xFF3C1E;
                local_0.x = static_cast<float>(pLevelDecorations[i].vPosition.x);
                local_0.y = static_cast<float>(pLevelDecorations[i].vPosition.y);
                local_0.z = static_cast<float>(pLevelDecorations[i].vPosition.z);
                local_0.r = 0.0f;
                local_0.g = 0.0f;
                local_0.b = 0.0f;
                local_0.particle_size = 1.0f;
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

Texture *RenderOpenGL::CreateTexture_Paletted(const std::string &name) {
    return TextureOpenGL::Create(new Paletted_Img_Loader(pIcons_LOD, name, 0));
}

Texture *RenderOpenGL::CreateTexture_ColorKey(const std::string &name, uint16_t colorkey) {
    return TextureOpenGL::Create(new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey));
}

Texture *RenderOpenGL::CreateTexture_Solid(const std::string &name) {
    return TextureOpenGL::Create(new Image16bit_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_Alpha(const std::string &name) {
    return TextureOpenGL::Create(new Alpha_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromIconsLOD(const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Compressed_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromNewLOD(const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Compressed_Loader(pNew_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromFile(const std::string &name) {
    return TextureOpenGL::Create(new PCX_File_Loader(name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Raw_Loader(pLOD, name));
}

Texture *RenderOpenGL::CreateTexture_Blank(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels) {

    return TextureOpenGL::Create(width, height, format, pixels);
}


Texture *RenderOpenGL::CreateTexture(const std::string &name) {
    return TextureOpenGL::Create(new Bitmaps_LOD_Loader(pBitmaps_LOD, name, engine->config->graphics.HWLBitmaps.Get()));
}

Texture *RenderOpenGL::CreateSprite(const std::string &name, unsigned int palette_id,
                                    /*refactor*/ unsigned int lod_sprite_id) {
    return TextureOpenGL::Create(
        new Sprites_LOD_Loader(pSprites_LOD, palette_id, name, lod_sprite_id, engine->config->graphics.HWLSprites.Get()));
}

void RenderOpenGL::Update_Texture(Texture *texture) {
    // takes care of endian flip from literals here - hence BGRA

    auto t = (TextureOpenGL *)texture;
    glBindTexture(GL_TEXTURE_2D, t->GetOpenGlTexture());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->GetWidth(), t->GetHeight(), GL_BGRA, GL_UNSIGNED_BYTE, t->GetPixels(IMAGE_FORMAT_A8R8G8B8));
    glBindTexture(GL_TEXTURE_2D, NULL);
}

void RenderOpenGL::DeleteTexture(Texture *texture) {
    // crash here when assets not loaded as texture

    auto t = (TextureOpenGL *)texture;
    GLuint texid = t->GetOpenGlTexture();
    if (texid != -1) {
        glDeleteTextures(1, &texid);
    }
}

void RenderOpenGL::RemoveTextureFromDevice(Texture* texture) {
    logger->Info("RenderGL - RemoveTextureFromDevice");
}

bool RenderOpenGL::MoveTextureToDevice(Texture *texture) {
    auto t = (TextureOpenGL *)texture;
    auto native_format = t->GetFormat();
    int gl_format = GL_RGB;
        // native_format == IMAGE_FORMAT_A1R5G5B5 ? GL_RGBA : GL_RGB;

    uint8_t *pixels = nullptr;
    if (native_format == IMAGE_FORMAT_R5G6B5 || native_format == IMAGE_FORMAT_A1R5G5B5 || native_format == IMAGE_FORMAT_A8R8G8B8 || native_format == IMAGE_FORMAT_R8G8B8A8
         || native_format == IMAGE_FORMAT_R8G8B8) {
        pixels = (uint8_t *)t->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        // takes care of endian flip from literals here - hence BGRA
        gl_format = GL_BGRA;
    } else {
        if (engine->config->debug.VerboseLogging.Get())
            log->Warning("Image %s not loaded!", t->GetName()->c_str());
    }

    if (pixels) {
        GLuint texid;
        glGenTextures(1, &texid);
        t->SetOpenGlTexture(texid);

        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t->GetWidth(), t->GetHeight(),
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

void RenderOpenGL::_set_3d_projection_matrix() {
    float near_clip = pCamera3D->GetNearClip();
    float far_clip = pCamera3D->GetFarClip();

    // build projection matrix with glm
    projmat = glm::perspective(glm::radians(pCamera3D->fov_y_deg), pCamera3D->aspect, near_clip, far_clip);
}

void RenderOpenGL::_set_3d_modelview_matrix() {
    float camera_x = pCamera3D->vCameraPos.x;
    float camera_y = pCamera3D->vCameraPos.y;
    float camera_z = pCamera3D->vCameraPos.z;

    // build view matrix with glm
    glm::vec3 campos = glm::vec3(camera_x, camera_y, camera_z);
    glm::vec3 eyepos = glm::vec3(camera_x - cosf(2.0f * pi_double * pCamera3D->sRotationZ / 2048.0f),
        camera_y - sinf(2.0f * pi_double * pCamera3D->sRotationZ / 2048.0f),
        camera_z - tanf(2.0f * pi_double * -pCamera3D->sRotationY / 2048.0f));
    glm::vec3 upvec = glm::vec3(0.0f, 0.0f, 1.0f);

    viewmat = glm::lookAtLH(campos, eyepos, upvec);
}

void RenderOpenGL::_set_ortho_projection(bool gameviewport) {
    if (!gameviewport) {  // project over entire window
        glViewport(0, 0, output.w, output.h);
        projmat = glm::ortho(float(0), float(output.w), float(output.h), float(0), float(-1), float(1));
    } else {  // project to game viewport
        glViewport(game_viewport_x, output.h-game_viewport_w-1, game_viewport_width, game_viewport_height);
        projmat = glm::ortho(float(game_viewport_x), float(game_viewport_z), float(game_viewport_w), float(game_viewport_y), float(1), float(-1));
    }
}

void RenderOpenGL::_set_ortho_modelview() {
    // load identity matrix
    viewmat = glm::mat4x4(1);
}


// ---------------------- terrain -----------------------
const int terrain_block_scale = 512;
const int terrain_height_scale = 32;

// struct for storing vert data for gpu submit
struct GLshaderverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat u;
    GLfloat v;
    GLfloat texunit;
    GLfloat texturelayer;
    GLfloat normx;
    GLfloat normy;
    GLfloat normz;
    GLfloat attribs;
    GLfloat sector;
};

GLshaderverts terrshaderstore[127 * 127 * 6] = {};

void RenderOpenGL::DrawTerrainD3D() {
    // shader version
    // draws entire terrain in one go at the moment
    // textures must all be square and same size
    // terrain is static and verts only submitted once on VAO creation

    // face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // camera matrices
    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    // TODO(pskelton): move this to map loading
    // generate array and populate data
    if (terrainVAO == 0) {
        static RenderVertexSoft pTerrainVertices[128 * 128];
        int blockScale = 512;
        int heightScale = 32;

        // generate vertex locations
        for (unsigned int y = 0; y < 128; ++y) {
            for (unsigned int x = 0; x < 128; ++x) {
                pTerrainVertices[y * 128 + x].vWorldPosition.x = (-64.0f + x) * blockScale;
                pTerrainVertices[y * 128 + x].vWorldPosition.y = (64.0f - y) * blockScale;
                pTerrainVertices[y * 128 + x].vWorldPosition.z = heightScale * pOutdoor->pTerrain.pHeightmap[y * 128 + x];
            }
        }

        // reserve first 7 layers for water tiles in unit 0
        auto wtrtexture = this->hd_water_tile_anim[0];
        terraintexturesizes[0] = wtrtexture->GetWidth();

        for (int buff = 0; buff < 7; buff++) {
            char container_name[64];
            sprintf(container_name, "HDWTR%03u", buff);

            terraintexmap.insert(std::make_pair(container_name, terraintexmap.size()));
            numterraintexloaded[0]++;
        }

        for (int y = 0; y < 127; ++y) {
            for (int x = 0; x < 127; ++x) {
                // map is 127 x 127 squares - each square has two triangles - each tri has 3 verts

                // first find all required textures for terrain and add to map
                auto tile = pOutdoor->GetTileDescByGrid(x, y);
                int tileunit = 0;
                int tilelayer = 0;

                // check if tile->name is already in list
                auto mapiter = terraintexmap.find(tile->name);
                if (mapiter != terraintexmap.end()) {
                    // if so, extract unit and layer
                    int unitlayer = mapiter->second;
                    tilelayer = unitlayer & 0xFF;
                    tileunit = (unitlayer & 0xFF00) >> 8;
                } else if (tile->name == "wtrtyl") {
                    // water tile
                    tileunit = 0;
                    tilelayer = 0;
                } else {
                    // else need to add it
                    auto thistexture = assets->GetBitmap(tile->name);
                    int width = thistexture->GetWidth();
                    // check size to see what unit it needs
                    int i;
                    for (i = 0; i < 8; i++) {
                        if (terraintexturesizes[i] == width || terraintexturesizes[i] == 0) break;
                    }

                    if (i == 8) {
                        logger->Warning("Texture unit full - draw terrain!");
                        tileunit = 0;
                        tilelayer = 0;
                    } else {
                        if (terraintexturesizes[i] == 0) terraintexturesizes[i] = width;
                        tileunit = i;
                        tilelayer = numterraintexloaded[i];

                        // encode unit and layer together
                        int encode = (tileunit << 8) | tilelayer;

                        if (numterraintexloaded[i] < 256) {
                            // intsert into tex map
                            terraintexmap.insert(std::make_pair(tile->name, encode));
                            numterraintexloaded[i]++;
                        } else {
                            logger->Warning("Texture layer full - draw terrain!");
                            tileunit = 0;
                            tilelayer = 0;
                        }
                    }
                }

                // next calculate all vertices vertices
                uint norm_idx = pTerrainNormalIndices[(2 * x * 128) + (2 * y) + 2 /*+ 1*/];  // 2 is top tri // 3 is bottom
                uint bottnormidx = pTerrainNormalIndices[(2 * x * 128) + (2 * y) + 3];
                assert(norm_idx < pTerrainNormals.size());
                assert(bottnormidx < pTerrainNormals.size());
                Vec3f *norm = &pTerrainNormals[norm_idx];
                Vec3f *norm2 = &pTerrainNormals[bottnormidx];

                // calc each vertex
                // [0] - x,y        n1
                terrshaderstore[6 * (x + (127 * y))].x = pTerrainVertices[y * 128 + x].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y))].y = pTerrainVertices[y * 128 + x].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y))].z = pTerrainVertices[y * 128 + x].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y))].u = 0;
                terrshaderstore[6 * (x + (127 * y))].v = 0;
                terrshaderstore[6 * (x + (127 * y))].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y))].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y))].normx = norm->x;
                terrshaderstore[6 * (x + (127 * y))].normy = norm->y;
                terrshaderstore[6 * (x + (127 * y))].normz = norm->z;
                terrshaderstore[6 * (x + (127 * y))].attribs = 0;

                // [1] - x+1,y+1    n1
                terrshaderstore[6 * (x + (127 * y)) + 1].x = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y)) + 1].y = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y)) + 1].z = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y)) + 1].u = 1;
                terrshaderstore[6 * (x + (127 * y)) + 1].v = 1;
                terrshaderstore[6 * (x + (127 * y)) + 1].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y)) + 1].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y)) + 1].normx = norm->x;
                terrshaderstore[6 * (x + (127 * y)) + 1].normy = norm->y;
                terrshaderstore[6 * (x + (127 * y)) + 1].normz = norm->z;
                terrshaderstore[6 * (x + (127 * y)) + 1].attribs = 0;

                // [2] - x+1,y      n1
                terrshaderstore[6 * (x + (127 * y)) + 2].x = pTerrainVertices[y * 128 + x + 1].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y)) + 2].y = pTerrainVertices[y * 128 + x + 1].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y)) + 2].z = pTerrainVertices[y * 128 + x + 1].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y)) + 2].u = 1;
                terrshaderstore[6 * (x + (127 * y)) + 2].v = 0;
                terrshaderstore[6 * (x + (127 * y)) + 2].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y)) + 2].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y)) + 2].normx = norm->x;
                terrshaderstore[6 * (x + (127 * y)) + 2].normy = norm->y;
                terrshaderstore[6 * (x + (127 * y)) + 2].normz = norm->z;
                terrshaderstore[6 * (x + (127 * y)) + 2].attribs = 0;

                // [3] - x,y        n2
                terrshaderstore[6 * (x + (127 * y)) + 3].x = pTerrainVertices[y * 128 + x].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y)) + 3].y = pTerrainVertices[y * 128 + x].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y)) + 3].z = pTerrainVertices[y * 128 + x].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y)) + 3].u = 0;
                terrshaderstore[6 * (x + (127 * y)) + 3].v = 0;
                terrshaderstore[6 * (x + (127 * y)) + 3].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y)) + 3].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y)) + 3].normx = norm2->x;
                terrshaderstore[6 * (x + (127 * y)) + 3].normy = norm2->y;
                terrshaderstore[6 * (x + (127 * y)) + 3].normz = norm2->z;
                terrshaderstore[6 * (x + (127 * y)) + 3].attribs = 0;

                // [4] - x,y+1      n2
                terrshaderstore[6 * (x + (127 * y)) + 4].x = pTerrainVertices[(y + 1) * 128 + x].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y)) + 4].y = pTerrainVertices[(y + 1) * 128 + x].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y)) + 4].z = pTerrainVertices[(y + 1) * 128 + x].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y)) + 4].u = 0;
                terrshaderstore[6 * (x + (127 * y)) + 4].v = 1;
                terrshaderstore[6 * (x + (127 * y)) + 4].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y)) + 4].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y)) + 4].normx = norm2->x;
                terrshaderstore[6 * (x + (127 * y)) + 4].normy = norm2->y;
                terrshaderstore[6 * (x + (127 * y)) + 4].normz = norm2->z;
                terrshaderstore[6 * (x + (127 * y)) + 4].attribs = 0;

                // [5] - x+1,y+1    n2
                terrshaderstore[6 * (x + (127 * y)) + 5].x = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.x;
                terrshaderstore[6 * (x + (127 * y)) + 5].y = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.y;
                terrshaderstore[6 * (x + (127 * y)) + 5].z = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition.z;
                terrshaderstore[6 * (x + (127 * y)) + 5].u = 1;
                terrshaderstore[6 * (x + (127 * y)) + 5].v = 1;
                terrshaderstore[6 * (x + (127 * y)) + 5].texunit = tileunit;
                terrshaderstore[6 * (x + (127 * y)) + 5].texturelayer = tilelayer;
                terrshaderstore[6 * (x + (127 * y)) + 5].normx = norm2->x;
                terrshaderstore[6 * (x + (127 * y)) + 5].normy = norm2->y;
                terrshaderstore[6 * (x + (127 * y)) + 5].normz = norm2->z;
                terrshaderstore[6 * (x + (127 * y)) + 5].attribs = 0;
            }
        }

        // generate VAO
        glGenVertexArrays(1, &terrainVAO);
        glGenBuffers(1, &terrainVBO);

        glBindVertexArray(terrainVAO);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);

        // submit vert data
        glBufferData(GL_ARRAY_BUFFER, sizeof(terrshaderstore), terrshaderstore, GL_STATIC_DRAW);
        // submit data layout
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, x));
        glEnableVertexAttribArray(0);
        // tex uv attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, u));
        glEnableVertexAttribArray(1);
        // tex unit attribute
        // tex array layer attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, texunit));
        glEnableVertexAttribArray(2);
        // normals
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, normx));
        glEnableVertexAttribArray(3);
        // attribs - not used here yet
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, attribs));
        glEnableVertexAttribArray(4);

        // texture set up - load in all previously found
        for (int unit = 0; unit < 8; unit++) {
            assert(numterraintexloaded[unit] <= 256);
            // skip if textures are empty
            if (numterraintexloaded[unit] == 0) continue;

            glGenTextures(1, &terraintextures[unit]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, terraintextures[unit]);

            // create blank memory for later texture submission
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, terraintexturesizes[unit], terraintexturesizes[unit], numterraintexloaded[unit], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            // loop through texture map
            std::map<std::string, int>::iterator it = terraintexmap.begin();
            while (it != terraintexmap.end()) {
                int comb = it->second;
                int tlayer = comb & 0xFF;
                int tunit = (comb & 0xFF00) >> 8;

                if (tunit == unit) {
                    // get texture
                    auto texture = assets->GetBitmap(it->first);
                    // send texture data to gpu
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0, 0, tlayer,
                        terraintexturesizes[unit], terraintexturesizes[unit], 1,
                        GL_BGRA,
                        GL_UNSIGNED_BYTE,
                        texture->GetPixels(IMAGE_FORMAT_A8R8G8B8));
                }

                it++;
            }

            // last texture setups
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }
    }

/////////////////////////////////////////////////////
    // actual drawing

    // terrain debug
    if (engine->config->debug.Terrain.Get())
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // load texture arrays in - we only use unit 0 for water and unit 1 for tiles for time being
    for (int unit = 0; unit < 8; unit++) {
        // skip if textures are empty
        if (numterraintexloaded[unit] > 0) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D_ARRAY, terraintextures[unit]);
        }
    }

    // load terrain verts
    glBindVertexArray(terrainVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    // use the terrain shader
    glUseProgram(terrainshader.ID);

    // set projection matrix
    glUniformMatrix4fv(glGetUniformLocation(terrainshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    // set view matrix
    glUniformMatrix4fv(glGetUniformLocation(terrainshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);
    // set animated water frame
    glUniform1i(glGetUniformLocation(terrainshader.ID, "waterframe"), GLint(this->hd_water_current_frame));
    // set texture unit location
    glUniform1i(glGetUniformLocation(terrainshader.ID, "textureArray0"), GLint(0));
    glUniform1i(glGetUniformLocation(terrainshader.ID, "textureArray1"), GLint(1));

    glUniform1f(glGetUniformLocation(terrainshader.ID, "gamma"), gamma);

    // set fog uniforms
    glUniform3f(glGetUniformLocation(terrainshader.ID, "fog.color"), fogr, fogg, fogb);
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogend"), GLfloat(fogend));

    GLfloat camera[3] {};
    camera[0] = (float)(pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0));
    camera[1] = (float)(pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0));
    camera[2] = (float)(pParty->vPosition.z + pParty->sEyelevel);
    glUniform3fv(glGetUniformLocation(terrainshader.ID, "CameraPos"), 1, &camera[0]);


    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0) * 2 * pi_double) / 1440) + 1) * 0.27;
    float diffuseon = pWeather->bNight ? 0 : 1;

    glUniform3fv(glGetUniformLocation(terrainshader.ID, "sun.direction"), 1, &pOutdoor->vSunlight[0]);
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.ambient"), ambient, ambient, ambient);
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.diffuse"), diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3));
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.specular"), diffuseon * 0.35f * ambient, diffuseon * 0.28f * ambient, 0.0f);

    // red colouring
    if (pParty->armageddon_timer) {
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.ambient"), 1.0, 0, 0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.diffuse"), 1.0, 0, 0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.specular"), 0, 0, 0);
    }


    // TODO(pskelton): this should be a seperate function
    // rest of lights stacking
    GLuint num_lights = 0;

    // get party torchlight as priority - can be radius == 0
    for (int i = 0; i < 1; ++i) {
        if (pMobileLightsStack->uNumLightsActive < 1) continue;

        auto test = pMobileLightsStack->pLights[i];
        std::string slotnum = std::to_string(num_lights);

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0f;
        float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0f;
        float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0f;

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        auto test = pStationaryLightsStack->pLights[i];

        float x = test.vPosition.x;
        float y = test.vPosition.y;
        float z = test.vPosition.z;

        float r = test.uLightColorR / 255.0;
        float g = test.uLightColorG / 255.0;
        float b = test.uLightColorB / 255.0;

        float lightrad = test.uRadius;

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), r, g, b);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        auto test = pMobileLightsStack->pLights[i];

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0;
        float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0;
        float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0;

        float lightrad = pMobileLightsStack->pLights[i].uRadius;

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), r, g, b);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    if (engine->config->debug.VerboseLogging.Get())
        logger->Info("Lights this frame: %u", num_lights);

    // blank the rest of the lights
    for (int blank = num_lights; blank < 20; blank++) {
        std::string slotnum = std::to_string(blank);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 0.0);
    }

    // actually draw the whole terrain
    glDrawArrays(GL_TRIANGLES, 0, (127 * 127 * 6));
    drawcalls++;

    // unload
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NULL);

    //end terrain debug
    if (engine->config->debug.Terrain.Get())
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // stack new decals onto terrain faces ////////////////////////////////////////////////
    // TODO(pskelton): clean up
    if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;
    unsigned int NumBloodsplats = decal_builder->bloodsplat_container->uNumBloodsplats;

    // loop over blood to lay
    for (uint i = 0; i < NumBloodsplats; ++i) {
        // approx location of bloodsplat
        int splatx = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].x;
        int splaty = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].y;
        int splatz = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].z;
        int testx = WorldPosToGridCellX(splatx);
        int testy = WorldPosToGridCellY(splaty);
        // use terrain squares in block surrounding to try and stack faces

        int scope = std::ceil(decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].radius / 512);

        for (int loopy = (testy - scope); loopy <= (testy + scope); ++loopy) {
            for (int loopx = (testx - scope); loopx <= (testx + scope); ++loopx) {
                if (loopy < 0) continue;
                if (loopy > 127) continue;
                if (loopx < 0) continue;
                if (loopx > 127) continue;

                struct Polygon *pTilePolygon = &array_77EC08[pODMRenderParams->uNumPolygons];
                pTilePolygon->flags = pOutdoor->GetTileAttribByGrid(loopx, loopy);

                uint norm_idx = pTerrainNormalIndices[(2 * loopx * 128) + (2 * loopy) + 2];  // 2 is top tri // 3 is bottom
                uint bottnormidx = pTerrainNormalIndices[(2 * loopx * 128) + (2 * loopy) + 3];
                assert(norm_idx < pTerrainNormals.size());
                assert(bottnormidx < pTerrainNormals.size());
                Vec3f *norm = &pTerrainNormals[norm_idx];
                Vec3f *norm2 = &pTerrainNormals[bottnormidx];

                float _f1 = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0f - floorf(20.0f * _f1 + 0.5f);
                pTilePolygon->dimming_level = std::clamp((int)pTilePolygon->dimming_level, 0, 31);

                float Light_tile_dist = 0.0;

                int blockScale = 512;
                int heightScale = 32;

                static stru154 static_sub_0048034E_stru_154;

                // top tri
                // x, y
                VertexRenderList[0].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy))].x;
                VertexRenderList[0].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy))].y;
                VertexRenderList[0].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy))].z;
                // x + 1, y + 1
                VertexRenderList[1].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 1].x;
                VertexRenderList[1].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 1].y;
                VertexRenderList[1].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 1].z;
                // x + 1, y
                VertexRenderList[2].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 2].x;
                VertexRenderList[2].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 2].y;
                VertexRenderList[2].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 2].z;

                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, 3, 1);
                static_sub_0048034E_stru_154.ClassifyPolygon(norm, Light_tile_dist);
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level, LocationTerrain, &static_sub_0048034E_stru_154, 3, VertexRenderList, 0/**(float*)&uClipFlag*/, -1);

                //bottom tri
                float _f = norm2->x * pOutdoor->vSunlight.x + norm2->y * pOutdoor->vSunlight.y + norm2->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);
                pTilePolygon->dimming_level = std::clamp((int)pTilePolygon->dimming_level, 0, 31);

                // x, y
                VertexRenderList[0].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].x;
                VertexRenderList[0].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].y;
                VertexRenderList[0].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].z;
                // x, y + 1
                VertexRenderList[1].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].x;
                VertexRenderList[1].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].y;
                VertexRenderList[1].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].z;
                // x + 1, y + 1
                VertexRenderList[2].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].x;
                VertexRenderList[2].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].y;
                VertexRenderList[2].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].z;

                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm2, &Light_tile_dist, VertexRenderList, 3, 0);
                static_sub_0048034E_stru_154.ClassifyPolygon(norm2, Light_tile_dist);
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level, LocationTerrain, &static_sub_0048034E_stru_154, 3, VertexRenderList, 0/**(float*)&uClipFlag_2*/, -1);
            }
        }
    }

    // end of new system test
    return;

    // end shder version
}

// this is now obselete with shader terrain drawing
void RenderOpenGL::DrawTerrainPolygon(struct Polygon *poly, bool transparent, bool clampAtTextureBorders) { return; }

void RenderOpenGL::DrawOutdoorSkyD3D() {
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  horizon_height_offset = ((double)(pCamera3D->ViewPlaneDist_X * pCamera3D->vCameraPos.z)
        / ((double)pCamera3D->ViewPlaneDist_X + pCamera3D->GetFarClip())
        + (double)(pViewport->uScreenCenterY));

    float depth_to_far_clip = cos((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = sin((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();

    float bot_y_proj = ((double)(pViewport->uScreenCenterY) -
        (double)pCamera3D->ViewPlaneDist_X /
        (depth_to_far_clip + 0.0000001) *
        (height_to_far_clip - (double)pCamera3D->vCameraPos.z));

    struct Polygon pSkyPolygon;
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.ptr_38 = &SkyBillboard;


    // if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
    // pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
    // else
    // pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
    // pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ?
    // (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);

    if (!pOutdoor->sky_texture)
        pOutdoor->sky_texture = assets->GetBitmap("plansky3");

    pSkyPolygon.texture = pOutdoor->sky_texture;
    if (pSkyPolygon.texture) {
        pSkyPolygon.dimming_level = (uCurrentlyLoadedLevelType == LEVEL_Outdoor)? 31 : 0;
        pSkyPolygon.uNumVertices = 4;

        // centering(центруем)-----------------------------------------------------------------
        // plane of sky polygon rotation vector - pitch rotation around y
        float v18x = -sin((-pCamera3D->sRotationY + 16) * rot_to_rads);
        float v18y = 0;
        float v18z = -cos((pCamera3D->sRotationY + 16) * rot_to_rads);

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
        VertexRenderList[0].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;  // 8
        VertexRenderList[0].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;  // 8

        VertexRenderList[1].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;   // 8
        VertexRenderList[1].vWorldViewProjY = (double)bot_y_proj + 1;  // 247

        VertexRenderList[2].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;   // 468
        VertexRenderList[2].vWorldViewProjY = (double)bot_y_proj + 1;  // 247

        VertexRenderList[3].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;  // 468
        VertexRenderList[3].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;  // 8

        float widthperpixel = 1 / pCamera3D->ViewPlaneDist_X;

        for (uint i = 0; i < pSkyPolygon.uNumVertices; ++i) {
            // outbound screen X dist
            float x_dist = widthperpixel * (pViewport->uScreenCenterX - VertexRenderList[i].vWorldViewProjX);
            // outbound screen y dist
            float y_dist = widthperpixel * (horizon_height_offset - VertexRenderList[i].vWorldViewProjY);

            // rotate vectors to cam facing
            float skyfinalleft = (pSkyPolygon.ptr_38->CamVecLeft_X * x_dist) + (pSkyPolygon.ptr_38->CamVecLeft_Z * y_dist) + pSkyPolygon.ptr_38->CamVecLeft_Y;
            float skyfinalfront = (pSkyPolygon.ptr_38->CamVecFront_X * x_dist) + (pSkyPolygon.ptr_38->CamVecFront_Z * y_dist) + pSkyPolygon.ptr_38->CamVecFront_Y;

            // pitch rotate sky to get top
            float top_y_proj = v18x + v18y + v18z * y_dist;
            if (top_y_proj > 0.0f) top_y_proj = -0.0000001f;

            float worldviewdepth = -64.0 / top_y_proj;
            if (worldviewdepth < 0) worldviewdepth = pCamera3D->GetFarClip();

            // offset tex coords
            float texoffset_U = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalleft * worldviewdepth));
            VertexRenderList[i].u = texoffset_U / ((float)pSkyPolygon.texture->GetWidth());
            float texoffset_V = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalfront * worldviewdepth));
            VertexRenderList[i].v = texoffset_V / ((float)pSkyPolygon.texture->GetHeight());

            VertexRenderList[i].vWorldViewPosition.x = pCamera3D->GetFarClip();

            // this basically acts as texture perspective correction
            VertexRenderList[i]._rhw = (double)(worldviewdepth);
        }

        if (engine->config->graphics.Fog.Get()) {
            // fade sky
            VertexRenderList[4].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[4].vWorldViewProjY = (double)pViewport->uViewportTL_Y;
            VertexRenderList[5].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[5].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.Get();
            VertexRenderList[6].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[6].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.Get();
            VertexRenderList[7].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[7].vWorldViewProjY = (double)pViewport->uViewportTL_Y;

            // sub sky
            VertexRenderList[8].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[8].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.Get();
            VertexRenderList[9].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[9].vWorldViewProjY = (double)pViewport->uViewportBR_Y + 1;
            VertexRenderList[10].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[10].vWorldViewProjY = (double)pViewport->uViewportBR_Y + 1;
            VertexRenderList[11].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[11].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.Get();
        }

        _set_ortho_projection(1);
        _set_ortho_modelview();
        DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}



//----- (004A2DA3) --------------------------------------------------------
void RenderOpenGL::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) {
    auto texture = (TextureOpenGL *)pSkyPolygon->texture;
    auto texid = texture->GetOpenGlTexture();

    static Texture* effpar03 = assets->GetBitmap("effpar03");
    auto texturesolid = (TextureOpenGL*)effpar03;
    float texidsolid = static_cast<float>(texturesolid->GetOpenGlTexture());

    //glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    uint uTint = GetActorTintColor(pSkyPolygon->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0);
    uint uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF, uTintB = uTint & 0xFF;
    float scrspace{ pCamera3D->GetFarClip() };



    // load up poly
    for (int z = 0; z < (pSkyPolygon->uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        forcepersverts *thisvert = &forceperstore[forceperstorecnt];

        // copy first
        thisvert->x = VertexRenderList[0].vWorldViewProjX;
        thisvert->y = VertexRenderList[0].vWorldViewProjY;
        thisvert->z = 1.0f;
        thisvert->w = VertexRenderList[0]._rhw;
        thisvert->u = VertexRenderList[0].u;
        thisvert->v = VertexRenderList[0].v;
        thisvert->q = 1.0f;
        thisvert->screenspace = scrspace;
        thisvert->r = (uTintR) / 255.0f;
        thisvert->g = (uTintG) / 255.0f;
        thisvert->b = (uTintB) / 255.0f;
        thisvert->a = 1.0f;
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (uint i = 1; i < 3; ++i) {
            thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
            thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
            thisvert->z = 1.0f;
            thisvert->w = VertexRenderList[z + i]._rhw;
            thisvert->u = VertexRenderList[z + i].u;
            thisvert->v = VertexRenderList[z + i].v;
            thisvert->q = 1.0f;
            thisvert->screenspace = scrspace;
            thisvert->r = (uTintR) / 255.0f;
            thisvert->g = (uTintG) / 255.0f;
            thisvert->b = (uTintB) / 255.0f;
            thisvert->a = 1.0f;
            thisvert->texid = texid;
            thisvert++;
        }

        forceperstorecnt += 3;
        assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
    }

    if (engine->config->graphics.Fog.Get()) {
        // draw blend sky
        // load up poly
        for (int z = 4; z < 6; z++) {
            // 456, 467..
            forcepersverts *thisvert = &forceperstore[forceperstorecnt];

            // copy first
            thisvert->x = VertexRenderList[4].vWorldViewProjX;
            thisvert->y = VertexRenderList[4].vWorldViewProjY;
            thisvert->z = 1.0f;
            thisvert->w = 1.0f;
            thisvert->u = 0.5f;
            thisvert->v = 0.5f;
            thisvert->q = 1.0f;
            thisvert->screenspace = scrspace;
            thisvert->r = (uTintR) / 255.0f;
            thisvert->g = (uTintG) / 255.0f;
            thisvert->b = (uTintB) / 255.0f;
            thisvert->a = 0.0f;
            thisvert->texid = texidsolid;
            thisvert++;

            // copy other two (z+1)(z+2)
            for (uint i = 1; i < 3; ++i) {
                thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
                thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
                thisvert->z = 1.0f;
                thisvert->w = 1.0f;
                thisvert->u = 0.5f;
                thisvert->v = 0.5f;
                thisvert->q = 1.0f;
                thisvert->screenspace = scrspace;
                thisvert->r = (uTintR) / 255.0f;
                thisvert->g = (uTintG) / 255.0f;
                thisvert->b = (uTintB) / 255.0f;
                thisvert->a = ((z + i) == 7) ? 0.0f : 1.0f;
                thisvert->texid = texidsolid;
                thisvert++;
            }

            forceperstorecnt += 3;
            assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
        }

        // draw sub sky
        // load up poly
        for (int z = 8; z < 10; z++) {
            // 456, 467..
            forcepersverts *thisvert = &forceperstore[forceperstorecnt];

            // copy first
            thisvert->x = VertexRenderList[8].vWorldViewProjX;
            thisvert->y = VertexRenderList[8].vWorldViewProjY;
            thisvert->z = 1.0f;
            thisvert->w = 1.0f;
            thisvert->u = 0.5f;
            thisvert->v = 0.5f;
            thisvert->q = 1.0f;
            thisvert->screenspace = scrspace;
            thisvert->r = (uTintR) / 255.0f;
            thisvert->g = (uTintG) / 255.0f;
            thisvert->b = (uTintB) / 255.0f;
            thisvert->a = 1.0f;
            thisvert->texid = texidsolid;
            thisvert++;

            // copy other two (z+1)(z+2)
            for (uint i = 1; i < 3; ++i) {
                thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
                thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
                thisvert->z = 1.0f;
                thisvert->w = 1.0f;
                thisvert->u = 0.5f;
                thisvert->v = 0.5f;
                thisvert->q = 1.0f;
                thisvert->screenspace = scrspace;
                thisvert->r = (uTintR) / 255.0f;
                thisvert->g = (uTintG) / 255.0f;
                thisvert->b = (uTintB) / 255.0f;
                thisvert->a = 1.0f;
                thisvert->texid = texidsolid;
                thisvert++;
            }

            forceperstorecnt += 3;
            assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
        }
    }

    DrawForcePerVerts();
}

void RenderOpenGL::DrawForcePerVerts() {
    if (!forceperstorecnt) return;

    if (forceperVAO == 0) {
        glGenVertexArrays(1, &forceperVAO);
        glGenBuffers(1, &forceperVBO);

        glBindVertexArray(forceperVAO);
        glBindBuffer(GL_ARRAY_BUFFER, forceperVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(forceperstore), forceperstore, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(forcepersverts), (void *)offsetof(forcepersverts, x));
        glEnableVertexAttribArray(0);
        // tex uv
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(forcepersverts), (void *)offsetof(forcepersverts, u));
        glEnableVertexAttribArray(1);
        // screen space depth
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(forcepersverts), (void *)offsetof(forcepersverts, screenspace));
        glEnableVertexAttribArray(2);
        // colour
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(forcepersverts), (void *)offsetof(forcepersverts, r));
        glEnableVertexAttribArray(3);
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, forceperVBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(forcepersverts) * forceperstorecnt, forceperstore);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(forceperVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glUseProgram(forcepershader.ID);

    // set sampler to texure0
    glUniform1i(glGetUniformLocation(forcepershader.ID, "texture0"), GLint(0));

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(forcepershader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(forcepershader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);


    // set fog - force perspective is handled slightly differently becuase of the sky
    float fpfogr{1.0f}, fpfogg{1.0f}, fpfogb{1.0f};
    int fpfogstart{};
    int fpfogend{};
    int fpfogmiddle{};
    uint fpfogcol{ GetLevelFogColor() };

    if (engine->config->graphics.Fog.Get() && uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        if (fpfogcol) {
            fpfogstart = day_fogrange_1;
            fpfogmiddle = day_fogrange_2;
            fpfogend = pCamera3D->GetFarClip();
            fpfogr = fpfogg = fpfogb = (GetLevelFogColor() & 0xFF) / 255.0f;
        } else {
            fpfogstart = pCamera3D->GetFarClip();
            fpfogmiddle = 0.0f;
            fpfogend = fpfogstart + 1;
            fpfogr = fpfogg = fpfogb = forceperstore[0].r;
        }
    } else {
        fpfogstart = pCamera3D->GetFarClip();
        fpfogmiddle = 0.0f;
        fpfogend = fpfogstart;
    }

    // set fog uniforms
    glUniform3f(glGetUniformLocation(forcepershader.ID, "fog.color"), fpfogr, fpfogg, fpfogb);
    glUniform1f(glGetUniformLocation(forcepershader.ID, "fog.fogstart"), GLfloat(fpfogstart));
    glUniform1f(glGetUniformLocation(forcepershader.ID, "fog.fogmiddle"), GLfloat(fpfogmiddle));
    glUniform1f(glGetUniformLocation(forcepershader.ID, "fog.fogend"), GLfloat(fpfogend));

    // draw all similar textures in batches
    int offset = 0;
    while (offset < forceperstorecnt) {
        // set texture
        GLfloat thistex = forceperstore[offset].texid;
        glBindTexture(GL_TEXTURE_2D, thistex);

        int cnt = 0;
        do {
            cnt++;
            if (offset + (3 * cnt) > forceperstorecnt) {
                --cnt;
                break;
            }
        } while (forceperstore[offset + (cnt * 3)].texid == thistex);

        glDrawArrays(GL_TRIANGLES, offset, (3 * cnt));
        drawcalls++;

        offset += (3 * cnt);
    }

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    forceperstorecnt = 0;
}

void RenderOpenGL::SetFogParametersGL() {
    uint fogcol{ GetLevelFogColor() };

    if (engine->config->graphics.Fog.Get() && uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        if (fogcol) {
            fogstart = day_fogrange_1;
            fogmiddle = day_fogrange_2;
            fogend = pCamera3D->GetFarClip();
            fogr = fogg = fogb = (GetLevelFogColor() & 0xFF) / 255.0f;
        } else {
            fogend = pCamera3D->GetFarClip();
            fogmiddle = 0.0f;
            fogstart = fogend * engine->config->graphics.FogDepthRatio.Get();

            // grabs sky back fog colour
            uint uTint = GetActorTintColor(31, 0, fogend, 1, 0);
            uTint = uTint & 0xFF;
            fogr = fogg = fogb = uTint / 255.0f;
        }
    } else {
        // puts fog beyond viewclip so we never see it
        fogstart = pCamera3D->GetFarClip();
        fogmiddle = 0.0f;
        fogend = pCamera3D->GetFarClip();
    }
}

void RenderOpenGL::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {
    engine->draw_debug_outlines();
    this->DoRenderBillboards_D3D();
    spell_fx_renderer->RenderSpecialEffects();
}

struct billbverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat u;
    GLfloat v;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    GLfloat screenspace;
    GLfloat texid;
    GLfloat blend;
    GLfloat paletteindex;
};

billbverts billbstore[1000] {};
int billbstorecnt{ 0 };

//----- (004A1C1E) --------------------------------------------------------
void RenderOpenGL::DoRenderBillboards_D3D() {
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);  // in theory billboards all sorted by depth so dont cull by depth test
    glDisable(GL_CULL_FACE);  // some quads are reversed to reuse sprites opposite hand

    _set_ortho_projection(1);
    _set_ortho_modelview();

    if (billbstorecnt && engine->config->debug.VerboseLogging.Get())
        logger->Warning("Billboard shader store isnt empty!");

    // track loaded tex
    float gltexid{ 0 };
    // track blend mode
    //RenderBillboardD3D::OpacityType blendtrack{ RenderBillboardD3D::NoBlend };

    float oneon = 1.0f / (pCamera3D->GetNearClip() * 2.0f);
    float oneof = 1.0f / (pCamera3D->GetFarClip());

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i) {
        //if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend) {
        //    if (blendtrack != pBillboardRenderListD3D[i].opacity) {
        //        blendtrack = pBillboardRenderListD3D[i].opacity;
        //        SetBillboardBlendOptions(blendtrack);
        //    }
        //}

        //int palette{ pBillboardRenderListD3D[i].PaletteID};
        int paletteindex{ pBillboardRenderListD3D[i].PaletteIndex };

        if (engine->config->graphics.HWLSprites.Get())
            paletteindex = 0;

        if (pBillboardRenderListD3D[i].texture) {
            //if (!engine->config->graphics.HWLSprites.Get())
            //    palette = pBillboardRenderListD3D[i].texture->g
            auto texture = (TextureOpenGL *)pBillboardRenderListD3D[i].texture;
            gltexid = texture->GetOpenGlTexture();
        } else {
            static Texture *effpar03 = assets->GetBitmap("effpar03");
            auto texture = (TextureOpenGL *)effpar03;
            gltexid = static_cast<float>(texture->GetOpenGlTexture());
        }

        //if (gltexid != testtexid) {
        //    gltexid = testtexid;
        //    glBindTexture(GL_TEXTURE_2D, gltexid);
        //}

        auto billboard = &pBillboardRenderListD3D[i];
        auto b = &pBillboardRenderList[i];

        float oneoz = 1.0f / billboard->screen_space_z;
        float thisdepth = (oneoz - oneon) / (oneof - oneon);

        float thisblend = static_cast<float>(billboard->opacity);

        // 0 1 2 / 0 2 3

        billbstore[billbstorecnt].x = billboard->pQuads[0].pos.x;
        billbstore[billbstorecnt].y = billboard->pQuads[0].pos.y;
        billbstore[billbstorecnt].z = thisdepth;
        billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[0].texcoord.x, 0.01f, 0.99f);
        billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[0].texcoord.y, 0.01f, 0.99f);
        billbstore[billbstorecnt].r = ((billboard->pQuads[0].diffuse >> 16) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].g = ((billboard->pQuads[0].diffuse >> 8) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].b = ((billboard->pQuads[0].diffuse >> 0) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].a = 1;
        billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
        billbstore[billbstorecnt].texid = gltexid;
        billbstore[billbstorecnt].blend = thisblend;
        billbstore[billbstorecnt].paletteindex = paletteindex;
        billbstorecnt++;

        billbstore[billbstorecnt].x = billboard->pQuads[1].pos.x;
        billbstore[billbstorecnt].y = billboard->pQuads[1].pos.y;
        billbstore[billbstorecnt].z = thisdepth;
        billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[1].texcoord.x, 0.01f, 0.99f);
        billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[1].texcoord.y, 0.01f, 0.99f);
        billbstore[billbstorecnt].r = ((billboard->pQuads[1].diffuse >> 16) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].g = ((billboard->pQuads[1].diffuse >> 8) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].b = ((billboard->pQuads[1].diffuse >> 0) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].a = 1;
        billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
        billbstore[billbstorecnt].texid = gltexid;
        billbstore[billbstorecnt].blend = thisblend;
        billbstore[billbstorecnt].paletteindex = paletteindex;
        billbstorecnt++;

        billbstore[billbstorecnt].x = billboard->pQuads[2].pos.x;
        billbstore[billbstorecnt].y = billboard->pQuads[2].pos.y;
        billbstore[billbstorecnt].z = thisdepth;
        billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[2].texcoord.x, 0.01f, 0.99f);
        billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[2].texcoord.y, 0.01f, 0.99f);
        billbstore[billbstorecnt].r = ((billboard->pQuads[2].diffuse >> 16) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].g = ((billboard->pQuads[2].diffuse >> 8) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].b = ((billboard->pQuads[2].diffuse >> 0) & 0xFF) / 255.0f;
        billbstore[billbstorecnt].a = 1;
        billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
        billbstore[billbstorecnt].texid = gltexid;
        billbstore[billbstorecnt].blend = thisblend;
        billbstore[billbstorecnt].paletteindex = paletteindex;
        billbstorecnt++;

        ////////////////////////////////

        if (billboard->pQuads[3].pos.x != 0.0f && billboard->pQuads[3].pos.y != 0.0f && billboard->pQuads[3].pos.z != 0.0f) {
            billbstore[billbstorecnt].x = billboard->pQuads[0].pos.x;
            billbstore[billbstorecnt].y = billboard->pQuads[0].pos.y;
            billbstore[billbstorecnt].z = thisdepth;
            billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[0].texcoord.x, 0.01f, 0.99f);
            billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[0].texcoord.y, 0.01f, 0.99f);
            billbstore[billbstorecnt].r = ((billboard->pQuads[0].diffuse >> 16) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].g = ((billboard->pQuads[0].diffuse >> 8) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].b = ((billboard->pQuads[0].diffuse >> 0) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].a = 1;
            billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
            billbstore[billbstorecnt].texid = gltexid;
            billbstore[billbstorecnt].blend = thisblend;
            billbstore[billbstorecnt].paletteindex = paletteindex;
            billbstorecnt++;

            billbstore[billbstorecnt].x = billboard->pQuads[2].pos.x;
            billbstore[billbstorecnt].y = billboard->pQuads[2].pos.y;
            billbstore[billbstorecnt].z = thisdepth;
            billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[2].texcoord.x, 0.01f, 0.99f);
            billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[2].texcoord.y, 0.01f, 0.99f);
            billbstore[billbstorecnt].r = ((billboard->pQuads[2].diffuse >> 16) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].g = ((billboard->pQuads[2].diffuse >> 8) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].b = ((billboard->pQuads[2].diffuse >> 0) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].a = 1;
            billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
            billbstore[billbstorecnt].texid = gltexid;
            billbstore[billbstorecnt].blend = thisblend;
            billbstore[billbstorecnt].paletteindex = paletteindex;
            billbstorecnt++;

            billbstore[billbstorecnt].x = billboard->pQuads[3].pos.x;
            billbstore[billbstorecnt].y = billboard->pQuads[3].pos.y;
            billbstore[billbstorecnt].z = thisdepth;
            billbstore[billbstorecnt].u = std::clamp(billboard->pQuads[3].texcoord.x, 0.01f, 0.99f);
            billbstore[billbstorecnt].v = std::clamp(billboard->pQuads[3].texcoord.y, 0.01f, 0.99f);
            billbstore[billbstorecnt].r = ((billboard->pQuads[3].diffuse >> 16) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].g = ((billboard->pQuads[3].diffuse >> 8) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].b = ((billboard->pQuads[3].diffuse >> 0) & 0xFF) / 255.0f;
            billbstore[billbstorecnt].a = 1;
            billbstore[billbstorecnt].screenspace = billboard->screen_space_z;
            billbstore[billbstorecnt].texid = gltexid;
            billbstore[billbstorecnt].blend = thisblend;
            billbstore[billbstorecnt].paletteindex = paletteindex;
            billbstorecnt++;
        }

        if (billbstorecnt > 990) {
            DrawBillboards();
        }
    }

    // uNumBillboardsToDraw = 0;

    DrawBillboards();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

// name better
void RenderOpenGL::DrawBillboards() {
    if (!billbstorecnt) return;

    if (billbVAO == 0) {
        glGenVertexArrays(1, &billbVAO);
        glGenBuffers(1, &billbVBO);

        glBindVertexArray(billbVAO);
        glBindBuffer(GL_ARRAY_BUFFER, billbVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(billbstore), billbstore, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, x));
        glEnableVertexAttribArray(0);
        // tex uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, u));
        glEnableVertexAttribArray(1);
        // colour
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, r));
        glEnableVertexAttribArray(2);
        // screenspace
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, screenspace));
        glEnableVertexAttribArray(3);
        // texid
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, texid));
        glEnableVertexAttribArray(4);
        // palette index
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, paletteindex));
        glEnableVertexAttribArray(5);
    }

    if (palbuf == 0) {
        // generate palette buffer texture
        glGenBuffers(1, &palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, pPaletteManager->GetGLPaletteSize(), pPaletteManager->GetGLPalettePtr(), GL_STATIC_DRAW);

        glGenTextures(1, &paltex);
        glBindTexture(GL_TEXTURE_BUFFER, paltex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        pPaletteManager->GLPaletteReset();
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, billbVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(billbverts) * billbstorecnt, billbstore);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (pPaletteManager->GetGLPaletteNeedsUpdate()) {
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, pPaletteManager->GetGLPaletteSize(), pPaletteManager->GetGLPalettePtr(), GL_STATIC_DRAW);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        pPaletteManager->GLPaletteReset();
    }

    glBindVertexArray(billbVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    glUseProgram(billbshader.ID);

    // set sampler to palette
    glUniform1i(glGetUniformLocation(billbshader.ID, "palbuf"), GLint(1));
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_BUFFER, paltex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
    glActiveTexture(GL_TEXTURE0);

    GLboolean repaint = !engine->config->graphics.HWLSprites.Get();
    glUniform1i(glGetUniformLocation(billbshader.ID, "repaint"), repaint);


    // set sampler to texure0
    glUniform1i(glGetUniformLocation(billbshader.ID, "texture0"), GLint(0));

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(billbshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(billbshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    glUniform1f(glGetUniformLocation(billbshader.ID, "gamma"), gamma);

    // set fog uniforms
    glUniform3f(glGetUniformLocation(billbshader.ID, "fog.color"), fogr, fogg, fogb);
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogend"), GLfloat(fogend));

    int offset = 0;
    while (offset < billbstorecnt) {
        // set texture
        GLfloat thistex = billbstore[offset].texid;
        glBindTexture(GL_TEXTURE_2D, billbstore[offset].texid);
        if (repaint && billbstore[offset].paletteindex) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        GLfloat thisblend = billbstore[offset].blend;
        if (thisblend == 0.0) {
            // disable alpha blending and enable fog for opaque items
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogstart"), GLfloat(fogstart));
        } else {
            // enable blending and disable fog for transparent items
            glBlendFunc(GL_ONE, GL_ONE);
            glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogstart"), GLfloat(fogend));
        }


        int cnt = 0;
        do {
            cnt++;
            if (offset + (3 * cnt) > billbstorecnt) {
                --cnt;
                break;
            }
        } while (billbstore[offset + (cnt * 3)].texid == thistex && billbstore[offset + (cnt * 3)].blend == thisblend);

        glDrawArrays(GL_TRIANGLES, offset, (3 * cnt));
        drawcalls++;

        offset += (3 * cnt);
    }

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    billbstorecnt = 0;
}

//----- (004A1DA8) --------------------------------------------------------
void RenderOpenGL::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1) {
    return;
}

void RenderOpenGL::SetUIClipRect(unsigned int x, unsigned int y, unsigned int z,
                                 unsigned int w) {
    this->clip_x = x;
    this->clip_y = y;
    this->clip_z = z;
    this->clip_w = w;
    glScissor(x, output.h -w, z-x, w-y);  // invert glscissor co-ords 0,0 is BL
}

void RenderOpenGL::ResetUIClipRect() {
    this->SetUIClipRect(0, 0, output.w, output.h);
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

void RenderOpenGL::DrawTextureNew(float u, float v, Image *tex, uint32_t colourmask) {
    TextureOpenGL *texture = dynamic_cast<TextureOpenGL *>(tex);
    if (!texture) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Info("Null texture passed to DrawTextureNew");
        return;
    }

    float r = ((colourmask >> 16) & 0xFF) / 255.0f;
    float g = ((colourmask >> 8) & 0xFF) / 255.0f;
    float b = ((colourmask >> 0) & 0xFF) / 255.0f;

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = tex->GetWidth();
    int height = tex->GetHeight();

    int x = u * output.w;
    int y = v * output.h;
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= output.w || x >= this->clip_z || y >= output.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = texture->GetOpenGlTexture();

    float drawx = static_cast<float>(std::max(x, this->clip_x));
    float drawy = static_cast<float>(std::max(y, this->clip_y));
    float draww = static_cast<float>(std::min(w, this->clip_w));
    float drawz = static_cast<float>(std::min(z, this->clip_z));

    float texx = (drawx - x) / float(width);
    float texy = (drawy - y) / float(height);
    float texz = (drawz - x) / float(width);
    float texw = (draww - y) / float(height);

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

void RenderOpenGL::DrawTextureCustomHeight(float u, float v, class Image *img, int custom_height) {
    TextureOpenGL* texture = dynamic_cast<TextureOpenGL*>(img);
    if (!texture) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Info("Null texture passed to DrawTextureCustomHeight");
        return;
    }

    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = img->GetWidth();
    int height = img->GetHeight();

    int x = u * output.w;
    int y = v * output.h + 0.5;
    int z = x + width;
    int w = y + custom_height;

    // check bounds
    if (x >= output.w || x >= this->clip_z || y >= output.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = texture->GetOpenGlTexture();

    float drawx = static_cast<float>(std::max(x, this->clip_x));
    float drawy = static_cast<float>(std::max(y, this->clip_y));
    float draww = static_cast<float>(std::min(w, this->clip_w));
    float drawz = static_cast<float>(std::min(z, this->clip_z));

    float texx = (drawx - x) / float(width);
    float texy = (drawy - y) / float(height);
    float texz = float(drawz) / z;
    float texw = float(draww) / w;

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


twodverts textshaderstore[10000] = {};
int textvertscnt = 0;

void RenderOpenGL::BeginTextNew(Texture *main, Texture *shadow) {
    // draw any images in buffer
    if (twodvertscnt) {
        DrawTwodVerts();
    }

    auto texturemain = (TextureOpenGL *)main;
    GLuint texmainidcheck = texturemain->GetOpenGlTexture();

    // if we are changing font draw whats in the text buffer
    if (texmainidcheck != texmain) {
        EndTextNew();
    }

    texmain = texturemain->GetOpenGlTexture();

    auto textureshadow = (TextureOpenGL *)shadow;
    texshadow = textureshadow->GetOpenGlTexture();

    // set up buffers
    // set up counts
    // set up textures

    return;
}

void RenderOpenGL::EndTextNew() {
    if (!textvertscnt) return;

    if (twodvertscnt) {
        DrawTwodVerts();
    }

    if (textVAO == 0) {
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);

        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(textshaderstore), NULL, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void *)offsetof(twodverts, x));
        glEnableVertexAttribArray(0);
        // tex uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void *)offsetof(twodverts, u));
        glEnableVertexAttribArray(1);
        // colour
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void *)offsetof(twodverts, r));
        glEnableVertexAttribArray(2);
        // texid
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void *)offsetof(twodverts, texid));
        glEnableVertexAttribArray(3);
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    // orphan
    glBufferData(GL_ARRAY_BUFFER, sizeof(textshaderstore), NULL, GL_DYNAMIC_DRAW);
    // update buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(twodverts) * textvertscnt, textshaderstore);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(textVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glUseProgram(textshader.ID);

    // glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set sampler to texure0
    glUniform1i(glGetUniformLocation(textshader.ID, "texture0"), GLint(0));
    glUniform1i(glGetUniformLocation(textshader.ID, "texture1"), GLint(1));

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(textshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(textshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    // set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texmain);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texshadow);

    glDrawArrays(GL_TRIANGLES, 0, textvertscnt);
    drawcalls++;

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    textvertscnt = 0;
    // texmain = 0;
    // texshadow = 0;
    return;
}

void RenderOpenGL::DrawTextNew(int x, int y, int width, int h, float u1, float v1, float u2, float v2, int isshadow, uint16_t colour) {
    float b = ((colour & 31) * 8) / 255.0f;
    float g = (((colour >> 5) & 63) * 4) / 255.0f;
    float r = (((colour >> 11) & 31) * 8) / 255.0f;
    // not 100% sure why this is required but it is
    if (r == 0.0f) r = 0.00392f;

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int z = x + width;
    int w = y + h;
    // check bounds
    if (x >= output.w || x >= clipz || y >= output.h || y >= clipw) return;
    // check for overlap
    if (!(clipx < z && clipz > x && clipy < w && clipw > y)) return;

    float drawx = static_cast<float>(x);
    float drawy = static_cast<float>(y);
    float draww = static_cast<float>(w);
    float drawz = static_cast<float>(z);

    float depth = 0;
    float texx = u1;
    float texy = v1;
    float texz = u2;
    float texw = v2;

    // 0 1 2 / 0 2 3
    textshaderstore[textvertscnt].x = drawx;
    textshaderstore[textvertscnt].y = drawy;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texx;
    textshaderstore[textvertscnt].v = texy;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1.0f;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = drawy;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texy;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1.0f;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    ////////////////////////////////
    textshaderstore[textvertscnt].x = drawx;
    textshaderstore[textvertscnt].y = drawy;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texx;
    textshaderstore[textvertscnt].v = texy;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawx;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texx;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].r = r;
    textshaderstore[textvertscnt].g = g;
    textshaderstore[textvertscnt].b = b;
    textshaderstore[textvertscnt].a = 1;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    if (textvertscnt > 9990) EndTextNew();
}

void RenderOpenGL::DrawText(int uOutX, int uOutY, uint8_t* pFontPixels,
                            unsigned int uCharWidth, unsigned int uCharHeight,
                            uint8_t* pFontPalette, uint16_t uFaceColor,
                            uint16_t uShadowColor) {
    return;
}

void RenderOpenGL::DrawTextAlpha(int x, int y, unsigned char *font_pixels,
                                 int uCharWidth, unsigned int uFontHeight,
                                 uint8_t *pPalette,
                                 bool present_time_transparency) {
    return;
}

void RenderOpenGL::Present() {
    // flush any undrawn items
    DrawTwodVerts();
    EndLines2D();
    EndTextNew();

    openGlContext->SwapBuffers();

    if (engine->config->graphics.FPSLimit.Get() > 0) {
        // crude frame rate limiting
        const float MIN_FRAME_TIME = 1000000000 / (float)engine->config->graphics.FPSLimit.Get();

        static std::chrono::time_point<std::chrono::high_resolution_clock> lastframe{ std::chrono::high_resolution_clock::now() };
        uint64_t framedt{};
        std::chrono::time_point<std::chrono::high_resolution_clock> now{};

        // run in circles
        do {
            now = std::chrono::high_resolution_clock::now();
            framedt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastframe).count();
        } while (framedt < MIN_FRAME_TIME);
        lastframe = now;
    }
}

GLshaderverts *outbuildshaderstore[16] = { nullptr };
int numoutbuildverts[16] = { 0 };

void RenderOpenGL::DrawBuildingsD3D() {
    // shader
    // verts are streamed to gpu as required
    // textures can be different sizes

    // TODO(pskelton): might have to pass a texture width through for the waterr flow textures to size right
    // and get the correct water speed

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    if (outbuildVAO[0] == 0) {
        // count all triangles
        // make out build shader store
        int verttotals = 0;

        for (int i = 0; i < 16; i++) {
            numoutbuildverts[i] = 0;

            //for (BSPModel& model : pOutdoor->pBModels) {
            //    //int reachable;
            //    //if (IsBModelVisible(&model, &reachable)) {
            //    //model.field_40 |= 1;
            //    if (!model.pFaces.empty()) {
            //        for (ODMFace& face : model.pFaces) {
            //            if (!face.Invisible()) {
            //                numoutbuildverts += 3 * (face.uNumVertices - 2);
            //            }
            //        }
            //    }
            //}

            free(outbuildshaderstore[i]);
            outbuildshaderstore[i] = nullptr;
            outbuildshaderstore[i] = (GLshaderverts *)malloc(sizeof(GLshaderverts) * 20000);
        }

        // reserve first 7 layers for water tiles in unit 0
        auto wtrtexture = this->hd_water_tile_anim[0];
        //terraintexmap.insert(std::make_pair("wtrtyl", terraintexmap.size()));
        //numterraintexloaded[0]++;
        outbuildtexturewidths[0] = wtrtexture->GetWidth();
        outbuildtextureheights[0] = wtrtexture->GetHeight();

        for (int buff = 0; buff < 7; buff++) {
            char container_name[64];
            sprintf(container_name, "HDWTR%03u", buff);

            outbuildtexmap.insert(std::make_pair(container_name, outbuildtexmap.size()));
            numoutbuildtexloaded[0]++;
        }





        for (BSPModel &model : pOutdoor->pBModels) {
            //int reachable;
            //if (IsBModelVisible(&model, &reachable)) {
            model.field_40 |= 1;
            if (!model.pFaces.empty()) {
                for (ODMFace &face : model.pFaces) {
                    if (!face.Invisible()) {
                        //v53 = 0;
                        //auto poly = &array_77EC08[pODMRenderParams->uNumPolygons];

                        //poly->flags = 0;
                        //poly->field_32 = 0;
                        TextureOpenGL* tex = (TextureOpenGL*)face.GetTexture();

                        std::string *texname = tex->GetName();
                        // gather up all texture and shaderverts data

                        //auto tile = pOutdoor->GetTileDescByGrid(x, y);

                        int texunit = 0;
                        int texlayer = 0;
                        int attribflags = 0;

                        if (face.uAttributes & FACE_IsFluid) attribflags |= 2;
                        if (face.uAttributes & FACE_INDOOR_SKY) attribflags |= 0x400;

                        if (face.uAttributes & FACE_FlowDown)
                            attribflags |= 0x400;
                        else if (face.uAttributes & FACE_FlowUp)
                            attribflags |= 0x800;

                        if (face.uAttributes & FACE_FlowRight)
                            attribflags |= 0x2000;
                        else if (face.uAttributes & FACE_FlowLeft)
                            attribflags |= 0x1000;

                        // check if tile->name is already in list
                        auto mapiter = outbuildtexmap.find(*texname);
                        if (mapiter != outbuildtexmap.end()) {
                            // if so, extract unit and layer
                            int unitlayer = mapiter->second;
                            texlayer = unitlayer & 0xFF;
                            texunit = (unitlayer & 0xFF00) >> 8;
                        } else if (*texname == "wtrtyl") {
                            // water tile
                            texunit = 0;
                            texlayer = 0;
                        } else {
                            // else need to add it
                            auto thistexture = assets->GetBitmap(*texname);
                            int width = thistexture->GetWidth();
                            int height = thistexture->GetHeight();
                            // check size to see what unit it needs
                            int i;
                            for (i = 0; i < 16; i++) {
                                if ((outbuildtexturewidths[i] == width && outbuildtextureheights[i] == height) || outbuildtexturewidths[i] == 0) break;
                            }

                            if (i == 16) {
                                logger->Warning("Texture unit full - draw building!");
                                texunit = 0;
                                texlayer = 0;
                            } else {
                                if (outbuildtexturewidths[i] == 0) {
                                    outbuildtexturewidths[i] = width;
                                    outbuildtextureheights[i] = height;
                                }

                                texunit = i;
                                texlayer = numoutbuildtexloaded[i];

                                // encode unit and layer together
                                int encode = (texunit << 8) | texlayer;

                                if (numoutbuildtexloaded[i] < 256) {
                                    // intsert into tex map
                                    outbuildtexmap.insert(std::make_pair(*texname, encode));
                                    numoutbuildtexloaded[i]++;
                                } else {
                                    logger->Warning("Texture layer full - draw building!");
                                    texunit = 0;
                                    texlayer = 0;
                                }
                            }
                        }

                        face.texunit = texunit;
                        face.texlayer = texlayer;
                    }
                }
            }
            //}
        }

        for (int l = 0; l < 16; l++) {
            glGenVertexArrays(1, &outbuildVAO[l]);
            glGenBuffers(1, &outbuildVBO[l]);

            glBindVertexArray(outbuildVAO[l]);
            glBindBuffer(GL_ARRAY_BUFFER, outbuildVBO[l]);

            glBufferData(GL_ARRAY_BUFFER, sizeof(GLshaderverts) * 20000, outbuildshaderstore[l], GL_DYNAMIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, x));
            glEnableVertexAttribArray(0);
            // tex uv attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, u));
            glEnableVertexAttribArray(1);
            // tex unit attribute
            // tex array layer attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, texunit));
            glEnableVertexAttribArray(2);
            // normals
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, normx));
            glEnableVertexAttribArray(3);
            // attribs - not used here yet
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, attribs));
            glEnableVertexAttribArray(4);
        }

        // texture set up

        // loop over all units
        for (int unit = 0; unit < 16; unit++) {
            assert(numoutbuildtexloaded[unit] <= 256);
            // skip if textures are empty
            if (numoutbuildtexloaded[unit] == 0) continue;

            glGenTextures(1, &outbuildtextures[unit]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, outbuildtextures[unit]);

            // create blank memory for later texture submission
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, outbuildtexturewidths[unit], outbuildtextureheights[unit], numoutbuildtexloaded[unit], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            std::map<std::string, int>::iterator it = outbuildtexmap.begin();
            while (it != outbuildtexmap.end()) {
                // skip if wtrtyl
                //if ((it->first).substr(0, 6) == "wtrtyl") {
                //    std::cout << "skipped  " << it->first << std::endl;
                 //   it++;
                 //   continue;
                //}

                int comb = it->second;
                int tlayer = comb & 0xFF;
                int tunit = (comb & 0xFF00) >> 8;

                if (tunit == unit) {
                    // get texture
                    auto texture = assets->GetBitmap(it->first);

                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0, 0, tlayer,
                        outbuildtexturewidths[unit], outbuildtextureheights[unit], 1,
                        GL_BGRA,
                        GL_UNSIGNED_BYTE,
                        texture->GetPixels(IMAGE_FORMAT_A8R8G8B8));
                }

                it++;
            }

            //iterate through terrain tex map
            //ignore wtrtyl
            //laod in

            //auto tile = pOutdoor->GetTileDescByGrid(0, 0);
            //bool border = tile->IsWaterBorderTile();


            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }
    }

        // else update verts - blank store
        for (int i = 0; i < 16; i++) {
            numoutbuildverts[i] = 0;
        }

        for (BSPModel &model : pOutdoor->pBModels) {
            bool reachable;
            if (IsBModelVisible(&model, 256, &reachable)) {
                //if (model.index == 35) continue;
                model.field_40 |= 1;
                if (!model.pFaces.empty()) {
                    for (ODMFace &face : model.pFaces) {
                        if (!face.Invisible()) {
                            array_73D150[0].vWorldPosition = model.pVertices[face.pVertexIDs[0]].ToFloat();

                            if (pCamera3D->is_face_faced_to_cameraODM(&face, &array_73D150[0])) {
                                int texunit = face.texunit;
                                int texlayer = face.texlayer;

                                if (texlayer == -1) { // texture has been reset - see if its in the map
                                    TextureOpenGL *tex = (TextureOpenGL *)face.GetTexture();
                                    std::string *texname = tex->GetName();
                                    auto mapiter = bsptexmap.find(*texname);
                                    if (mapiter != bsptexmap.end()) {
                                        // if so, extract unit and layer
                                        int unitlayer = mapiter->second;
                                        face.texlayer = texlayer = unitlayer & 0xFF;
                                        face.texunit = texunit = (unitlayer & 0xFF00) >> 8;
                                    } else {
                                        logger->Warning("Texture not found in map!");
                                        // TODO(pskelton): set to water for now - fountains in walls of mist
                                        texunit = face.texlayer = 0;
                                        texlayer = face.texunit = 0;
                                    }
                                }

                                int attribflags = 0;

                                if (face.uAttributes & FACE_IsFluid) attribflags |= 2;
                                if (face.uAttributes & FACE_INDOOR_SKY) attribflags |= 0x400;

                                if (face.uAttributes & FACE_FlowDown)
                                    attribflags |= 0x400;
                                else if (face.uAttributes & FACE_FlowUp)
                                    attribflags |= 0x800;

                                if (face.uAttributes & FACE_FlowRight)
                                    attribflags |= 0x2000;
                                else if (face.uAttributes & FACE_FlowLeft)
                                    attribflags |= 0x1000;

                                if (face.uAttributes & (FACE_OUTLINED | FACE_IsSecret))
                                    attribflags |= 0x00010000;

                                // load up verts here
                                for (int z = 0; z < (face.uNumVertices - 2); z++) {
                                    // 123, 134, 145, 156..
                                    GLshaderverts* thisvert = &outbuildshaderstore[texunit][numoutbuildverts[texunit]];

                                    // copy first
                                    thisvert->x = model.pVertices[face.pVertexIDs[0]].x;
                                    thisvert->y = model.pVertices[face.pVertexIDs[0]].y;
                                    thisvert->z = model.pVertices[face.pVertexIDs[0]].z;
                                    thisvert->u = face.pTextureUIDs[0] + face.sTextureDeltaU;
                                    thisvert->v = face.pTextureVIDs[0] + face.sTextureDeltaV;
                                    thisvert->texunit = texunit;
                                    thisvert->texturelayer = texlayer;
                                    thisvert->normx = face.pFacePlane.vNormal.x;
                                    thisvert->normy = face.pFacePlane.vNormal.y;
                                    thisvert->normz = face.pFacePlane.vNormal.z;
                                    thisvert->attribs = attribflags;
                                    thisvert++;

                                    // copy other two (z+1)(z+2)
                                    for (uint i = 1; i < 3; ++i) {
                                        thisvert->x = model.pVertices[face.pVertexIDs[z + i]].x;
                                        thisvert->y = model.pVertices[face.pVertexIDs[z + i]].y;
                                        thisvert->z = model.pVertices[face.pVertexIDs[z + i]].z;
                                        thisvert->u = face.pTextureUIDs[z + i] + face.sTextureDeltaU;
                                        thisvert->v = face.pTextureVIDs[z + i] + face.sTextureDeltaV;
                                        thisvert->texunit = texunit;
                                        thisvert->texturelayer = texlayer;
                                        thisvert->normx = face.pFacePlane.vNormal.x;
                                        thisvert->normy = face.pFacePlane.vNormal.y;
                                        thisvert->normz = face.pFacePlane.vNormal.z;
                                        thisvert->attribs = attribflags;
                                        thisvert++;
                                    }

                                    numoutbuildverts[texunit] += 3;
                                    assert(numoutbuildverts[texunit] <= 9999);
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int l = 0; l < 16; l++) {
            if (numoutbuildverts[l]) {
                glBindBuffer(GL_ARRAY_BUFFER, outbuildVBO[l]);
                // orphan buffer
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLshaderverts) * 20000, NULL, GL_DYNAMIC_DRAW);
                // update buffer
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLshaderverts) * numoutbuildverts[l], outbuildshaderstore[l]);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

    // terrain debug
    if (engine->config->debug.Terrain.Get())
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    ////


    glUseProgram(outbuildshader.ID);
    // set projection
    glUniformMatrix4fv(glGetUniformLocation(outbuildshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    // set view
    glUniformMatrix4fv(glGetUniformLocation(outbuildshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    glUniform1i(glGetUniformLocation(outbuildshader.ID, "waterframe"), GLint(this->hd_water_current_frame));
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "flowtimer"), GLint(platform->TickCount() >> 4));

    glUniform1f(glGetUniformLocation(outbuildshader.ID, "gamma"), gamma);

    // set texture unit location
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "textureArray0"), GLint(0));

    // set fog uniforms
    glUniform3f(glGetUniformLocation(outbuildshader.ID, "fog.color"), fogr, fogg, fogb);
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogend"), GLfloat(fogend));

    GLfloat camera[3] {};
    camera[0] = (float)(pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0f));
    camera[1] = (float)(pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0f));
    camera[2] = (float)(pParty->vPosition.z + pParty->sEyelevel);
    glUniform3fv(glGetUniformLocation(outbuildshader.ID, "CameraPos"), 1, &camera[0]);


    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0f;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0f) * 2 * pi_double) / 1440) + 1) * 0.27f;
    float diffuseon = pWeather->bNight ? 0.0f : 1.0f;

    glUniform3fv(glGetUniformLocation(outbuildshader.ID, "sun.direction"), 1, &pOutdoor->vSunlight[0]);
    glUniform3f(glGetUniformLocation(outbuildshader.ID, "sun.ambient"), ambient, ambient, ambient);
    glUniform3f(glGetUniformLocation(outbuildshader.ID, "sun.diffuse"), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f));
    glUniform3f(glGetUniformLocation(outbuildshader.ID, "sun.specular"), diffuseon * 0.35f * ambient, diffuseon * 0.28f * ambient, 0.0f);

    if (pParty->armageddon_timer) {
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.ambient"), 1.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.diffuse"), 1.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.specular"), 0.0f, 0.0f, 0.0f);
    }


    // TODO(pskelton): this should be a seperate function
    // rest of lights stacking
    GLuint num_lights = 0;

    // get party torchlight as priority - can be radius == 0
    for (int i = 0; i < 1; ++i) {
        if (pMobileLightsStack->uNumLightsActive < 1) continue;

        auto test = pMobileLightsStack->pLights[i];
        std::string slotnum = std::to_string(num_lights);

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0f;
        float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0f;
        float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0f;

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        auto test = pStationaryLightsStack->pLights[i];

        float x = test.vPosition.x;
        float y = test.vPosition.y;
        float z = test.vPosition.z;

        float r = test.uLightColorR / 255.0;
        float g = test.uLightColorG / 255.0;
        float b = test.uLightColorB / 255.0;

        float lightrad = test.uRadius;

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), r, g, b);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        auto test = pMobileLightsStack->pLights[i];

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0;
        float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0;
        float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0;

        float lightrad = pMobileLightsStack->pLights[i].uRadius;

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), r, g, b);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    if (engine->config->debug.VerboseLogging.Get())
        logger->Info("Lights this frame: %u", num_lights);

    // blank the rest of the lights
    for (int blank = num_lights; blank < 20; blank++) {
        std::string slotnum = std::to_string(blank);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 0.0);
    }


    // toggle for water faces or not
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "watertiles"), GLint(1));

    glActiveTexture(GL_TEXTURE0);

    for (int unit = 0; unit < 16; unit++) {
        // skip if textures are empty
        //if (numoutbuildtexloaded[unit] > 0) {
            if (unit == 1) {
                glUniform1i(glGetUniformLocation(outbuildshader.ID, "watertiles"), GLint(0));
            }

            // draw each set of triangles
            glBindTexture(GL_TEXTURE_2D_ARRAY, outbuildtextures[unit]);
            glBindVertexArray(outbuildVAO[unit]);
            glDrawArrays(GL_TRIANGLES, 0, (numoutbuildverts[unit]));
            drawcalls++;
        //}
    }

    // unload
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NULL);

    //end terrain debug
    if (engine->config->debug.Terrain.Get())
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    // TODO(pskelton): clean up
    // need to stack decals
    if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;

    for (BSPModel &model : pOutdoor->pBModels) {
        bool reachable;
        if (!IsBModelVisible(&model, 256, &reachable)) {
            continue;
        }
        model.field_40 |= 1;
        if (model.pFaces.empty()) {
            continue;
        }

        for (ODMFace &face : model.pFaces) {
            if (face.Invisible()) {
                continue;
            }


            //////////////////////////////////////////////

            struct Polygon *poly = &array_77EC08[pODMRenderParams->uNumPolygons];
            poly->flags = 0;
            poly->field_32 = 0;

            // if (v53 == face.uNumVertices) poly->field_32 |= 1;
            poly->pODMFace = &face;
            poly->uNumVertices = face.uNumVertices;
            poly->field_59 = 5;


            float _f1 = face.pFacePlane.vNormal.x * pOutdoor->vSunlight.x + face.pFacePlane.vNormal.y * pOutdoor->vSunlight.y + face.pFacePlane.vNormal.z * pOutdoor->vSunlight.z;
            poly->dimming_level = 20.0 - floorf(20.0 * _f1 + 0.5f);
            poly->dimming_level = std::clamp((int)poly->dimming_level, 0, 31);

            for (uint vertex_id = 1; vertex_id <= face.uNumVertices; vertex_id++) {
                array_73D150[vertex_id - 1].vWorldPosition.x =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].x;
                array_73D150[vertex_id - 1].vWorldPosition.y =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].y;
                array_73D150[vertex_id - 1].vWorldPosition.z =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].z;
            }


            for (int vertex_id = 0; vertex_id < face.uNumVertices; ++vertex_id) {
                memcpy(&VertexRenderList[vertex_id], &array_73D150[vertex_id], sizeof(VertexRenderList[vertex_id]));
                VertexRenderList[vertex_id]._rhw = 1.0 / (array_73D150[vertex_id].vWorldViewPosition.x + 0.0000001);
            }


            float Light_tile_dist = 0.0;



            //static stru154 static_sub_0048034E_stru_154;


            if (pCamera3D->is_face_faced_to_cameraODM(&face, &array_73D150[0])) {
                face.bVisible = 1;
                poly->uBModelFaceID = face.index;
                poly->uBModelID = model.index;
                poly->pid =
                    PID(OBJECT_Face, (face.index | (model.index << 6)));

                static stru154 static_RenderBuildingsD3D_stru_73C834;




                decal_builder->ApplyBloodSplat_OutdoorFace(&face);

                //lightmap_builder->StationaryLightsCount = 0;
                int v31 = 0;
                if (decal_builder->uNumSplatsThisFace > 0) {
                    //v31 = nearclip ? 3 : farclip != 0 ? 5 : 0;

                    static_RenderBuildingsD3D_stru_73C834.GetFacePlaneAndClassify(&face, model.pVertices);
                    if (decal_builder->uNumSplatsThisFace > 0) {
                        decal_builder->BuildAndApplyDecals(
                            31 - poly->dimming_level, LocationBuildings,
                            &static_RenderBuildingsD3D_stru_73C834,
                            face.uNumVertices, VertexRenderList, (char)v31,
                            -1);
                    }
                }
            }
        }
    }

    return;



    ///////////////// shader end
}

GLshaderverts* BSPshaderstore[16] = { nullptr };
int numBSPverts[16] = { 0 };

void RenderOpenGL::DrawIndoorFaces() {
    // void RenderOpenGL::DrawIndoorBSP() {

    // TODO(pskelton): might have to pass a texture width through for the waterr flow textures to size right
    // and get the correct water speed


        glEnable(GL_CULL_FACE);

        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        _set_ortho_projection(1);
        _set_ortho_modelview();

        _set_3d_projection_matrix();
        _set_3d_modelview_matrix();

        if (bspVAO[0] == 0) {
            // lights setup
            int cntnosect = 0;

            for (int lightscnt = 0; lightscnt < pStationaryLightsStack->uNumLightsActive; ++lightscnt) {
                auto test = pStationaryLightsStack->pLights[lightscnt];


                // kludge for getting lights in  visible sectors
                pStationaryLightsStack->pLights[lightscnt].uSectorID = pIndoor->GetSector(test.vPosition.ToInt());

                if (pStationaryLightsStack->pLights[lightscnt].uSectorID == 0) cntnosect++;
            }

            logger->Warning("%i lights - sector not found", cntnosect);


            // count triangles and create store
            //int verttotals = 0;
            //numBSPverts = 0;

            //for (int count = 0; count < pIndoor->uNumFaces; count++) {
            //    auto face = &pIndoor->pFaces[count];
            //    if (face->Portal()) continue;
            //    if (!face->GetTexture()) continue;

            //    //if (face->uAttributes & FACE_IS_DOOR) continue;

            //    if (face->uNumVertices) {
            //        numBSPverts += 3 * (face->uNumVertices - 2);
            //    }
            //}

            //free(BSPshaderstore);
            //BSPshaderstore = NULL;
            //BSPshaderstore = (GLshaderverts*)malloc(sizeof(GLshaderverts) * numBSPverts);

            for (int i = 0; i < 16; i++) {
                numBSPverts[i] = 0;
                free(BSPshaderstore[i]);
                BSPshaderstore[i] = nullptr;
                BSPshaderstore[i] = (GLshaderverts*)malloc(sizeof(GLshaderverts) * 20000);
            }


            // reserve first 7 layers for water tiles in unit 0
            auto wtrtexture = this->hd_water_tile_anim[0];
            //terraintexmap.insert(std::make_pair("wtrtyl", terraintexmap.size()));
            //numterraintexloaded[0]++;
            bsptexturewidths[0] = wtrtexture->GetWidth();
            bsptextureheights[0] = wtrtexture->GetHeight();

            for (int buff = 0; buff < 7; buff++) {
                char container_name[64];
                sprintf(container_name, "HDWTR%03u", buff);

                bsptexmap.insert(std::make_pair(container_name, bsptexmap.size()));
                bsptexloaded[0]++;
            }


            for (int test = 0; test < pIndoor->pFaces.size(); test++) {
                BLVFace* face = &pIndoor->pFaces[test];

                if (face->Portal()) continue;
                if (!face->GetTexture()) continue;
                //if (face->uAttributes & FACE_IS_DOOR) continue;

                TextureOpenGL* tex = (TextureOpenGL*)face->GetTexture();
                std::string* texname = tex->GetName();

                int texunit = 0;
                int texlayer = 0;
                int attribflags = 0;

                if (face->uAttributes & FACE_IsFluid) attribflags |= 2;
                if (face->uAttributes & FACE_INDOOR_SKY) attribflags |= 0x400;

                if (face->uAttributes & FACE_FlowDown)
                    attribflags |= 0x400;
                else if (face->uAttributes & FACE_FlowUp)
                    attribflags |= 0x800;

                if (face->uAttributes & FACE_FlowRight)
                    attribflags |= 0x2000;
                else if (face->uAttributes & FACE_FlowLeft)
                    attribflags |= 0x1000;

                if (face->uAttributes & (FACE_OUTLINED | FACE_IsSecret))
                    attribflags |= 0x00010000;

                // check if tile->name is already in list
                auto mapiter = bsptexmap.find(*texname);
                if (mapiter != bsptexmap.end()) {
                    // if so, extract unit and layer
                    int unitlayer = mapiter->second;
                    texlayer = unitlayer & 0xFF;
                    texunit = (unitlayer & 0xFF00) >> 8;
                } else if (*texname == "wtrtyl") {
                    // water tile
                    texunit = 0;
                    texlayer = 0;
                } else {
                    // else need to add it
                    auto thistexture = assets->GetBitmap(*texname);
                    int width = thistexture->GetWidth();
                    int height = thistexture->GetHeight();
                    // check size to see what unit it needs
                    int i;
                    for (i = 0; i < 16; i++) {
                        if ((bsptexturewidths[i] == width && bsptextureheights[i] == height) || bsptexturewidths[i] == 0) break;
                    }

                    if (i == 16) {
                        logger->Warning("Texture unit full - draw Indoor faces!");
                        texunit = 0;
                        texlayer = 0;
                    } else {
                        if (bsptexturewidths[i] == 0) {
                            bsptexturewidths[i] = width;
                            bsptextureheights[i] = height;
                        }

                        texunit = i;
                        texlayer = bsptexloaded[i];

                        // encode unit and layer together
                        int encode = (texunit << 8) | texlayer;

                        if (bsptexloaded[i] < 256) {
                            // intsert into tex map
                            bsptexmap.insert(std::make_pair(*texname, encode));
                            bsptexloaded[i]++;
                        } else {
                            logger->Warning("Texture layer full - draw indoor faces!");
                            texunit = 0;
                            texlayer = 0;
                        }
                    }
                }

                face->texunit = texunit;
                face->texlayer = texlayer;
            }

            for (int l = 0; l < 16; l++) {
                glGenVertexArrays(1, &bspVAO[l]);
                glGenBuffers(1, &bspVBO[l]);

                glBindVertexArray(bspVAO[l]);
                glBindBuffer(GL_ARRAY_BUFFER, bspVBO[l]);

                glBufferData(GL_ARRAY_BUFFER, sizeof(GLshaderverts) * 20000, NULL, GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, x));
                glEnableVertexAttribArray(0);
                // tex uv attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, u));
                glEnableVertexAttribArray(1);
                // tex unit attribute
                // tex array layer attribute
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, texunit));
                glEnableVertexAttribArray(2);
                // normals
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, normx));
                glEnableVertexAttribArray(3);
                // attribs
                glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void *)offsetof(GLshaderverts, attribs));
                glEnableVertexAttribArray(4);
                //sector
                glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(GLshaderverts), (void*)offsetof(GLshaderverts, sector));
                glEnableVertexAttribArray(5);
            }

            // texture set up

            // loop over all units
            for (int unit = 0; unit < 16; unit++) {
                assert(bsptexloaded[unit] <= 256);
                // skip if textures are empty
                if (bsptexloaded[unit] == 0) continue;

                glGenTextures(1, &bsptextures[unit]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, bsptextures[unit]);

                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, bsptexturewidths[unit], bsptextureheights[unit], bsptexloaded[unit], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

                std::map<std::string, int>::iterator it = bsptexmap.begin();
                while (it != bsptexmap.end()) {
                    int comb = it->second;
                    int tlayer = comb & 0xFF;
                    int tunit = (comb & 0xFF00) >> 8;

                    if (tunit == unit) {
                        // get texture
                        auto texture = assets->GetBitmap(it->first);

                        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                            0,
                            0, 0, tlayer,
                            bsptexturewidths[unit], bsptextureheights[unit], 1,
                            GL_BGRA,
                            GL_UNSIGNED_BYTE,
                            texture->GetPixels(IMAGE_FORMAT_A8R8G8B8));

                        //numterraintexloaded[0]++;
                    }

                    it++;
                }




                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            }
        }


            // update verts - blank store

            for (int i = 0; i < 16; i++) {
                numBSPverts[i] = 0;
            }

            bool drawnsky = false;

            for (uint i = 0; i < pBspRenderer->num_faces; ++i) {
                // viewed through portal
                /*IndoorLocation::ExecDraw_d3d(pBspRenderer->faces[i].uFaceID,
                    pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].ViewportNodeFrustum,
                    4, pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].pPortalBounding);*/

                unsigned int uFaceID = pBspRenderer->faces[i].uFaceID;
                if (uFaceID >= pIndoor->pFaces.size())
                    continue;
                BLVFace* face = &pIndoor->pFaces[uFaceID];

                if (face->Portal()) {
                    continue;
                }

                if (face->uNumVertices < 3) continue;

                if (face->Invisible()) {
                    continue;
                }

                if (!face->GetTexture()) {
                    continue;
                }

                IndoorCameraD3D_Vec4* portalfrustumnorm = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].ViewportNodeFrustum;
                unsigned int uNumFrustums = 4;
                RenderVertexSoft* pPortalBounding = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].pPortalBounding;

                // uint ColourMask;  // ebx@25
                unsigned int uNumVerticesa;  // [sp+24h] [bp-4h]@17
                // int LightLevel;                     // [sp+34h] [bp+Ch]@25

                static RenderVertexSoft static_vertices_buff_in[64];  // buff in
                static RenderVertexSoft static_vertices_calc_out[64];  // buff out - calc portal shape

                static stru154 FacePlaneHolder;  // idb

                // moved face to camera check to avoid missing minimap outlines
                if (/*pCamera3D->is_face_faced_to_cameraBLV(face) ||*/ true) {
                    uNumVerticesa = face->uNumVertices;

                    // copy to buff in
                    for (uint i = 0; i < face->uNumVertices; ++i) {
                        static_vertices_buff_in[i].vWorldPosition.x = pIndoor->pVertices[face->pVertexIDs[i]].x;
                        static_vertices_buff_in[i].vWorldPosition.y = pIndoor->pVertices[face->pVertexIDs[i]].y;
                        static_vertices_buff_in[i].vWorldPosition.z = pIndoor->pVertices[face->pVertexIDs[i]].z;
                        static_vertices_buff_in[i].u = (signed short)face->pVertexUIDs[i];
                        static_vertices_buff_in[i].v = (signed short)face->pVertexVIDs[i];
                    }

                    // ceiling sky faces are not frustum culled
                    float skymodtimex{};
                    float skymodtimey{};
                    if (face->Indoor_sky()) {
                        if (face->uPolygonType != POLYGON_InBetweenFloorAndWall && face->uPolygonType != POLYGON_Floor) {
                            // draw forced perspective sky
                            DrawIndoorSky(face->uNumVertices, uFaceID);
                            continue;
                        } else {
                            skymodtimex = (OS_GetTime() / 32.0f) - pCamera3D->vCameraPos.x;
                            skymodtimey = (OS_GetTime() / 32.0f) + pCamera3D->vCameraPos.y;
                        }
                    }

                    // check if this face is visible through current portal node
                    if (pCamera3D->CullFaceToFrustum(static_vertices_buff_in, &uNumVerticesa, static_vertices_calc_out, portalfrustumnorm, 4)) {
                        face->uAttributes |= FACE_SeenByParty;

                        // check face is towards camera
                        if (pCamera3D->is_face_faced_to_cameraBLV(face)) {
                            ++pBLVRenderParams->uNumFacesRenderedThisFrame;
                            // load up verts here
                            int texlayer = 0;
                            int texunit = 0;
                            int attribflags = 0;

                            if (face->uAttributes & FACE_IsFluid) attribflags |= 2;

                            if (face->uAttributes & FACE_FlowDown)
                                attribflags |= 0x400;
                            else if (face->uAttributes & FACE_FlowUp)
                                attribflags |= 0x800;

                            if (face->uAttributes & FACE_FlowRight)
                                attribflags |= 0x2000;
                            else if (face->uAttributes & FACE_FlowLeft)
                                attribflags |= 0x1000;

                            if (face->uAttributes & (FACE_OUTLINED | FACE_IsSecret))
                                attribflags |= 0x00010000;

                            texlayer = face->texlayer;
                            texunit = face->texunit;

                            if (texlayer == -1) { // texture has been reset - see if its in the map
                                TextureOpenGL *tex = (TextureOpenGL *)face->GetTexture();
                                std::string *texname = tex->GetName();
                                auto mapiter = bsptexmap.find(*texname);
                                if (mapiter != bsptexmap.end()) {
                                    // if so, extract unit and layer
                                    int unitlayer = mapiter->second;
                                    face->texlayer = texlayer = unitlayer & 0xFF;
                                    face->texunit = texunit = (unitlayer & 0xFF00) >> 8;
                                } else {
                                    logger->Warning("Texture not found in map!");
                                    // TODO(pskelton): set to water for now - fountains in walls of mist
                                    texlayer = face->texlayer = 0;
                                    texunit = face->texunit = 0;
                                }
                            }


                            for (int z = 0; z < (face->uNumVertices - 2); z++) {
                                // 123, 134, 145, 156..
                                GLshaderverts *thisvert = &BSPshaderstore[texunit][numBSPverts[texunit]];

                                // copy first
                                thisvert->x = pIndoor->pVertices[face->pVertexIDs[0]].x;
                                thisvert->y = pIndoor->pVertices[face->pVertexIDs[0]].y;
                                thisvert->z = pIndoor->pVertices[face->pVertexIDs[0]].z;
                                thisvert->u = face->pVertexUIDs[0] + pIndoor->pFaceExtras[face->uFaceExtraID].sTextureDeltaU  /*+ face->sTextureDeltaU*/;
                                thisvert->v = face->pVertexVIDs[0] + pIndoor->pFaceExtras[face->uFaceExtraID].sTextureDeltaV  /*+ face->sTextureDeltaV*/;
                                if (face->Indoor_sky()) {
                                    thisvert->u = (skymodtimex + thisvert->u) * 0.25f;
                                    thisvert->v = (skymodtimey + thisvert->v) * 0.25f;
                                }
                                thisvert->texunit = texunit;
                                thisvert->texturelayer = texlayer;
                                thisvert->normx = face->pFacePlane.vNormal.x;
                                thisvert->normy = face->pFacePlane.vNormal.y;
                                thisvert->normz = face->pFacePlane.vNormal.z;
                                thisvert->attribs = attribflags;
                                thisvert->sector = face->uSectorID;
                                thisvert++;

                                // copy other two (z+1)(z+2)
                                for (uint i = 1; i < 3; ++i) {
                                    thisvert->x = pIndoor->pVertices[face->pVertexIDs[z + i]].x;
                                    thisvert->y = pIndoor->pVertices[face->pVertexIDs[z + i]].y;
                                    thisvert->z = pIndoor->pVertices[face->pVertexIDs[z + i]].z;
                                    thisvert->u = face->pVertexUIDs[z + i] + pIndoor->pFaceExtras[face->uFaceExtraID].sTextureDeltaU  /*+ face->sTextureDeltaU*/;
                                    thisvert->v = face->pVertexVIDs[z + i] + pIndoor->pFaceExtras[face->uFaceExtraID].sTextureDeltaV  /*+ face->sTextureDeltaV*/;
                                    if (face->Indoor_sky()) {
                                        thisvert->u = (skymodtimex + thisvert->u) * 0.25f;
                                        thisvert->v = (skymodtimey + thisvert->v) * 0.25f;
                                    }
                                    thisvert->texunit = texunit;
                                    thisvert->texturelayer = texlayer;
                                    thisvert->normx = face->pFacePlane.vNormal.x;
                                    thisvert->normy = face->pFacePlane.vNormal.y;
                                    thisvert->normz = face->pFacePlane.vNormal.z;
                                    thisvert->attribs = attribflags;
                                    thisvert->sector = face->uSectorID;
                                    thisvert++;
                                }

                                numBSPverts[texunit] += 3;
                                assert(numBSPverts[texunit] <= 19999);
                            }
                        }
                    }
                }
            }

            for (int l = 0; l < 16; l++) {
                if (numBSPverts[l]) {
                    glBindBuffer(GL_ARRAY_BUFFER, bspVBO[l]);
                    // orphan buffer
                    glBufferData(GL_ARRAY_BUFFER, sizeof(GLshaderverts) * 20000, NULL, GL_DYNAMIC_DRAW);
                    // update buffer
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLshaderverts) * numBSPverts[l], BSPshaderstore[l]);
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);

        // terrain debug
        if (engine->config->debug.Terrain.Get())
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        ////
        for (int unit = 0; unit < 16; unit++) {
            // skip if textures are empty
            //if (bsptexloaded[unit] > 0) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D_ARRAY, bsptextures[unit]);
            //}
        }

        //glBindVertexArray(bspVAO);
        //glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(1);
        //glEnableVertexAttribArray(2);
        //glEnableVertexAttribArray(3);
        //glEnableVertexAttribArray(4);

        glUseProgram(bspshader.ID);

        //// set projection
        glUniformMatrix4fv(glGetUniformLocation(bspshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
        //// set view
        glUniformMatrix4fv(glGetUniformLocation(bspshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

        glUniform1f(glGetUniformLocation(bspshader.ID, "gamma"), gamma);
        glUniform1i(glGetUniformLocation(bspshader.ID, "waterframe"), GLint(this->hd_water_current_frame));
        glUniform1i(glGetUniformLocation(bspshader.ID, "flowtimer"), GLint(platform->TickCount() >> 4));

        // set texture unit location
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray0"), GLint(0));
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray1"), GLint(1));
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray2"), GLint(2));


        GLfloat camera[3] {};
        camera[0] = (float)(pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0f));
        camera[1] = (float)(pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0f));
        camera[2] = (float)(pParty->vPosition.z + pParty->sEyelevel);
        glUniform3fv(glGetUniformLocation(bspshader.ID, "CameraPos"), 1, &camera[0]);


        // lighting stuff
        GLfloat sunvec[3] {};
        //sunvec[0] = 0;  // (float)pOutdoor->vSunlight.x / 65536.0;
        //sunvec[1] = 0;  // (float)pOutdoor->vSunlight.y / 65536.0;
        //sunvec[2] = 0;  // (float)pOutdoor->vSunlight.z / 65536.0;


        int16_t mintest = 0;

        for (int i = 0; i < pIndoor->pSectors.size(); i++) {
            mintest = std::max(mintest, pIndoor->pSectors[i].uMinAmbientLightLevel);
        }

        Lights.uCurrentAmbientLightLevel = (Lights.uDefaultAmbientLightLevel + mintest);

        float ambient = (248.0f - (Lights.uCurrentAmbientLightLevel << 3)) / 255.0f;
        //pParty->uCurrentMinute + pParty->uCurrentHour * 60.0;  // 0 - > 1439
    // ambient = 0.15 + (sinf(((ambient - 360.0) * 2 * pi_double) / 1440) + 1) * 0.27;

        float diffuseon = 0.0f;  // pWeather->bNight ? 0 : 1;

        glUniform3fv(glGetUniformLocation(bspshader.ID, "sun.direction"), 1, &sunvec[0]);
        glUniform3f(glGetUniformLocation(bspshader.ID, "sun.ambient"), ambient, ambient, ambient);
        glUniform3f(glGetUniformLocation(bspshader.ID, "sun.diffuse"), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f));
        glUniform3f(glGetUniformLocation(bspshader.ID, "sun.specular"), diffuseon * 1.0f, diffuseon * 0.8f, 0.0f);

        // point lights - fspointlights
        // rest of lights stacking
        GLuint num_lights = 0;   // 1;

        // get party torchlight as priority
        for (int i = 0; i < 1; ++i) {
            if (pMobileLightsStack->uNumLightsActive < 1) continue;

            auto test = pMobileLightsStack->pLights[i];
            std::string slotnum = std::to_string(num_lights);

            float x = pMobileLightsStack->pLights[i].vPosition.x;
            float y = pMobileLightsStack->pLights[i].vPosition.y;
            float z = pMobileLightsStack->pLights[i].vPosition.z;

            float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0f;
            float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0f;
            float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0f;

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

        // stack the static lights next (wall torches)
        for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            auto test = pStationaryLightsStack->pLights[i];

            // is this on the sector list
            bool onlist = false;
            for (uint i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
                if (test.uSectorID == listsector) {
                    onlist = true;
                    break;
                }
            }

            // does light sphere collide with current sector
            // expanded current sector
            bool fromexpanded{ false };
            BBoxs cursectorexpand = pIndoor->pSectors[pBLVRenderParams->uPartySectorID].pBounding.Expanded(test.uRadius);
            if (cursectorexpand.Contains(test.vPosition.ToShort())) {
                onlist = true;
                fromexpanded = true;
            }

            if (!onlist) continue;

            // cull through viewing frustum
            bool visinfrustum{ false };
            if (!fromexpanded) {
                for (int i = 0; i < pBspRenderer->num_nodes; ++i) {
                    if (pBspRenderer->nodes[i].uSectorID == test.uSectorID) {
                        if (IsSphereInFrustum(test.vPosition, test.uRadius, pBspRenderer->nodes[i].ViewportNodeFrustum))
                            visinfrustum = true;
                    }
                }
            } else {
                if (IsSphereInFrustum(test.vPosition, test.uRadius)) visinfrustum = true;
            }
            if (!visinfrustum) continue;

            std::string slotnum = std::to_string(num_lights);

            float x = test.vPosition.x;
            float y = test.vPosition.y;
            float z = test.vPosition.z;

            float r = test.uLightColorR / 255.0f;
            float g = test.uLightColorG / 255.0f;
            float b = test.uLightColorB / 255.0f;

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), test.uSectorID);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

        // whatevers left for mobile lights
        for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            // TODO(pskelton): nearest lights should be prioritsed
            auto test = pMobileLightsStack->pLights[i];
            if (!IsSphereInFrustum(test.vPosition, test.uRadius)) continue;

            std::string slotnum = std::to_string(num_lights);

            float x = pMobileLightsStack->pLights[i].vPosition.x;
            float y = pMobileLightsStack->pLights[i].vPosition.y;
            float z = pMobileLightsStack->pLights[i].vPosition.z;

            float r = pMobileLightsStack->pLights[i].uLightColorR / 255.0f;
            float g = pMobileLightsStack->pLights[i].uLightColorG / 255.0f;
            float b = pMobileLightsStack->pLights[i].uLightColorB / 255.0f;

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), r, g, b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

        if (engine->config->debug.VerboseLogging.Get())
            logger->Info("Lights this frame: %u", num_lights);

        // blank any lights not used
        for (int blank = num_lights; blank < 40; blank++) {
            std::string slotnum = std::to_string(blank);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 0.0);
        }



        // toggle for water faces or not
        glUniform1i(glGetUniformLocation(bspshader.ID, "watertiles"), GLint(1));

        glActiveTexture(GL_TEXTURE0);

        for (int unit = 0; unit < 16; unit++) {
            // skip if textures are empty
            //if (numoutbuildtexloaded[unit] > 0) {
            if (unit == 1) {
                glUniform1i(glGetUniformLocation(bspshader.ID, "watertiles"), GLint(0));
            }

            // draw each set of triangles
            glBindTexture(GL_TEXTURE_2D_ARRAY, bsptextures[unit]);
            glBindVertexArray(bspVAO[unit]);
            glDrawArrays(GL_TRIANGLES, 0, (numBSPverts[unit]));
            drawcalls++;
            //}
        }

        glUseProgram(0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);



        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);

        // indoor sky drawing
        if (forceperstorecnt) {
            // set forced matrixs
            _set_ortho_projection(1);
            _set_ortho_modelview();

            SkyBillboard.CalcSkyFrustumVec(1, 0, 0, 0, 1, 0);

            DrawForcePerVerts();

            _set_3d_projection_matrix();
            _set_3d_modelview_matrix();
        }


        //end terrain debug
        if (engine->config->debug.Terrain.Get())
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);




        // stack decals start

        if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;
        static stru154 FacePlaneHolder;
        static RenderVertexSoft static_vertices_buff_in[64];  // buff in

        // loop over faces
        for (int test = 0; test < pIndoor->pFaces.size(); test++) {
            BLVFace* pface = &pIndoor->pFaces[test];

            if (pface->Portal()) continue;
            if (!pface->GetTexture()) continue;

            // check if faces is visible
            bool onlist = false;
            for (uint i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
                if (pface->uSectorID == listsector) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;


            decal_builder->ApplyBloodsplatDecals_IndoorFace(test);
            if (!decal_builder->uNumSplatsThisFace) continue;

            FacePlaneHolder.face_plane.vNormal.x = pface->pFacePlane.vNormal.x;
            FacePlaneHolder.polygonType = pface->uPolygonType;
            FacePlaneHolder.face_plane.vNormal.y = pface->pFacePlane.vNormal.y;
            FacePlaneHolder.face_plane.vNormal.z = pface->pFacePlane.vNormal.z;
            FacePlaneHolder.face_plane.dist = pface->pFacePlane.dist;

            // copy to buff in
            for (uint i = 0; i < pface->uNumVertices; ++i) {
                static_vertices_buff_in[i].vWorldPosition.x =
                    pIndoor->pVertices[pface->pVertexIDs[i]].x;
                static_vertices_buff_in[i].vWorldPosition.y =
                    pIndoor->pVertices[pface->pVertexIDs[i]].y;
                static_vertices_buff_in[i].vWorldPosition.z =
                    pIndoor->pVertices[pface->pVertexIDs[i]].z;
                static_vertices_buff_in[i].u = (signed short)pface->pVertexUIDs[i];
                static_vertices_buff_in[i].v = (signed short)pface->pVertexVIDs[i];
            }

            // blood draw
            decal_builder->BuildAndApplyDecals(Lights.uCurrentAmbientLightLevel, LocationIndoors, &FacePlaneHolder,
                pface->uNumVertices, static_vertices_buff_in,
                0, pface->uSectorID);
        }



        ///////////////////////////////////////////////////////
        // stack decals end

        return;
}

// d3d only
void RenderOpenGL::DrawIndoorBatched() { return; }

// no longer require with shaders
void RenderOpenGL::DrawPolygon(struct Polygon *pPolygon) { return; }
void RenderOpenGL::DrawIndoorPolygon(unsigned int uNumVertices, BLVFace *pFace, int uPackedID, unsigned int uColor, int a8) { return; }

bool RenderOpenGL::SwitchToWindow() {
    // pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    // pViewport->ResetScreen();
    // CreateZBuffer();

    return true;
}


bool RenderOpenGL::Initialize() {
    if (!RenderBase::Initialize()) {
        return false;
    }

    if (window != nullptr) {
        PlatformOpenGLOptions opts;

        //  Use OpenGL 4.1 core
        opts.versionMajor = 4;
        opts.versionMinor = 1;
        opts.profile = CoreProfile;

        //  Turn on 24bit Z buffer.
        //  You may need to change this to 16 or 32 for your system
        opts.depthBits = 24;
        opts.stencilBits = 8;

        opts.vsyncMode = config->graphics.VSync.Get() ? AdaptiveVSync : NoVSync;

        context_ = window->CreateOpenGLContext(opts);
        ::openGlContext = context_.get();

        auto gladLoadFunc = [](void *ptr, const char *name) {
            return reinterpret_cast<GLADapiproc>(static_cast<PlatformOpenGLContext *>(ptr)->GetProcAddress(name));
        };

        int version = gladLoadGLUserPtr(gladLoadFunc, context_.get());
        if (!version)
            log->Warning("GLAD: Failed to initialize the OpenGL loader");

        log->Info("SDL2: supported OpenGL: %s", glGetString(GL_VERSION));
        log->Info("SDL2: supported GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        log->Info("SDL2: OpenGL version: %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        gladSetGLPostCallback(GL_Check_Errors);

        return Reinitialize(true);
    }

    return false;
}

// do not use
void RenderOpenGL::WritePixel16(int x, int y, uint16_t color) { return; }

void RenderOpenGL::FillRectFast(unsigned int uX, unsigned int uY,
                                unsigned int uWidth, unsigned int uHeight,
                                unsigned int uColor16) {
    float b = ((uColor16 & 0x1F) * 8) / 255.0f;
    float g = (((uColor16 >> 5) & 0x3F) * 4) / 255.0f;
    float r = (((uColor16 >> 11) & 0x1F) * 8) / 255.0f;

    float depth = 0;
    int x = uX;
    int y = uY;
    int z = x + uWidth;
    int w = y + uHeight;

    // check bounds
    if (x >= output.w || x >= this->clip_z || y >= output.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;


    static Texture* effpar03 = assets->GetBitmap("effpar03");
    auto texture = (TextureOpenGL*)effpar03;
    float gltexid = static_cast<float>(texture->GetOpenGlTexture());

    float drawx = static_cast<float>(std::max(x, this->clip_x));
    float drawy = static_cast<float>(std::max(y, this->clip_y));
    float draww = static_cast<float>(std::min(w, this->clip_w));
    float drawz = static_cast<float>(std::min(z, this->clip_z));

    float texx = 0.5f;
    float texy = 0.5f;
    float texz = 0.5f;
    float texw = 0.5f;

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].r = r;
    twodshaderstore[twodvertscnt].g = g;
    twodshaderstore[twodvertscnt].b = b;
    twodshaderstore[twodvertscnt].a = 1;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

// gl shaders
bool RenderOpenGL::InitShaders() {
    if (!std::filesystem::exists(MakeDataPath("shaders"))) {
        logger->Warning("\n ----------- ERROR ----------- \nExpected to find folder:\n\n %s\n", MakeDataPath("shaders").c_str());
        logger->Warning("Please create it and copy shader files in!\n\n --------------------------------------------");
        return false;
    }

    if (std::filesystem::is_empty(MakeDataPath("shaders"))) {
        logger->Warning("\n ----------- ERROR ----------- \nShader folder %s is empty", MakeDataPath("shaders").c_str());
        logger->Warning(" Please copy shader files in! \n ---------------------------------------\n");
        return false;
    }

    logger->Info("Initialising GL shaders!");

    terrainshader.build(MakeDataPath("shaders", "glterrain.vert").c_str(), MakeDataPath("shaders", "glterrain.frag").c_str());
    if (terrainshader.ID == 0) {
        logger->Warning("Terrain shader failed to compile!");
        return false;
    }

    outbuildshader.build(MakeDataPath("shaders", "gloutbuild.vert").c_str(), MakeDataPath("shaders", "gloutbuild.frag").c_str());
    if (outbuildshader.ID == 0) {
        logger->Warning("Outdoors building shader failed to compile!");
        return false;
    }

    bspshader.build(MakeDataPath("shaders", "glbspshader.vert").c_str(), MakeDataPath("shaders", "glbspshader.frag").c_str());
    if (bspshader.ID == 0) {
        logger->Warning("Indoors bsp shader failed to compile!");
        return false;
    }

    textshader.build(MakeDataPath("shaders", "gltextshader.vert").c_str(), MakeDataPath("shaders", "gltextshader.frag").c_str());
    if (textshader.ID == 0) {
        logger->Warning("Text shader failed to compile!");
        return false;
    }
    textVAO = 0;

    lineshader.build(MakeDataPath("shaders", "gllinesshader.vert").c_str(), MakeDataPath("shaders", "gllinesshader.frag").c_str());
    if (lineshader.ID == 0) {
        logger->Warning("Lines shader failed to compile!");
        return false;
    }
    lineVAO = 0;

    twodshader.build(MakeDataPath("shaders", "gltwodshader.vert").c_str(), MakeDataPath("shaders", "gltwodshader.frag").c_str());
    if (twodshader.ID == 0) {
        logger->Warning("2d shader failed to compile!");
        return false;
    }
    twodVAO = 0;

    billbshader.build(MakeDataPath("shaders", "glbillbshader.vert").c_str(), MakeDataPath("shaders", "glbillbshader.frag").c_str());
    if (billbshader.ID == 0) {
        logger->Warning("Billboards shader failed to compile!");
        return false;
    }
    billbVAO = 0;
    palbuf = 0;

    decalshader.build(MakeDataPath("shaders", "gldecalshader.vert").c_str(), MakeDataPath("shaders", "gldecalshader.frag").c_str());
    if (decalshader.ID == 0) {
        logger->Warning("Decals shader failed to compile!");
        return false;
    }
    decalVAO = 0;

    forcepershader.build(MakeDataPath("shaders", "glforcepershader.vert").c_str(), MakeDataPath("shaders", "glforcepershader.frag").c_str());
    if (forcepershader.ID == 0) {
        logger->Warning("Forced perspective shader failed to compile!");
        return false;
    }
    forceperVAO = 0;

    logger->Info("Shaders compiled successfully");
    return true;
}

bool RenderOpenGL::Reinitialize(bool firstInit) {
    output = window->Size();

    if (!firstInit) {
        game_viewport_x = viewparams->uScreen_topL_X = engine->config->graphics.ViewPortX1.Get(); //8
        game_viewport_y = viewparams->uScreen_topL_Y = engine->config->graphics.ViewPortY1.Get(); //8
        game_viewport_z = viewparams->uScreen_BttmR_X = output.w - engine->config->graphics.ViewPortX2.Get(); //468;
        game_viewport_w = viewparams->uScreen_BttmR_Y = output.h - engine->config->graphics.ViewPortY2.Get(); //352;

        game_viewport_width = game_viewport_z - game_viewport_x;
        game_viewport_height = game_viewport_w - game_viewport_y;

        viewparams->uSomeY = viewparams->uScreen_topL_Y;
        viewparams->uSomeX = viewparams->uScreen_topL_X;
        viewparams->uSomeZ = viewparams->uScreen_BttmR_X;
        viewparams->uSomeW = viewparams->uScreen_BttmR_Y;

        pViewport->SetScreen(viewparams->uScreen_topL_X, viewparams->uScreen_topL_Y,
                            viewparams->uScreen_BttmR_X,
                            viewparams->uScreen_BttmR_Y);
    }

    pViewport->ResetScreen();
    CreateZBuffer();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // Black Background
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, output.w, output.h);
    glScissor(0, 0, output.w, output.h);
    glEnable(GL_SCISSOR_TEST);

    // Swap Buffers (Double Buffering)
    openGlContext->SwapBuffers();

    this->clip_x = this->clip_y = 0;
    this->clip_z = output.w;
    this->clip_w = output.h;

    // PostInitialization();

    // check gpu gl capability params
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GPU_MAX_TEX_SIZE);
    assert(GPU_MAX_TEX_SIZE >= 512);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &GPU_MAX_TEX_LAYERS);
    assert(GPU_MAX_TEX_LAYERS >= 256);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &GPU_MAX_TEX_UNITS);
    assert(GPU_MAX_TEX_UNITS >= 16);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &GPU_MAX_UNIFORM_COMP);
    assert(GPU_MAX_UNIFORM_COMP >= 1024);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &GPU_MAX_TOTAL_TEXTURES);
    assert(GPU_MAX_TOTAL_TEXTURES >= 80);

    if (firstInit) {
        // initiate shaders
        if (!InitShaders()) {
            logger->Warning("--- Shader initialisation has failed ---");
            return false;
        }
    } else {
        // TODO: invalidate all previously loaded textures and then load them again as they can be no longer alive on GPU (issue #199).

        ReloadShaders();
    }

    return true;
}

void RenderOpenGL::ReloadShaders() {
    logger->Info("Reloading Shaders...");
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (!terrainshader.reload()) logger->Warning("Terrain shader failed to reload!");
    if (!outbuildshader.reload()) logger->Warning("Outdoor buildings shader failed to reload!");
    ReleaseTerrain();

    if (!bspshader.reload()) logger->Warning("BSP shader failed to reload!");
    ReleaseBSP();

    if (!textshader.reload()) logger->Warning("Text shader faied to reload!");
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    textVAO = textVBO = 0;
    textvertscnt = 0;

    if (!lineshader.reload()) logger->Warning("Line shader failed to reload!");
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    lineVAO = lineVBO = 0;
    linevertscnt = 0;

    if (!twodshader.reload()) logger->Warning("2d shader failed to reload!");
    glDeleteVertexArrays(1, &twodVAO);
    glDeleteBuffers(1, &twodVBO);
    twodVAO = twodVBO = 0;
    twodvertscnt = 0;

    if (!billbshader.reload()) logger->Warning("Billboard shader failed to reload!");
    glDeleteVertexArrays(1, &billbVAO);
    glDeleteBuffers(1, &billbVBO);
    billbVAO = billbVBO = 0;
    glDeleteTextures(1, &paltex);
    glDeleteBuffers(1, &palbuf);
    paltex = palbuf = 0;
    billbstorecnt = 0;

    if (!decalshader.reload()) logger->Warning("Decal shader failed to reload!");
    glDeleteVertexArrays(1, &decalVAO);
    glDeleteBuffers(1, &decalVBO);
    decalVAO = decalVBO = 0;
    numdecalverts = 0;

    if (!forcepershader.reload()) logger->Warning("Forced perspective shader failed to reload!");
    glDeleteVertexArrays(1, &forceperVAO);
    glDeleteBuffers(1, &forceperVBO);
    forceperVAO = forceperVBO = 0;
    forceperstorecnt = 0;

    if (nuklearshader.ID != 0) {
        if (!nuklearshader.reload()) {
            logger->Warning("Nuklear shader failed to reload!");
        } else {
            nk_dev.uniform_tex = glGetUniformLocation(nuklearshader.ID, "Texture");
            nk_dev.uniform_proj = glGetUniformLocation(nuklearshader.ID, "ProjMtx");
            nk_dev.attrib_pos = glGetAttribLocation(nuklearshader.ID, "Position");
            nk_dev.attrib_uv = glGetAttribLocation(nuklearshader.ID, "TexCoord");
            nk_dev.attrib_col = glGetAttribLocation(nuklearshader.ID, "Color");
        }
    }
}

void RenderOpenGL::ReleaseTerrain() {
    /*GLuint terrainVBO, terrainVAO;
    GLuint terraintextures[8];
    uint numterraintexloaded[8];
    uint terraintexturesizes[8];
    std::map<std::string, int> terraintexmap;*/

    terraintexmap.clear();

    for (int i = 0; i < 8; i++) {
        glDeleteTextures(1, &terraintextures[i]);
        terraintextures[i] = 0;
        numterraintexloaded[i] = 0;
        terraintexturesizes[i] = 0;
    }

    glDeleteBuffers(1, &terrainVBO);
    glDeleteVertexArrays(1, &terrainVAO);

    terrainVBO = 0;
    terrainVAO = 0;

    /*GLuint outbuildVBO, outbuildVAO;
    GLuint outbuildtextures[8];
    uint numoutbuildtexloaded[8];
    uint outbuildtexturewidths[8];
    uint outbuildtextureheights[8];
    std::map<std::string, int> outbuildtexmap;*/

    outbuildtexmap.clear();

    for (int i = 0; i < 16; i++) {
        glDeleteTextures(1, &outbuildtextures[i]);
        outbuildtextures[i] = 0;
        numoutbuildtexloaded[i] = 0;
        outbuildtexturewidths[i] = 0;
        outbuildtextureheights[i] = 0;
        glDeleteBuffers(1, &outbuildVBO[i]);
        glDeleteVertexArrays(1, &outbuildVAO[i]);
        outbuildVBO[i] = 0;
        outbuildVAO[i] = 0;
        if (outbuildshaderstore[i]) {
            free(outbuildshaderstore[i]);
            outbuildshaderstore[i] = nullptr;
        }
    }
}

void RenderOpenGL::ReleaseBSP() {
    /*GLuint bspVBO, bspVAO;
    GLuint bsptextures[16];
    uint bsptexloaded[16];
    uint bsptexturewidths[16];
    uint bsptextureheights[16];
    std::map<std::string, int> bsptexmap;*/

    bsptexmap.clear();

    for (int i = 0; i < 16; i++) {
        glDeleteTextures(1, &bsptextures[i]);
        bsptextures[i] = 0;
        bsptexloaded[i] = 0;
        bsptexturewidths[i] = 0;
        bsptextureheights[i] = 0;
        glDeleteBuffers(1, &bspVBO[i]);
        glDeleteVertexArrays(1, &bspVAO[i]);
        bspVAO[i] = 0;
        bspVBO[i] = 0;
        if (BSPshaderstore[i]) {
            free(BSPshaderstore[i]);
            BSPshaderstore[i] = nullptr;
        }
    }
}


void RenderOpenGL::DrawTwodVerts() {
    if (!twodvertscnt) return;

    int savex = this->clip_x;
    int savey = this->clip_y;
    int savez = this->clip_z;
    int savew = this->clip_w;
    render->ResetUIClipRect();

    if (twodVAO == 0) {
        glGenVertexArrays(1, &twodVAO);
        glGenBuffers(1, &twodVBO);

        glBindVertexArray(twodVAO);
        glBindBuffer(GL_ARRAY_BUFFER, twodVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(twodshaderstore), twodshaderstore, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, x));
        glEnableVertexAttribArray(0);
        // tex uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, u));
        glEnableVertexAttribArray(1);
        // colour
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, r));
        glEnableVertexAttribArray(2);
        // texid
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, texid));
        glEnableVertexAttribArray(3);
        // paletteid
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, paletteid));
        glEnableVertexAttribArray(4);
    }

    if (palbuf == 0) {
        // generate palette buffer texture
        glGenBuffers(1, &palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, pPaletteManager->GetGLPaletteSize(), pPaletteManager->GetGLPalettePtr(), GL_STATIC_DRAW);

        glGenTextures(1, &paltex);
        glBindTexture(GL_TEXTURE_BUFFER, paltex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        pPaletteManager->GLPaletteReset();
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, twodVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(twodverts) * twodvertscnt, twodshaderstore);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (pPaletteManager->GetGLPaletteNeedsUpdate()) {
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, pPaletteManager->GetGLPaletteSize(), pPaletteManager->GetGLPalettePtr(), GL_STATIC_DRAW);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        pPaletteManager->GLPaletteReset();
    }

    glBindVertexArray(twodVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glUseProgram(twodshader.ID);

    // set sampler to palette
    glUniform1i(glGetUniformLocation(twodshader.ID, "palbuf"), GLint(1));
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_BUFFER, paltex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
    glActiveTexture(GL_TEXTURE0);

    GLboolean repaint = !engine->config->graphics.HWLSprites.Get();
    glUniform1i(glGetUniformLocation(twodshader.ID, "repaint"), repaint);

    // glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // set sampler to texure0
    glUniform1i(glGetUniformLocation(twodshader.ID, "texture0"), GLint(0));

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(twodshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(twodshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    int offset = 0;
    while (offset < twodvertscnt) {
        // set texture
        GLfloat thistex = twodshaderstore[offset].texid;
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(twodshaderstore[offset].texid));
        if (repaint && twodshaderstore[offset].paletteid) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        int cnt = 0;
        do {
            cnt++;
            if (offset + (6 * cnt) > twodvertscnt) {
                --cnt;
                break;
            }
        } while (twodshaderstore[offset + (cnt * 6)].texid == thistex);

        glDrawArrays(GL_TRIANGLES, offset, (6*cnt));
        drawcalls++;

        offset += (6*cnt);
    }

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    twodvertscnt = 0;
    render->SetUIClipRect(savex, savey, savez, savew);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


bool RenderOpenGL::NuklearInitialize(struct nk_tex_font *tfont) {
    struct nk_context* nk_ctx = nuklear->ctx;
    if (!nk_ctx) {
        log->Warning("Nuklear context is not available");
        return false;
    }

    if (!NuklearCreateDevice()) {
        log->Warning("Nuklear device creation failed");
        NuklearRelease();
        return false;
    }

    nk_font_atlas_init_default(&nk_dev.atlas);
    struct nk_tex_font *font = NuklearFontLoad(NULL, 13);
    nk_dev.atlas.default_font = font->font;
    if (!nk_dev.atlas.default_font) {
        log->Warning("Nuklear default font loading failed");
        NuklearRelease();
        return false;
    }

    memcpy(tfont, font, sizeof(struct nk_tex_font));

    if (!nk_init_default(nk_ctx, &nk_dev.atlas.default_font->handle)) {
        log->Warning("Nuklear initialization failed");
        NuklearRelease();
        return false;
    }

    nk_buffer_init_default(&nk_dev.cmds);

    return true;
}

bool RenderOpenGL::NuklearCreateDevice() {
    nuklearshader.build(MakeDataPath("shaders", "glnuklear.vert").c_str(), MakeDataPath("shaders", "glnuklear.frag").c_str());
    if (nuklearshader.ID == 0) {
        logger->Warning("Nuklear shader failed to compile!");
        return false;
    }

    nk_buffer_init_default(&nk_dev.cmds);
    nk_dev.uniform_tex = glGetUniformLocation(nuklearshader.ID, "Texture");
    nk_dev.uniform_proj = glGetUniformLocation(nuklearshader.ID, "ProjMtx");
    nk_dev.attrib_pos = glGetAttribLocation(nuklearshader.ID, "Position");
    nk_dev.attrib_uv = glGetAttribLocation(nuklearshader.ID, "TexCoord");
    nk_dev.attrib_col = glGetAttribLocation(nuklearshader.ID, "Color");
    {
        GLsizei vs = sizeof(struct nk_vertex);
        size_t vp = offsetof(struct nk_vertex, position);
        size_t vt = offsetof(struct nk_vertex, uv);
        size_t vc = offsetof(struct nk_vertex, col);

        glGenBuffers(1, &nk_dev.vbo);
        glGenBuffers(1, &nk_dev.ebo);
        glGenVertexArrays(1, &nk_dev.vao);

        glBindVertexArray(nk_dev.vao);
        glBindBuffer(GL_ARRAY_BUFFER, nk_dev.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nk_dev.ebo);

        glEnableVertexAttribArray((GLuint)nk_dev.attrib_pos);
        glEnableVertexAttribArray((GLuint)nk_dev.attrib_uv);
        glEnableVertexAttribArray((GLuint)nk_dev.attrib_col);

        glVertexAttribPointer((GLuint)nk_dev.attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)nk_dev.attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)nk_dev.attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool RenderOpenGL::NuklearRender(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer) {
    struct nk_context *nk_ctx = nuklear->ctx;
    if (!nk_ctx)
        return false;

    if (!nk_ctx->begin)
        return false;


    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale {};
    GLfloat ortho[4][4] = {
        { 2.0f,  0.0f,  0.0f,  0.0f },
        { 0.0f, -2.0f,  0.0f,  0.0f },
        { 0.0f,  0.0f, -1.0f,  0.0f },
        { -1.0f, 1.0f,  0.0f,  1.0f },
    };

    height = output.h;
    width = output.w;
    display_height = render->GetRenderHeight();
    display_width = render->GetRenderWidth();

    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    scale.x = (float)display_width / (float)width;
    scale.y = (float)display_height / (float)height;

    /* setup global state */
    glViewport(0, 0, display_width, display_height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(nuklearshader.ID);
    glUniform1i(nk_dev.uniform_tex, 0);
    glUniformMatrix4fv(nk_dev.uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* allocate vertex and element buffer */
        glBindVertexArray(nk_dev.vao);
        glBindBuffer(GL_ARRAY_BUFFER, nk_dev.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nk_dev.ebo);

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load vertices/elements directly into vertex/element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_vertex);
            config.null = nk_dev.null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (nk_size)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (nk_size)max_element_buffer);
            nk_convert(nk_ctx, &nk_dev.cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, nk_ctx, &nk_dev.cmds) {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor((GLint)(cmd->clip_rect.x * scale.x),
                (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                (GLint)(cmd->clip_rect.w * scale.x),
                (GLint)(cmd->clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            ++drawcalls;
            offset += cmd->elem_count;
        }
        nk_clear(nk_ctx);
        nk_buffer_clear(&nk_dev.cmds);
    }

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    // glDisable(GL_SCISSOR_TEST);

    return true;
}

void RenderOpenGL::NuklearRelease() {
    nk_font_atlas_clear(&nk_dev.atlas);

    glDeleteProgram(nuklearshader.ID);
    glDeleteBuffers(1, &nk_dev.vbo);
    glDeleteBuffers(1, &nk_dev.ebo);
    glDeleteVertexArrays(1, &nk_dev.vao);

    nk_buffer_free(&nk_dev.cmds);

    memset(&nk_dev, 0, sizeof(nk_dev));
}

struct nk_tex_font *RenderOpenGL::NuklearFontLoad(const char* font_path, size_t font_size) {
    const void *image;
    int w, h;
    GLuint texid;

    struct nk_tex_font *tfont = new (struct nk_tex_font);
    if (!tfont)
        return NULL;

    struct nk_font_config cfg = nk_font_config(font_size);
    cfg.merge_mode = nk_false;
    cfg.coord_type = NK_COORD_UV;
    cfg.spacing = nk_vec2(0, 0);
    cfg.oversample_h = 3;
    cfg.oversample_v = 1;
    cfg.range = nk_font_cyrillic_glyph_ranges();
    cfg.size = font_size;
    cfg.pixel_snap = 0;
    cfg.fallback_glyph = '?';

    nk_font_atlas_begin(&nk_dev.atlas);

    if (!font_path)
        tfont->font = nk_font_atlas_add_default(&nk_dev.atlas, font_size, 0);
    else
        tfont->font = nk_font_atlas_add_from_file(&nk_dev.atlas, font_path, font_size, &cfg);

    image = nk_font_atlas_bake(&nk_dev.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    tfont->texid = texid;
    nk_font_atlas_end(&nk_dev.atlas, nk_handle_id(texid), &nk_dev.null);

    return tfont;
}

void RenderOpenGL::NuklearFontFree(struct nk_tex_font *tfont) {
    if (tfont)
        glDeleteTextures(1, &tfont->texid);
}

struct nk_image RenderOpenGL::NuklearImageLoad(Image *img) {
    GLuint texid;
    auto t = (TextureOpenGL *)img;
    texid = t->GetOpenGlTexture();

    t->SetOpenGlTexture(texid);
    return nk_image_id(texid);
}

void RenderOpenGL::NuklearImageFree(Image *img) {
    auto t = (TextureOpenGL *)img;
    GLuint texid = t->GetOpenGlTexture();
    if (texid != -1) {
        glDeleteTextures(1, &texid);
    }
}
