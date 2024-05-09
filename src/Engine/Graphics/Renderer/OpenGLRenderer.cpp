#include "OpenGLRenderer.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <map>
#include <string>

#include <glad/gl.h> // NOLINT: not a C system header.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightsStack.h"
#include "OpenGLShader.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/TextureFrameTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Arcomage/Arcomage.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineCallObserver.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Image/ImageFunctions.h"
#include "Library/Color/Colorf.h"
#include "Library/Logger/Logger.h"
#include "Library/Geometry/Size.h"

#include "Utility/String/Format.h"
#include "Utility/Memory/MemSet.h"

#include "NuklearOverlayRenderer.h"

#ifndef LOWORD
    #define LOWORD(l) ((unsigned short)(((std::uintptr_t)(l)) & 0xFFFF))
#endif

static constexpr int DEFAULT_AMBIENT_LIGHT_LEVEL = 0;

// globals
//TODO(pskelton): Combine and contain
int uNumDecorationsDrawnThisFrame;
RenderBillboard pBillboardRenderList[500];
unsigned int uNumBillboardsToDraw;
int uNumSpritesDrawnThisFrame;
RenderVertexSoft array_73D150[20];
RenderVertexSoft VertexRenderList[50];
RenderVertexD3D3 d3d_vertex_buffer[50];
RenderVertexSoft array_507D30[50];

static GLuint framebuffer = 0;
static GLuint framebufferTextures[2] = {0, 0};
static bool OpenGLES = false;

namespace detail_gl_error {
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(GLenum, CASE_SENSITIVE, {
    { GL_INVALID_OPERATION, "INVALID_OPERATION" },
    { GL_INVALID_ENUM, "INVALID_ENUM" },
    { GL_INVALID_VALUE, "INVALID_VALUE" },
    { GL_OUT_OF_MEMORY, "OUT_OF_MEMORY" },
    { GL_INVALID_FRAMEBUFFER_OPERATION, "INVALID_FRAMEBUFFER_OPERATION" }
})
} // namespace detail_gl_error

// improved error check - using glad post call back
void GL_Check_Errors(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLenum err = glad_glGetError();

    while (err != GL_NO_ERROR) {
        static std::string error;
        if (!detail_gl_error::trySerialize(err, &error))
            error = "Unknown Error";

        logger->warning("OpenGL error ({}): {} from function {}", err, error, name);

        err = glad_glGetError();
    }
}

namespace detail_fb_error {
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(GLenum, CASE_SENSITIVE, {
    {GL_FRAMEBUFFER_UNDEFINED, "framebuffer is undefined"},
    {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "framebuffer has missing attachment"},
    {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "framebuffer has incomplete attachment"},
    {GL_FRAMEBUFFER_UNSUPPORTED, "framebuffer is unsupported"},
    {GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "framebuffer has incomplete multisample"},
    {0, "unknown error"}
})
} // namespace detail_fb_error

void GL_Check_Framebuffer(const char *name) {
    static std::string error;

    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (!detail_fb_error::trySerialize(status, &error))
        return;

    logger->warning("OpenGL Framebuffer error ({}): {} from function {}", status, error, name);
}

// sky billboard stuff

void SkyBillboardStruct::CalcSkyFrustumVec(int x1, int y1, int z1, int x2, int y2, int z2) {
    // 6 0 0 0 6 0

    // TODO(pskelton): clean up and move out of here

    float cosz = pCamera3D->_yawRotationCosine;
    float cosx = pCamera3D->_pitchRotationCosine;
    float sinz = pCamera3D->_yawRotationSine;
    float sinx = pCamera3D->_pitchRotationSine;

    // positions all minus ?
    float v11 = cosz * -pCamera3D->vCameraPos.x + sinz * -pCamera3D->vCameraPos.y;
    float v24 = cosz * -pCamera3D->vCameraPos.y - sinz * -pCamera3D->vCameraPos.x;

    // cam position transform
    if (pCamera3D->_viewPitch) {
        this->field_0_party_dir_x = (v11 * cosx) + (-pCamera3D->vCameraPos.z * sinx);
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z * cosx) /*-*/ + (v11 * sinx);
    } else {
        this->field_0_party_dir_x = v11;
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z);
    }

    // set 1 position transfrom (6 0 0) looks like cam left vector
    if (pCamera3D->_viewPitch) {
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
    if (pCamera3D->_viewPitch) {
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

OpenGLRenderer::OpenGLRenderer(
    std::shared_ptr<GameConfig> config,
    DecalBuilder *decal_builder,
    SpellFxRenderer *spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis *vis
) : BaseRenderer(config, decal_builder, spellfx, particle_engine, vis) {
    clip_w = 0;
    clip_x = 0;
    clip_y = 0;
    clip_z = 0;
}

OpenGLRenderer::~OpenGLRenderer() { logger->info("RenderGl - Destructor"); }

void OpenGLRenderer::Release() { logger->info("RenderGL - Release"); }

RgbaImage OpenGLRenderer::ReadScreenPixels() {
    RgbaImage result = RgbaImage::uninitialized(outputRender.w, outputRender.h);
    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    }
    glReadPixels(0, 0, outputRender.w, outputRender.h, GL_RGBA, GL_UNSIGNED_BYTE, result.pixels().data());
    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
    return result;
}

bool OpenGLRenderer::InitializeFullscreen() {
    // pViewport->ResetScreen();
    // CreateZBuffer();

    return true;
}

// when losing and regaining window focus - not required for OGL??
void OpenGLRenderer::RestoreFrontBuffer() { logger->info("RenderGl - RestoreFrontBuffer"); }
void OpenGLRenderer::RestoreBackBuffer() { logger->info("RenderGl - RestoreBackBuffer"); }

void OpenGLRenderer::BltBackToFontFast(int a2, int a3, Recti *a4) {
    logger->info("RenderGl - BltBackToFontFast");
    // never called anywhere
}

void OpenGLRenderer::ClearTarget(Color uColor) {
    /* TODO(Gerark) Should we bind to the framebuffer before clearing?
    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    }
    */

    glClearColor(0, 0, 0, 0/*0.9f, 0.5f, 0.1f, 1.0f*/);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}

struct linesverts {
    GLfloat x;
    GLfloat y;
    Colorf color;
};

linesverts lineshaderstore[2000] = {};
int linevertscnt = 0;

void OpenGLRenderer::BeginLines2D() {
    if (linevertscnt)
        logger->trace("BeginLines with points still stored in buffer");

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
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(linesverts), (void *)offsetof(linesverts, color));
        glEnableVertexAttribArray(1);
    }
}

void OpenGLRenderer::EndLines2D() {
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

void OpenGLRenderer::RasterLine2D(int uX, int uY, int uZ, int uW, Color uColor32) {
    Colorf cf = uColor32.toColorf();

    lineshaderstore[linevertscnt].x = static_cast<float>(uX);
    lineshaderstore[linevertscnt].y = static_cast<float>(uY);
    lineshaderstore[linevertscnt].color = cf;
    linevertscnt++;

    lineshaderstore[linevertscnt].x = static_cast<float>(uZ);
    lineshaderstore[linevertscnt].y = static_cast<float>(uW);
    lineshaderstore[linevertscnt].color = cf;
    linevertscnt++;

    // draw if buffer full
    if (linevertscnt == 2000) EndLines2D();
}

// used for debug protal lines
void OpenGLRenderer::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {
    BeginLines2D();
    for (unsigned i = 0; i < num_vertices - 1; ++i) {
        Colorf color0 = vertices[i].diffuse.toColorf();
        Colorf color1 = vertices[i + 1].diffuse.toColorf();

        lineshaderstore[linevertscnt].x = vertices[i].pos.x;
        lineshaderstore[linevertscnt].y = vertices[i].pos.y;
        lineshaderstore[linevertscnt].color = color0;
        linevertscnt++;

        lineshaderstore[linevertscnt].x = vertices[i + 1].pos.x + 0.5f;
        lineshaderstore[linevertscnt].y = vertices[i + 1].pos.y + 0.5f;
        lineshaderstore[linevertscnt].color = color1;
        linevertscnt++;

        // draw if buffer full
        if (linevertscnt == 2000) EndLines2D();
    }
    EndLines2D();
}

void OpenGLRenderer::BeginScene3D() {
    // Setup for 3D

    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextures[0], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufferTextures[1], 0);

        GL_Check_Framebuffer(__FUNCTION__);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 0/*0.9f, 0.5f, 0.1f, 1.0f*/);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render->uNumBillboardsToDraw = 0;  // moved from drawbillboards - cant reset this until mouse picking finished

    SetFogParametersGL();
    gamma = GetGamma();
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
    Colorf color;
    GLfloat texid;
};

const int MAX_FORCEPERSTORECNT{ 498 };
forcepersverts forceperstore[MAX_FORCEPERSTORECNT]{};
int forceperstorecnt{ 0 };


void OpenGLRenderer::DrawProjectile(float srcX, float srcY, float srcworldview, float srcfovoworldview,
                                    float dstX, float dstY, float dstworldview, float dstfovoworldview,
                                    GraphicsImage *texture) {
    // billboards projectile - lightning bolt

    int xDifference = bankersRounding(dstX - srcX);
    int yDifference = bankersRounding(dstY - srcY);
    int absYDifference = std::abs(yDifference);
    int absXDifference = std::abs(xDifference);
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
    v29[0].diffuse = colorTable.White;
    v29[0].specular = Color();
    v29[0].texcoord.x = 1.0;
    v29[0].texcoord.y = 0.0;

    v29[1].pos.x = dstxmod + dstX;
    v29[1].pos.y = dstY - dstymod;
    v29[1].pos.z = dstz;
    v29[1].rhw = dstrhw;
    v29[1].diffuse = colorTable.White;
    v29[1].specular = Color();
    v29[1].texcoord.x = 1.0;
    v29[1].texcoord.y = 1.0;

    v29[2].pos.x = dstX - dstxmod;
    v29[2].pos.y = dstymod + dstY;
    v29[2].pos.z = dstz;
    v29[2].rhw = dstrhw;
    v29[2].diffuse = colorTable.White;
    v29[2].specular = Color();
    v29[2].texcoord.x = 0.0;
    v29[2].texcoord.y = 1.0;

    v29[3].pos.x = srcX - srcxmod;
    v29[3].pos.y = srcymod + srcY;
    v29[3].pos.z = srcz;
    v29[3].rhw = srcrhw;
    v29[3].diffuse = colorTable.White;
    v29[3].specular = Color();
    v29[3].texcoord.x = 0.0;
    v29[3].texcoord.y = 0.0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    int texid = 0;

    if (texture) {
        texid = texture->renderId().value();
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
        thisvert->color = colorTable.White.toColorf();
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            thisvert->x = v29[z + i].pos.x;
            thisvert->y = v29[z + i].pos.y;
            thisvert->z = v29[z + i].pos.z;
            thisvert->w = 1.0f;
            thisvert->u = v29[z + i].texcoord.x;
            thisvert->v = v29[z + i].texcoord.y;
            thisvert->q = v29[z + i].rhw;
            thisvert->screenspace = (z + i == 3) ? srcworldview: dstworldview;
            thisvert->color = colorTable.White.toColorf();
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
    Colorf color;
    GLfloat texid;
    GLfloat paletteid;
};

twodverts twodshaderstore[500] = {};
int twodvertscnt = 0;

void OpenGLRenderer::ScreenFade(Color color, float t) {
    Colorf cf = color.toColorf();
    cf.a = std::clamp(t, 0.0f, 1.0f);

    float drawx = static_cast<float>(pViewport->uViewportTL_X);
    float drawy = static_cast<float>(pViewport->uViewportTL_Y);
    float drawz = static_cast<float>(pViewport->uViewportBR_X);
    float draww = static_cast<float>(pViewport->uViewportBR_Y);

    static GraphicsImage *effpar03 = assets->getBitmap("effpar03");
    float gltexid = static_cast<float>(effpar03->renderId().value());

    // 0 1 2 / 0 2 3

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = 0;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = 0.5f;
    twodshaderstore[twodvertscnt].v = 0.5f;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


void OpenGLRenderer::DrawTextureOffset(int pX, int pY, int move_X, int move_Y,
                                       GraphicsImage *pTexture) {
    DrawTextureNew((float)(pX - move_X)/outputRender.w, (float)(pY - move_Y)/outputRender.h, pTexture);
}


void OpenGLRenderer::DrawImage(GraphicsImage *img, const Recti &rect, unsigned paletteid, Color uColor32) {
    if (!img) {
        logger->trace("Null img passed to DrawImage");
        return;
    }

    Colorf cf = uColor32.toColorf();

    int width = img->width();
    int height = img->height();

    int x = rect.x;
    int y = rect.y;
    int z = rect.x + rect.w;
    int w = rect.y + rect.h;

    // check bounds
    if (x >= outputRender.w || x >= this->clip_z || y >= outputRender.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = static_cast<float>(img->renderId().value());

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
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = paletteid;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

// TODO(pskelton): sort this - forcing the draw is slow
// TODO(pskelton): stencil masking with opacity would be a better way to do this
void OpenGLRenderer::BlendTextures(int x, int y, GraphicsImage *imgin, GraphicsImage *imgblend, int time, int start_opacity,
                                   int end_opacity) {
    // thrown together as a crude estimate of the enchaintg effects
    // leaves gap where it shouldnt on dark pixels currently
    // doesnt use opacity params

    if (imgin && imgblend) {  // 2 images to blend
        const RgbaImage &itemImage = imgin->rgba();
        const RgbaImage &maskImage = imgblend->rgba();

        int w = imgin->width();
        int h = imgin->height();
        GraphicsImage *temp = GraphicsImage::Create(w, h);
        RgbaImage &dstImage = temp->rgba();

        Color c = maskImage.pixels()[2700];  // guess at brightest pixel
        unsigned int rmax = c.r;
        unsigned int gmax = c.g;
        unsigned int bmax = c.b;

        unsigned int bmin = bmax / 10;
        unsigned int gmin = gmax / 10;
        unsigned int rmin = rmax / 10;

        unsigned int bstep = (bmax - bmin) / 128;
        unsigned int gstep = (gmax - gmin) / 128;
        unsigned int rstep = (rmax - rmin) / 128;

        for (int ydraw = 0; ydraw < h; ++ydraw) {
            for (int xdraw = 0; xdraw < w; ++xdraw) {
                // should go blue -> black -> blue reverse
                // patchy -> solid -> patchy

                if (itemImage[ydraw][xdraw] != Color()) {  // check orig item not got blakc pixel
                    Color pixcol = maskImage[ydraw % maskImage.height()][xdraw % maskImage.width()];

                    unsigned int rcur = pixcol.r;
                    unsigned int gcur = pixcol.g;
                    unsigned int bcur = pixcol.b;

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

                    bcur = std::clamp(bcur, bmin, bmax);
                    gcur = std::clamp(gcur, gmin, gmax);
                    rcur = std::clamp(rcur, rmin, rmax);

                    dstImage[ydraw][xdraw] = Color(rcur, gcur, bcur);
                }
            }
        }

        // draw image
        render->Update_Texture(temp);
        render->DrawTextureNew(x / float(outputRender.w), y / float(outputRender.h), temp);

        render->DrawTwodVerts();

        temp->Release();
    }
}

// TODO(pskelton): renderbase
//----- (004A65CC) --------------------------------------------------------
//_4A65CC(unsigned int x, unsigned int y, Texture_MM7 *a4, Texture_MM7 *a5, int a6, int a7, int a8)
// a6 is time, a7 is 0, a8 is 63
void OpenGLRenderer::TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) {
    // TODO(pskelton): sort this - precalculate/ shader
    static std::array<GraphicsImage *, 14> cachedtemp {};
    static std::array<int, 14> cachetime { -1 };

    if (img) {
        std::string_view tempstr{ img->GetName() };
        int number = tempstr[4] - 48;
        int number2 = tempstr[5] - 48;

        int thisslot = 10 * number + number2 - 1;
        if (cachetime[thisslot] != time) {
            int width = img->width();
            int height = img->height();
            if (!cachedtemp[thisslot]) {
                cachedtemp[thisslot] = GraphicsImage::Create(width, height);
            }

            const Palette &palette = img->palette();
            auto temppix = cachedtemp[thisslot]->rgba().pixels();
            auto texpix24 = img->indexed().pixels();

            for (size_t i = 0, size = temppix.size(); i < size; i++) {
                int index = texpix24[i];
                if (index >= 0 && index <= 63) {
                    index = (time + index) % (2 * 63);
                    if (index >= 63)
                        index = (2 * 63) - index;
                    temppix[i] = palette.colors[index];
                }
            }

            cachetime[thisslot] = time;
            render->Update_Texture(cachedtemp[thisslot]);
        }

        render->DrawTextureNew(u, v, cachedtemp[thisslot]);
    }
}

// TODO(pskelton): renderbase
void OpenGLRenderer::DrawIndoorSky(unsigned int uNumVertices, int uFaceID) {
    BLVFace *pFace = &pIndoor->pFaces[uFaceID];
    if (pFace->uNumVertices <= 0) return;

    Polygon pSkyPolygon;
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.texture = pFace->GetTexture();
    if (!pSkyPolygon.texture) return;

    pSkyPolygon.ptr_38 = &SkyBillboard;
    pSkyPolygon.dimming_level = 0;
    pSkyPolygon.uNumVertices = pFace->uNumVertices;


    // TODO(pskelton): repeated maths could be saved when calculating sky planes
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  blv_horizon_height_offset = ((pCamera3D->ViewPlaneDistPixels * pCamera3D->vCameraPos.z)
        / (pCamera3D->ViewPlaneDistPixels + pCamera3D->GetFarClip())
        + (pBLVRenderParams->uViewportCenterY));

    double cam_y_rot_rad = (double)pCamera3D->_viewPitch * rot_to_rads;

    float depth_to_far_clip = std::cos(pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = std::sin(pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();

    float blv_bottom_y_proj = ((pBLVRenderParams->uViewportCenterY) -
        pCamera3D->ViewPlaneDistPixels /
        (depth_to_far_clip + 0.0000001f) *
        (height_to_far_clip - pCamera3D->vCameraPos.z));

    // rotation vec for sky plane - pitch
    float v_18x = -std::sin((-pCamera3D->_viewPitch + 16) * rot_to_rads);
    float v_18y = 0.0f;
    float v_18z = -std::cos((pCamera3D->_viewPitch + 16) * rot_to_rads);

    float inv_viewplanedist = 1.0f / pCamera3D->ViewPlaneDistPixels;

    // copy to buff in
    for (unsigned i = 0; i < pFace->uNumVertices; ++i) {
        array_507D30[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
        array_507D30[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
        array_507D30[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
        array_507D30[i].u = (signed short)pFace->pVertexUIDs[i];
        array_507D30[i].v = (signed short)pFace->pVertexVIDs[i];
    }

    // clip accurately to camera
    pCamera3D->ClipFaceToFrustum(array_507D30, &pSkyPolygon.uNumVertices, VertexRenderList, pBspRenderer->nodes[0].ViewportNodeFrustum.data(), 4, 0, 0);
    if (!pSkyPolygon.uNumVertices) return;

    pCamera3D->ViewTransform(VertexRenderList, pSkyPolygon.uNumVertices);
    pCamera3D->Project(VertexRenderList, pSkyPolygon.uNumVertices, false);

    unsigned _507D30_idx = 0;
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
        float texoffset_U = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalleft * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].u = texoffset_U / (pSkyPolygon.texture->width());
        float texoffset_V = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalfront * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].v = texoffset_V / (pSkyPolygon.texture->height());

        // this basically acts as texture perspective correction
        VertexRenderList[_507D30_idx]._rhw = worldviewdepth;
    }

    // no clipped polygon so draw and return??
    if (_507D30_idx >= pSkyPolygon.uNumVertices) {
        DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon);
        return;
    }
}

void OpenGLRenderer::DrawIndoorSkyPolygon(signed int uNumVertices, Polygon *pSkyPolygon) {
    int texid = pSkyPolygon->texture->renderId().value();

    Colorf uTint = GetActorTintColor(pSkyPolygon->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0).toColorf();
    float scrspace{ pCamera3D->GetFarClip() };

    float oneon = 1.0f / (pCamera3D->GetNearClip() * 2.0f);
    float oneof = 1.0f / (pCamera3D->GetFarClip());

    // load up poly
    for (int z = 0; z < (pSkyPolygon->uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        forcepersverts *thisvert = &forceperstore[forceperstorecnt];
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
        thisvert->color = uTint;
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
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
            thisvert->color = uTint;
            thisvert->texid = texid;
            thisvert++;
        }

        forceperstorecnt += 3;
        // TODO (pskelton): should force drawing if buffer is full
        assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
    }
}

bool OpenGLRenderer::AreRenderSurfacesOk() {
    logger->info("RenderGl - AreRenderSurfacesOk");
    return true;
}

RgbaImage OpenGLRenderer::MakeScreenshot32(const int width, const int height) {
    // TODO(pskelton): should this call drawworld instead??

    pCamera3D->_viewPitch = pParty->_viewPitch;
    pCamera3D->_viewYaw = pParty->_viewYaw;
    pCamera3D->vCameraPos.x = pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.y = pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.z = pParty->pos.z + pParty->eyeLevel;  // 193, but real 353
    pCamera3D->CalculateRotations(pParty->_viewYaw, pParty->_viewPitch);
    pCamera3D->CreateViewMatrixAndProjectionScale();
    pCamera3D->BuildViewFrustum();

    BeginScene3D();
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        render->uFogColor = GetLevelFogColor();
        pOutdoor->Draw();
    }
    DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();

    // TODO(captainurist): subImage().scale()
    RgbaImage sPixels = ReadScreenPixels();
    float interval_x = static_cast<float>(game_viewport_width) / width;
    float interval_y = static_cast<float>(game_viewport_height) / height;

    RgbaImage pPixels = RgbaImage::solid(width, height, Color());

    if (uCurrentlyLoadedLevelType != LEVEL_NULL) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                pPixels[y][x] = sPixels[outputRender.h - (y + 1) * interval_y - pViewport->uViewportTL_Y][x * interval_x + pViewport->uViewportTL_X];
            }
        }
    }

    return pPixels;
}

// TODO(pskelton): drop - not required in gl renderer now
void OpenGLRenderer::BeginLightmaps() { return; }
void OpenGLRenderer::EndLightmaps() { return; }
void OpenGLRenderer::BeginLightmaps2() { return; }
void OpenGLRenderer::EndLightmaps2() { return; }
bool OpenGLRenderer::DrawLightmap(Lightmap *pLightmap, Vec3f *pColorMult, float z_bias) {
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


void OpenGLRenderer::BeginDecals() {
    GraphicsImage *texture = assets->getBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->renderId().value());

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

void OpenGLRenderer::EndDecals() {
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
    glUniform3f(glGetUniformLocation(decalshader.ID, "fog.color"), fog.r, fog.g, fog.b);
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(decalshader.ID, "fog.fogend"), GLfloat(fogend));

    // set bias
    glUniform1f(glGetUniformLocation(decalshader.ID, "decalbias"), GLfloat(0.002f));

    // set texture unit location
    glUniform1i(glGetUniformLocation(decalshader.ID, "texture0"), GLint(0));
    glActiveTexture(GL_TEXTURE0);

    GraphicsImage *texture = assets->getBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->renderId().value());

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
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}



void OpenGLRenderer::DrawDecal(Decal *pDecal, float z_bias) {
    if (pDecal->uNumVertices < 3) {
        logger->warning("Decal has < 3 vertices");
        return;
    }

    float color_mult = pDecal->Fade_by_time();
    if (color_mult == 0.0f) return;

    // temp - bloodsplat persistance
    // color_mult = 1;

    // load into buffer
    Colorf decalColorMult = pDecal->uColorMultiplier.toColorf();

    for (int z = 0; z < (pDecal->uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        GLdecalverts *thisvert = &decalshaderstore[numdecalverts];
        Colorf uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[0].vWorldViewPosition.x, 0, nullptr).toColorf();

        float uFinalR = uTint.r * color_mult * decalColorMult.r;
        float uFinalG = uTint.g * color_mult * decalColorMult.g;
        float uFinalB = uTint.b * color_mult * decalColorMult.b;

        // copy first
        thisvert->x = pDecal->pVertices[0].vWorldPosition.x;
        thisvert->y = pDecal->pVertices[0].vWorldPosition.y;
        thisvert->z = pDecal->pVertices[0].vWorldPosition.z;
        thisvert->u = pDecal->pVertices[0].u;
        thisvert->v = pDecal->pVertices[0].v;
        thisvert->texunit = 0;
        thisvert->red = uFinalR;
        thisvert->green = uFinalG;
        thisvert->blue = uFinalB;
        thisvert->attribs = 0;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[z + i].vWorldViewPosition.x, 0, nullptr).toColorf();
            uFinalR = uTint.r * color_mult * decalColorMult.r;
            uFinalG = uTint.g * color_mult * decalColorMult.g;
            uFinalB = uTint.b * color_mult * decalColorMult.b;

            thisvert->x = pDecal->pVertices[z + i].vWorldPosition.x;
            thisvert->y = pDecal->pVertices[z + i].vWorldPosition.y;
            thisvert->z = pDecal->pVertices[z + i].vWorldPosition.z;
            thisvert->u = pDecal->pVertices[z + i].u;
            thisvert->v = pDecal->pVertices[z + i].v;
            thisvert->texunit = 0;
            thisvert->red = uFinalR;
            thisvert->green = uFinalG;
            thisvert->blue = uFinalB;
            thisvert->attribs = 0;
            thisvert++;
        }

        numdecalverts += 3;
        assert(numdecalverts <= 9999);
    }
}

void OpenGLRenderer::DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3, int blend_mode) {
    // want to draw psrcrect section @ point

    GraphicsImage *texture = pArcomageGame->pSprites;

    if (!texture) {
        logger->trace("Missing Arcomage Sprite Sheet");
        return;
    }

    float col = (blend_mode == 2) ? 1.0f : 0.5f;
    Colorf cf = Colorf(col, col, col);

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
    if (x >= outputRender.w || x >= this->clip_z || y >= outputRender.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = static_cast<float>(texture->renderId().value());
    int texwidth = texture->width();
    int texheight = texture->height();

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
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

void OpenGLRenderer::Update_Texture(GraphicsImage *texture) {
    UpdateTexture(texture->renderId(), texture->rgba());
}

TextureRenderId OpenGLRenderer::CreateTexture(RgbaImageView image) {
    assert(image);

    GLuint glId;
    glGenTextures(1, &glId);
    glBindTexture(GL_TEXTURE_2D, glId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels().data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    return TextureRenderId(glId);
}

void OpenGLRenderer::DeleteTexture(TextureRenderId id) {
    if (!id)
        return;

    GLuint glId = id.value();
    glDeleteTextures(1, &glId);
}

void OpenGLRenderer::UpdateTexture(TextureRenderId id, RgbaImageView image) {
    assert(image);
    assert(id);

    glBindTexture(GL_TEXTURE_2D, id.value());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels().data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TODO(pskelton): to camera?
void OpenGLRenderer::_set_3d_projection_matrix() {
    float near_clip = pCamera3D->GetNearClip();
    float far_clip = pCamera3D->GetFarClip();

    // build projection matrix with glm
    projmat = glm::perspective(glm::radians(pCamera3D->fov_y_deg), pCamera3D->aspect, near_clip, far_clip);
}

// TODO(pskelton): to camera?
void OpenGLRenderer::_set_3d_modelview_matrix() {
    float camera_x = pCamera3D->vCameraPos.x;
    float camera_y = pCamera3D->vCameraPos.y;
    float camera_z = pCamera3D->vCameraPos.z;

    // build view matrix with glm
    glm::vec3 campos = glm::vec3(camera_x, camera_y, camera_z);
    glm::vec3 eyepos = glm::vec3(camera_x - cosf(2.0f * pi_double * pCamera3D->_viewYaw / 2048.0f),
        camera_y - sinf(2.0f * pi_double * pCamera3D->_viewYaw / 2048.0f),
        camera_z - tanf(2.0f * pi_double * -pCamera3D->_viewPitch / 2048.0f));
    glm::vec3 upvec = glm::vec3(0.0f, 0.0f, 1.0f);

    viewmat = glm::lookAtLH(campos, eyepos, upvec);
}

// TODO(pskelton): to camera?
void OpenGLRenderer::_set_ortho_projection(bool gameviewport) {
    if (!gameviewport) {  // project over entire window
        glViewport(0, 0, outputRender.w, outputRender.h);
        projmat = glm::ortho(float(0), float(outputRender.w), float(outputRender.h), float(0), float(-1), float(1));
    } else {  // project to game viewport
        glViewport(game_viewport_x, outputRender.h-game_viewport_w-1, game_viewport_width, game_viewport_height);
        projmat = glm::ortho(float(game_viewport_x), float(game_viewport_z), float(game_viewport_w), float(game_viewport_y), float(1), float(-1));
    }
}

// TODO(pskelton): to camera?
void OpenGLRenderer::_set_ortho_modelview() {
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

void OpenGLRenderer::DrawOutdoorTerrain() {
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
        terraintexturesizes[0] = wtrtexture->width();

        for (int buff = 0; buff < 7; buff++) {
            std::string container_name = fmt::format("HDWTR{:03}", buff);
            terraintexmap.insert(std::make_pair(container_name, terraintexmap.size()));
            numterraintexloaded[0]++;
        }

        for (int y = 0; y < 127; ++y) {
            for (int x = 0; x < 127; ++x) {
                // map is 127 x 127 squares - each square has two triangles - each tri has 3 verts

                // first find all required textures for terrain and add to map
                auto tile = pOutdoor->getTileDescByGrid(x, y);
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
                    auto thistexture = assets->getBitmap(tile->name);
                    int width = thistexture->width();
                    // check size to see what unit it needs
                    int i;
                    for (i = 0; i < 8; i++) {
                        if (terraintexturesizes[i] == width || terraintexturesizes[i] == 0) break;
                    }

                    if (i == 8) {
                        logger->warning("Texture unit full - draw terrain!");
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
                            logger->warning("Texture layer full - draw terrain!");
                            tileunit = 0;
                            tilelayer = 0;
                        }
                    }
                }

                // next calculate all vertices vertices
                unsigned norm_idx = pTerrainNormalIndices[(2 * x * 128) + (2 * y) + 2 /*+ 1*/];  // 2 is top tri // 3 is bottom
                unsigned bottnormidx = pTerrainNormalIndices[(2 * x * 128) + (2 * y) + 3];
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
                    auto texture = assets->getBitmap(it->first);
                    // send texture data to gpu
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0, 0, tlayer,
                        terraintexturesizes[unit], terraintexturesizes[unit], 1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        texture->rgba().pixels().data());
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
    if (engine->config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
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
    glUniform3f(glGetUniformLocation(terrainshader.ID, "fog.color"), fog.r, fog.g, fog.b);
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(terrainshader.ID, "fog.fogend"), GLfloat(fogend));

    GLfloat camera[3] {};
    camera[0] = (float)(pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0));
    camera[1] = (float)(pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0));
    camera[2] = (float)(pParty->pos.z + pParty->eyeLevel);
    glUniform3fv(glGetUniformLocation(terrainshader.ID, "CameraPos"), 1, &camera[0]);


    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0) * 2 * pi_double) / 1440) + 1) * 0.27;
    float diffuseon = pWeather->bNight ? 0 : 1;

    glUniform3fv(glGetUniformLocation(terrainshader.ID, "sun.direction"), 1, &pOutdoor->vSunlight.x);
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.ambient"), ambient, ambient, ambient);
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.diffuse"), diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3));
    glUniform3f(glGetUniformLocation(terrainshader.ID, "sun.specular"), /*diffuseon * 0.35f * ambient*/ 0.0f, /*diffuseon * 0.28f * ambient*/ 0.0f, 0.0f);

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

        MobileLight &test = pMobileLightsStack->pLights[i];
        std::string slotnum = std::to_string(num_lights);

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        StationaryLight &test = pStationaryLightsStack->pLights[i];

        float x = test.vPosition.x;
        float y = test.vPosition.y;
        float z = test.vPosition.z;

        Colorf color = test.uLightColor.toColorf();

        float lightrad = test.uRadius;

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        MobileLight &test = pMobileLightsStack->pLights[i];

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

        float lightrad = pMobileLightsStack->pLights[i].uRadius;

        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(terrainshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

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
    glBindTexture(GL_TEXTURE_2D, 0);

    //end terrain debug
    if (engine->config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // stack new decals onto terrain faces ////////////////////////////////////////////////
    // TODO(pskelton): clean up and move to seperate function in decal builder
    if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;
    unsigned int NumBloodsplats = decal_builder->bloodsplat_container->uNumBloodsplats;

    // loop over blood to lay
    for (unsigned i = 0; i < NumBloodsplats; ++i) {
        // approx location of bloodsplat
        int splatx = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos.x;
        int splaty = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos.y;
        int splatz = decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos.z;
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

                // bottom tri
                // x, y
                VertexRenderList[3].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].x;
                VertexRenderList[3].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].y;
                VertexRenderList[3].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 3].z;
                // x, y + 1
                VertexRenderList[4].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].x;
                VertexRenderList[4].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].y;
                VertexRenderList[4].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 4].z;
                // x + 1, y + 1
                VertexRenderList[5].vWorldPosition.x = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].x;
                VertexRenderList[5].vWorldPosition.y = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].y;
                VertexRenderList[5].vWorldPosition.z = terrshaderstore[6 * (loopx + (127 * loopy)) + 5].z;

                float WorldMinZ = pOutdoor->GetPolygonMinZ(VertexRenderList, 6);
                float WorldMaxZ = pOutdoor->GetPolygonMaxZ(VertexRenderList, 6);

                // TODO(pskelton): terrain and boxes should be saved for easier retrieval
                // test expanded box against bloodsplat
                BBoxf thissquare{ terrshaderstore[6 * (loopx + (127 * loopy))].x ,
                                  terrshaderstore[6 * (loopx + (127 * loopy)) + 1].x,
                                  terrshaderstore[6 * (loopx + (127 * loopy)) + 1].y,
                                  terrshaderstore[6 * (loopx + (127 * loopy))].y,
                                  WorldMinZ,
                                  WorldMaxZ };

                // skip this square if no splat over lap
                if (!thissquare.intersectsCube(decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos, decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].radius))
                    continue;

                // splat hits this square of terrain
                Polygon *pTilePolygon = &array_77EC08[pODMRenderParams->uNumPolygons];
                pTilePolygon->flags = pOutdoor->getTileAttribByGrid(loopx, loopy);

                unsigned norm_idx = pTerrainNormalIndices[(2 * loopx * 128) + (2 * loopy) + 2];  // 2 is top tri // 3 is bottom
                unsigned bottnormidx = pTerrainNormalIndices[(2 * loopx * 128) + (2 * loopy) + 3];
                assert(norm_idx < pTerrainNormals.size());
                assert(bottnormidx < pTerrainNormals.size());
                Vec3f *norm = &pTerrainNormals[norm_idx];
                Vec3f *norm2 = &pTerrainNormals[bottnormidx];

                float Light_tile_dist = 0.0;

                // top tri
                float _f1 = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0f - floorf(20.0f * _f1 + 0.5f);
                pTilePolygon->dimming_level = std::clamp((int)pTilePolygon->dimming_level, 0, 31);

                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon->flags, norm, &Light_tile_dist, VertexRenderList, i);
                Planef plane;
                plane.normal = *norm;
                plane.dist = Light_tile_dist;
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level, LocationTerrain, plane, 3, VertexRenderList, 0, -1);

                //bottom tri
                float _f = norm2->x * pOutdoor->vSunlight.x + norm2->y * pOutdoor->vSunlight.y + norm2->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);
                pTilePolygon->dimming_level = std::clamp((int)pTilePolygon->dimming_level, 0, 31);

                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon->flags, norm2, &Light_tile_dist, (VertexRenderList + 3), i);
                plane.normal = *norm2;
                plane.dist = Light_tile_dist;
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level, LocationTerrain, plane, 3, (VertexRenderList + 3), 0, -1);
            }
        }
    }

    // end of new system test
    return;

    // end shder version
}

// TODO(pskelton): drop - this is now obselete with shader terrain drawing
void OpenGLRenderer::DrawTerrainPolygon(Polygon *poly, bool transparent, bool clampAtTextureBorders) { return; }

// TODO(pskelton): renderbase
void OpenGLRenderer::DrawOutdoorSky() {
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  horizon_height_offset = ((double)(pCamera3D->ViewPlaneDistPixels * pCamera3D->vCameraPos.z)
        / ((double)pCamera3D->ViewPlaneDistPixels + pCamera3D->GetFarClip())
        + (double)(pViewport->uScreenCenterY));

    float depth_to_far_clip = std::cos((double)pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = std::sin((double)pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();

    float bot_y_proj = ((double)(pViewport->uScreenCenterY) -
        (double)pCamera3D->ViewPlaneDistPixels /
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
        pOutdoor->sky_texture = assets->getBitmap("plansky3");

    pSkyPolygon.texture = pOutdoor->sky_texture;
    if (pSkyPolygon.texture) {
        pSkyPolygon.dimming_level = (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)? 31 : 0;
        pSkyPolygon.uNumVertices = 4;

        // centering(центруем)-----------------------------------------------------------------
        // plane of sky polygon rotation vector - pitch rotation around y
        float v18x = -std::sin((-pCamera3D->_viewPitch + 16) * rot_to_rads);
        float v18y = 0;
        float v18z = -std::cos((pCamera3D->_viewPitch + 16) * rot_to_rads);

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

        float widthperpixel = 1 / pCamera3D->ViewPlaneDistPixels;

        for (unsigned i = 0; i < pSkyPolygon.uNumVertices; ++i) {
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
            float texoffset_U = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalleft * worldviewdepth));
            VertexRenderList[i].u = texoffset_U / ((float) pSkyPolygon.texture->width());
            float texoffset_V = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalfront * worldviewdepth));
            VertexRenderList[i].v = texoffset_V / ((float) pSkyPolygon.texture->height());

            VertexRenderList[i].vWorldViewPosition.x = pCamera3D->GetFarClip();

            // this basically acts as texture perspective correction
            VertexRenderList[i]._rhw = (double)(worldviewdepth);
        }

        if (engine->config->graphics.Fog.value()) {
            // fade sky
            VertexRenderList[4].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[4].vWorldViewProjY = (double)pViewport->uViewportTL_Y;
            VertexRenderList[5].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[5].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.value();
            VertexRenderList[6].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[6].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.value();
            VertexRenderList[7].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[7].vWorldViewProjY = (double)pViewport->uViewportTL_Y;

            // sub sky
            VertexRenderList[8].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[8].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.value();
            VertexRenderList[9].vWorldViewProjX = (double)pViewport->uViewportTL_X;
            VertexRenderList[9].vWorldViewProjY = (double)pViewport->uViewportBR_Y + 1;
            VertexRenderList[10].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[10].vWorldViewProjY = (double)pViewport->uViewportBR_Y + 1;
            VertexRenderList[11].vWorldViewProjX = (double)pViewport->uViewportBR_X;
            VertexRenderList[11].vWorldViewProjY = (double)bot_y_proj - engine->config->graphics.FogHorizon.value();
        }

        _set_ortho_projection(1);
        _set_ortho_modelview();
        DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}



//----- (004A2DA3) --------------------------------------------------------
void OpenGLRenderer::DrawOutdoorSkyPolygon(Polygon *pSkyPolygon) {
    auto texture = pSkyPolygon->texture;
    auto texid = texture->renderId().value();

    static GraphicsImage *effpar03 = assets->getBitmap("effpar03");
    float texidsolid = static_cast<float>(effpar03->renderId().value());

    //glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    Colorf uTint = GetActorTintColor(pSkyPolygon->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0).toColorf();
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
        thisvert->color = uTint;
        thisvert->texid = texid;
        thisvert++;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
            thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
            thisvert->z = 1.0f;
            thisvert->w = VertexRenderList[z + i]._rhw;
            thisvert->u = VertexRenderList[z + i].u;
            thisvert->v = VertexRenderList[z + i].v;
            thisvert->q = 1.0f;
            thisvert->screenspace = scrspace;
            thisvert->color = uTint;
            thisvert->texid = texid;
            thisvert++;
        }

        forceperstorecnt += 3;
        assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
    }

    if (engine->config->graphics.Fog.value()) {
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
            thisvert->color = uTint;
            thisvert->color.a = 0;
            thisvert->texid = texidsolid;
            thisvert++;

            // copy other two (z+1)(z+2)
            for (unsigned i = 1; i < 3; ++i) {
                thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
                thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
                thisvert->z = 1.0f;
                thisvert->w = 1.0f;
                thisvert->u = 0.5f;
                thisvert->v = 0.5f;
                thisvert->q = 1.0f;
                thisvert->screenspace = scrspace;
                thisvert->color = uTint;
                thisvert->color.a = ((z + i) == 7) ? 0.0f : 1.0f;
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
            thisvert->color = uTint;
            thisvert->texid = texidsolid;
            thisvert++;

            // copy other two (z+1)(z+2)
            for (unsigned i = 1; i < 3; ++i) {
                thisvert->x = VertexRenderList[z + i].vWorldViewProjX;
                thisvert->y = VertexRenderList[z + i].vWorldViewProjY;
                thisvert->z = 1.0f;
                thisvert->w = 1.0f;
                thisvert->u = 0.5f;
                thisvert->v = 0.5f;
                thisvert->q = 1.0f;
                thisvert->screenspace = scrspace;
                thisvert->color = uTint;
                thisvert->texid = texidsolid;
                thisvert++;
            }

            forceperstorecnt += 3;
            assert(forceperstorecnt <= MAX_FORCEPERSTORECNT);
        }
    }

    DrawForcePerVerts();
}

void OpenGLRenderer::DrawForcePerVerts() {
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
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(forcepersverts), (void *)offsetof(forcepersverts, color));
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
    Color fpfogcol = GetLevelFogColor();

    if (engine->config->graphics.Fog.value() && uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        if (fpfogcol != Color()) {
            fpfogstart = day_fogrange_1;
            fpfogmiddle = day_fogrange_2;
            fpfogend = pCamera3D->GetFarClip();
            fpfogr = fpfogg = fpfogb = GetLevelFogColor().r / 255.0f;
        } else {
            fpfogstart = pCamera3D->GetFarClip();
            fpfogmiddle = 0.0f;
            fpfogend = fpfogstart + 1;
            fpfogr = fpfogg = fpfogb = forceperstore[0].color.r;
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

// TODO(pskelton): move ?
void OpenGLRenderer::SetFogParametersGL() {
    Color fogcol = GetLevelFogColor();

    if (engine->config->graphics.Fog.value() && uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        if (fogcol != Color()) {
            fogstart = day_fogrange_1;
            fogmiddle = day_fogrange_2;
            fogend = pCamera3D->GetFarClip();
            fog.r = fog.g = fog.b = GetLevelFogColor().r / 255.0f;
        } else {
            fogend = pCamera3D->GetFarClip();
            fogmiddle = 0.0f;
            fogstart = fogend * engine->config->graphics.FogDepthRatio.value();

            // grabs sky back fog colour
            Color uTint = GetActorTintColor(31, 0, fogend, 1, 0);
            fog.r = fog.g = fog.b = uTint.r / 255.0f;
        }
    } else {
        // puts fog beyond viewclip so we never see it
        fogstart = pCamera3D->GetFarClip();
        fogmiddle = 0.0f;
        fogend = pCamera3D->GetFarClip();
    }
}

struct billbverts {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat u;
    GLfloat v;
    Colorf color;
    GLfloat screenspace;
    GLfloat texid;
    GLfloat blend;
    GLfloat paletteindex;
};

billbverts billbstore[1000] {};
int billbstorecnt{ 0 };

//----- (004A1C1E) --------------------------------------------------------
void OpenGLRenderer::DoRenderBillboards_D3D() {
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);  // in theory billboards all sorted by depth so dont cull by depth test
    glDisable(GL_CULL_FACE);  // some quads are reversed to reuse sprites opposite hand

    _set_ortho_projection(1);
    _set_ortho_modelview();

    if (billbstorecnt)
        logger->trace("Billboard shader store isnt empty!");

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

        if (pBillboardRenderListD3D[i].texture) {
            auto texture = pBillboardRenderListD3D[i].texture;
            gltexid = texture->renderId().value();
        } else {
            static GraphicsImage *effpar03 = assets->getBitmap("effpar03");
            gltexid = static_cast<float>(effpar03->renderId().value());
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
        billbstore[billbstorecnt].color = billboard->pQuads[0].diffuse.toColorf();
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
        billbstore[billbstorecnt].color = billboard->pQuads[1].diffuse.toColorf();
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
        billbstore[billbstorecnt].color = billboard->pQuads[2].diffuse.toColorf();
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
            billbstore[billbstorecnt].color = billboard->pQuads[0].diffuse.toColorf();
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
            billbstore[billbstorecnt].color = billboard->pQuads[2].diffuse.toColorf();
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
            billbstore[billbstorecnt].color = billboard->pQuads[3].diffuse.toColorf();
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

    //glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

// name better
void OpenGLRenderer::DrawBillboards() {
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
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(billbverts), (void *)offsetof(billbverts, color));
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
        std::span<Color> palettes = pPaletteManager->paletteData();
        glGenBuffers(1, &palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, palettes.size_bytes(), palettes.data(), GL_STATIC_DRAW);

        glGenTextures(1, &paltex);
        glBindTexture(GL_TEXTURE_BUFFER, paltex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, billbVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(billbverts) * billbstorecnt, billbstore);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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


    // set sampler to texure0
    glUniform1i(glGetUniformLocation(billbshader.ID, "texture0"), GLint(0));

    //// set projection
    glUniformMatrix4fv(glGetUniformLocation(billbshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    //// set view
    glUniformMatrix4fv(glGetUniformLocation(billbshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    glUniform1f(glGetUniformLocation(billbshader.ID, "gamma"), gamma);

    // set fog uniforms
    glUniform3f(glGetUniformLocation(billbshader.ID, "fog.color"), fog.r, fog.g, fog.b);
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(billbshader.ID, "fog.fogend"), GLfloat(fogend));

    int offset = 0;
    while (offset < billbstorecnt) {
        // set texture
        GLfloat thistex = billbstore[offset].texid;
        glBindTexture(GL_TEXTURE_2D, billbstore[offset].texid);
        if (billbstore[offset].paletteindex) {
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
void OpenGLRenderer::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1) {
    return;
}

void OpenGLRenderer::SetUIClipRect(unsigned int x, unsigned int y, unsigned int z,
                                   unsigned int w) {
    this->clip_x = x;
    this->clip_y = y;
    this->clip_z = z;
    this->clip_w = w;
    glScissor(x, outputRender.h -w, z-x, w-y);  // invert glscissor co-ords 0,0 is BL
}

void OpenGLRenderer::ResetUIClipRect() {
    this->SetUIClipRect(0, 0, outputRender.w, outputRender.h);
}

void OpenGLRenderer::BeginScene2D() {
    // Setup for 2D

    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextures[0], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufferTextures[1], 0);

        GL_Check_Framebuffer(__FUNCTION__);
    }

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    _set_ortho_projection();
    _set_ortho_modelview();
}

// TODO(pskelton): use alpha from mask too
void OpenGLRenderer::DrawTextureNew(float u, float v, GraphicsImage *tex, Color colourmask) {
    assert(tex);

    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, tex->GetName());

    Colorf cf = colourmask.toColorf();

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = tex->width();
    int height = tex->height();

    int x = u * outputRender.w;
    int y = v * outputRender.h;
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= outputRender.w || x >= this->clip_z || y >= outputRender.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = tex->renderId().value();

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
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

// TODO(pskelton): add optional colour32
void OpenGLRenderer::DrawTextureCustomHeight(float u, float v, GraphicsImage *img, int custom_height) {
    assert(img);

    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, img->GetName());

    Colorf cf(1.0f, 1.0f, 1.0f);

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = img->width();
    int height = img->height();

    int x = u * outputRender.w;
    int y = v * outputRender.h + 0.5;
    int z = x + width;
    int w = y + custom_height;

    // check bounds
    if (x >= outputRender.w || x >= this->clip_z || y >= outputRender.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    float gltexid = img->renderId().value();

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
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}


twodverts textshaderstore[10000] = {};
int textvertscnt = 0;

void OpenGLRenderer::BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) {
    // draw any images in buffer
    if (twodvertscnt) {
        DrawTwodVerts();
    }

    GLuint texmainidcheck = main->renderId().value();

    // if we are changing font draw whats in the text buffer
    if (texmainidcheck != texmain) {
        EndTextNew();
    }

    texmain = main->renderId().value();

    texshadow = shadow->renderId().value();

    // set up buffers
    // set up counts
    // set up textures

    return;
}

void OpenGLRenderer::EndTextNew() {
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
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void *)offsetof(twodverts, color));
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

void OpenGLRenderer::DrawTextNew(int x, int y, int width, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) {
    Colorf cf = colour.toColorf();
    // not 100% sure why this is required but it is
    if (cf.r == 0.0f)
        cf.r = 0.00392f;

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int z = x + width;
    int w = y + h;
    // check bounds
    if (x >= outputRender.w || x >= clipz || y >= outputRender.h || y >= clipw) return;
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
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = drawy;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texy;
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    ////////////////////////////////
    textshaderstore[textvertscnt].x = drawx;
    textshaderstore[textvertscnt].y = drawy;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texx;
    textshaderstore[textvertscnt].v = texy;
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawz;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texz;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    textshaderstore[textvertscnt].x = drawx;
    textshaderstore[textvertscnt].y = draww;
    textshaderstore[textvertscnt].z = 0;
    textshaderstore[textvertscnt].u = texx;
    textshaderstore[textvertscnt].v = texw;
    textshaderstore[textvertscnt].color = cf;
    textshaderstore[textvertscnt].texid = (isshadow);
    textshaderstore[textvertscnt].paletteid = 0;
    textvertscnt++;

    if (textvertscnt > 9990) EndTextNew();
}

void OpenGLRenderer::flushAndScale() {
    // flush any undrawn items
    DrawTwodVerts();
    EndLines2D();
    EndTextNew();

    if (outputRender != outputPresent) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, outputPresent.w, outputPresent.h);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float ratio_width = (float)outputPresent.w / outputRender.w;
        float ratio_height = (float)outputPresent.h / outputRender.h;
        float ratio = std::min(ratio_width, ratio_height);

        float w = outputRender.w * ratio;
        float h = outputRender.h * ratio;

        Recti rect;
        rect.x = (float)outputPresent.w / 2 - w / 2;
        rect.y = (float)outputPresent.h / 2 - h / 2;
        rect.w = w;
        rect.h = h;

        GLenum filter = config->graphics.RenderFilter.value() == 1 ? GL_LINEAR : GL_NEAREST;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBlitFramebuffer(0, 0, outputRender.w, outputRender.h, rect.x, rect.y, rect.w + rect.x, rect.h + rect.y, GL_COLOR_BUFFER_BIT, filter);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
}

void OpenGLRenderer::swapBuffers() {
    if (outputRender != outputPresent) {
        glEnable(GL_SCISSOR_TEST);
        glViewport(0, 0, outputRender.w, outputRender.h);
    }

    openGLContext->swapBuffers();

    if (engine->config->graphics.FPSLimit.value() > 0)
        _frameLimiter.tick(engine->config->graphics.FPSLimit.value());
}

void OpenGLRenderer::Present() {
    flushAndScale();
    swapBuffers();
}

GLshaderverts *outbuildshaderstore[16] = { nullptr };
int numoutbuildverts[16] = { 0 };

void OpenGLRenderer::DrawOutdoorBuildings() {
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

            //for (BSPModel &model : pOutdoor->pBModels) {
            //    //int reachable;
            //    //if (IsBModelVisible(&model, &reachable)) {
            //    //model.field_40 |= 1;
            //    if (!model.pFaces.empty()) {
            //        for (ODMFace &face : model.pFaces) {
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
        outbuildtexturewidths[0] = wtrtexture->width();
        outbuildtextureheights[0] = wtrtexture->height();

        for (int buff = 0; buff < 7; buff++) {
            std::string container_name = fmt::format("HDWTR{:03}", buff);
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
                        // TODO(pskelton): Same as indoors. When ODM and BLV face is combined - seperate out function

                        if (!face.GetTexture()) continue;
                        GraphicsImage *tex = face.GetTexture();

                        std::string texname = tex->GetName();

                        Duration animLength;
                        Duration frame;
                        if (face.IsTextureFrameTable()) {
                            tex = pTextureFrameTable->GetFrameTexture((int64_t)face.resource, frame);
                            animLength = pTextureFrameTable->textureFrameAnimLength((int64_t)face.resource);
                            texname = tex->GetName();
                        }
                        // gather up all texture and shaderverts data

                        //auto tile = pOutdoor->getTileDescByGrid(x, y);

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

                        if (face.uAttributes & FACE_IsLava)
                            attribflags |= 0x4000;

                        // loop while running down animlength with frame animtimes
                        do {
                            // check if tile->name is already in list
                            auto mapiter = outbuildtexmap.find(texname);
                            if (mapiter != outbuildtexmap.end()) {
                                // if so, extract unit and layer
                                int unitlayer = mapiter->second;
                                texlayer = unitlayer & 0xFF;
                                texunit = (unitlayer & 0xFF00) >> 8;
                            } else if (texname == "wtrtyl") {
                                // water tile
                                texunit = 0;
                                texlayer = 0;
                            } else {
                                // else need to add it
                                auto thistexture = assets->getBitmap(texname);
                                int width = thistexture->width();
                                int height = thistexture->height();
                                // check size to see what unit it needs
                                int i;
                                for (i = 0; i < 16; i++) {
                                    if ((outbuildtexturewidths[i] == width && outbuildtextureheights[i] == height) || outbuildtexturewidths[i] == 0) break;
                                }

                                if (i == 16) {
                                    logger->warning("Texture unit full - draw building!");
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
                                        outbuildtexmap.insert(std::make_pair(texname, encode));
                                        numoutbuildtexloaded[i]++;
                                    } else {
                                        logger->warning("Texture layer full - draw building!");
                                        texunit = 0;
                                        texlayer = 0;
                                    }
                                }
                            }

                            if (face.IsTextureFrameTable()) {
                                // TODO(pskelton): any instances where animTime is not consistent would need checking
                                frame += pTextureFrameTable->textureFrameAnimTime((int64_t)face.resource);
                                tex = pTextureFrameTable->GetFrameTexture((int64_t)face.resource, frame);
                                if (!tex) break;
                                texname = tex->GetName();
                            }
                        } while (animLength > frame);

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
                    auto texture = assets->getBitmap(it->first);

                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0, 0, tlayer,
                        outbuildtexturewidths[unit], outbuildtextureheights[unit], 1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        texture->rgba().pixels().data());
                }

                it++;
            }

            //iterate through terrain tex map
            //ignore wtrtyl
            //laod in

            //auto tile = pOutdoor->getTileDescByGrid(0, 0);
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
                            array_73D150[0].vWorldPosition = model.pVertices[face.pVertexIDs[0]];

                            if (pCamera3D->is_face_faced_to_cameraODM(&face, &array_73D150[0])) {
                                int texunit = 0;
                                int texlayer = 0;

                                if (face.IsTextureFrameTable()) {
                                    texlayer = -1;
                                    texunit = -1;
                                } else {
                                    texlayer = face.texlayer;
                                    texunit = face.texunit;
                                }

                                if (texlayer == -1) { // texture has been reset - see if its in the map
                                    GraphicsImage *tex = face.GetTexture();
                                    std::string texname = tex->GetName();
                                    auto mapiter = bsptexmap.find(texname);
                                    if (mapiter != bsptexmap.end()) {
                                        // if so, extract unit and layer
                                        int unitlayer = mapiter->second;
                                        face.texlayer = texlayer = unitlayer & 0xFF;
                                        face.texunit = texunit = (unitlayer & 0xFF00) >> 8;
                                    } else {
                                        logger->warning("Texture not found in map!");
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

                                if (face.uAttributes & FACE_IsLava)
                                    attribflags |= 0x4000;

                                if (face.uAttributes & FACE_OUTLINED || (face.uAttributes & FACE_IsSecret) && engine->is_saturate_faces)
                                    attribflags |= 0x00010000;

                                // load up verts here
                                for (int z = 0; z < (face.uNumVertices - 2); z++) {
                                    // 123, 134, 145, 156..
                                    GLshaderverts *thisvert = &outbuildshaderstore[texunit][numoutbuildverts[texunit]];

                                    // copy first
                                    thisvert->x = model.pVertices[face.pVertexIDs[0]].x;
                                    thisvert->y = model.pVertices[face.pVertexIDs[0]].y;
                                    thisvert->z = model.pVertices[face.pVertexIDs[0]].z;
                                    thisvert->u = face.pTextureUIDs[0] + face.sTextureDeltaU;
                                    thisvert->v = face.pTextureVIDs[0] + face.sTextureDeltaV;
                                    thisvert->texunit = texunit;
                                    thisvert->texturelayer = texlayer;
                                    thisvert->normx = face.facePlane.normal.x;
                                    thisvert->normy = face.facePlane.normal.y;
                                    thisvert->normz = face.facePlane.normal.z;
                                    thisvert->attribs = attribflags;
                                    thisvert++;

                                    // copy other two (z+1)(z+2)
                                    for (unsigned i = 1; i < 3; ++i) {
                                        thisvert->x = model.pVertices[face.pVertexIDs[z + i]].x;
                                        thisvert->y = model.pVertices[face.pVertexIDs[z + i]].y;
                                        thisvert->z = model.pVertices[face.pVertexIDs[z + i]].z;
                                        thisvert->u = face.pTextureUIDs[z + i] + face.sTextureDeltaU;
                                        thisvert->v = face.pTextureVIDs[z + i] + face.sTextureDeltaV;
                                        thisvert->texunit = texunit;
                                        thisvert->texturelayer = texlayer;
                                        thisvert->normx = face.facePlane.normal.x;
                                        thisvert->normy = face.facePlane.normal.y;
                                        thisvert->normz = face.facePlane.normal.z;
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
    if (engine->config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glUseProgram(outbuildshader.ID);
    // set projection
    glUniformMatrix4fv(glGetUniformLocation(outbuildshader.ID, "projection"), 1, GL_FALSE, &projmat[0][0]);
    // set view
    glUniformMatrix4fv(glGetUniformLocation(outbuildshader.ID, "view"), 1, GL_FALSE, &viewmat[0][0]);

    glUniform1i(glGetUniformLocation(outbuildshader.ID, "waterframe"), GLint(this->hd_water_current_frame));
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "flowtimer"), GLint(pMiscTimer->time().realtimeMilliseconds() >> 4));
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "flowtimerms"), GLint(pMiscTimer->time().realtimeMilliseconds()));

    glUniform1f(glGetUniformLocation(outbuildshader.ID, "gamma"), gamma);

    // set texture unit location
    glUniform1i(glGetUniformLocation(outbuildshader.ID, "textureArray0"), GLint(0));

    // set fog uniforms
    glUniform3f(glGetUniformLocation(outbuildshader.ID, "fog.color"), fog.r, fog.g, fog.b);
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogstart"), GLfloat(fogstart));
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogmiddle"), GLfloat(fogmiddle));
    glUniform1f(glGetUniformLocation(outbuildshader.ID, "fog.fogend"), GLfloat(fogend));

    GLfloat camera[3] {};
    camera[0] = (float)(pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0f));
    camera[1] = (float)(pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0f));
    camera[2] = (float)(pParty->pos.z + pParty->eyeLevel);
    glUniform3fv(glGetUniformLocation(outbuildshader.ID, "CameraPos"), 1, &camera[0]);


    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0f;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0f) * 2 * pi_double) / 1440) + 1) * 0.27f;
    float diffuseon = pWeather->bNight ? 0.0f : 1.0f;

    glUniform3fv(glGetUniformLocation(outbuildshader.ID, "sun.direction"), 1, &pOutdoor->vSunlight.x);
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

        MobileLight &test = pMobileLightsStack->pLights[i];
        std::string slotnum = std::to_string(num_lights);

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        StationaryLight &test = pStationaryLightsStack->pLights[i];

        float x = test.vPosition.x;
        float y = test.vPosition.y;
        float z = test.vPosition.z;

        Colorf color = test.uLightColor.toColorf();

        float lightrad = test.uRadius;

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        std::string slotnum = std::to_string(num_lights);
        MobileLight &test = pMobileLightsStack->pLights[i];

        float x = pMobileLightsStack->pLights[i].vPosition.x;
        float y = pMobileLightsStack->pLights[i].vPosition.y;
        float z = pMobileLightsStack->pLights[i].vPosition.z;

        Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

        float lightrad = pMobileLightsStack->pLights[i].uRadius;

        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
        glUniform3f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), color.r, color.g, color.b);
        glUniform1f(glGetUniformLocation(outbuildshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), lightrad);

        num_lights++;
    }

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
    glBindTexture(GL_TEXTURE_2D, 0);

    //end terrain debug
    if (engine->config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // TODO(pskelton): clean up
    // need to stack decals
    if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;

    for (BSPModel &model : pOutdoor->pBModels) {
        if (model.pFaces.empty()) {
            continue;
        }

        // check for any splat in this models box - if not continue
        bool found{ false };
        for (int splat = 0; splat < decal_builder->bloodsplat_container->uNumBloodsplats; ++splat) {
            Bloodsplat *thissplat = &decal_builder->bloodsplat_container->pBloodsplats_to_apply[splat];
            if (model.pBoundingBox.intersectsCube(thissplat->pos, thissplat->radius)) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        for (ODMFace &face : model.pFaces) {
            if (face.Invisible()) {
                continue;
            }

            Polygon *poly = &array_77EC08[pODMRenderParams->uNumPolygons];
            poly->flags = 0;
            poly->field_32 = 0;

            // if (v53 == face.uNumVertices) poly->field_32 |= 1;
            poly->pODMFace = &face;
            poly->uNumVertices = face.uNumVertices;
            poly->field_59 = 5;


            float _f1 = face.facePlane.normal.x * pOutdoor->vSunlight.x + face.facePlane.normal.y * pOutdoor->vSunlight.y + face.facePlane.normal.z * pOutdoor->vSunlight.z;
            poly->dimming_level = 20.0 - floorf(20.0 * _f1 + 0.5f);
            poly->dimming_level = std::clamp((int)poly->dimming_level, 0, 31);

            for (unsigned vertex_id = 1; vertex_id <= face.uNumVertices; vertex_id++) {
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

            decal_builder->ApplyBloodSplat_OutdoorFace(&face);
            if (decal_builder->uNumSplatsThisFace > 0) {
                decal_builder->BuildAndApplyDecals(
                    31 - poly->dimming_level, LocationBuildings,
                    face.facePlane,
                    face.uNumVertices, VertexRenderList, 0, -1);
            }
        }
    }

    return;



    ///////////////// shader end
}

GLshaderverts *BSPshaderstore[16] = { nullptr };
int numBSPverts[16] = { 0 };

void OpenGLRenderer::DrawIndoorFaces() {
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
                StationaryLight &test = pStationaryLightsStack->pLights[lightscnt];


                // kludge for getting lights in  visible sectors
                pStationaryLightsStack->pLights[lightscnt].uSectorID = pIndoor->GetSector(test.vPosition);

                if (pStationaryLightsStack->pLights[lightscnt].uSectorID == 0) cntnosect++;
            }
            if (cntnosect)
                logger->warning("{} lights - sector not found", cntnosect);

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
            bsptexturewidths[0] = wtrtexture->width();
            bsptextureheights[0] = wtrtexture->height();

            for (int buff = 0; buff < 7; buff++) {
                std::string container_name = fmt::format("HDWTR{:03}", buff);
                bsptexmap.insert(std::make_pair(container_name, bsptexmap.size()));
                bsptexloaded[0]++;
            }


            for (int test = 0; test < pIndoor->pFaces.size(); test++) {
                BLVFace *face = &pIndoor->pFaces[test];

                if (face->isPortal()) continue;
                if (!face->GetTexture()) continue;
                //if (face->uAttributes & FACE_IS_DOOR) continue;

                // TODO(pskelton): Same as outdoors. When ODM and BLV face is combined - seperate out function
                GraphicsImage *tex = face->GetTexture();
                std::string texname = tex->GetName();

                Duration animLength;
                Duration frame;
                if (face->IsTextureFrameTable()) {
                    tex = pTextureFrameTable->GetFrameTexture((int64_t)face->resource, frame);
                    animLength = pTextureFrameTable->textureFrameAnimLength((int64_t)face->resource);
                    texname = tex->GetName();
                }

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

                if (face->uAttributes & FACE_IsLava)
                    attribflags |= 0x4000;

                if (face->uAttributes & (FACE_OUTLINED | FACE_IsSecret))
                    attribflags |= 0x00010000;

                // loop while running down animlength with frame animtimes
                do {
                    // check if tile->name is already in list
                    auto mapiter = bsptexmap.find(texname);
                    if (mapiter != bsptexmap.end()) {
                        // if so, extract unit and layer
                        int unitlayer = mapiter->second;
                        // TODO(pskelton): make this a pair/struct rather than encoding
                        texlayer = unitlayer & 0xFF;
                        texunit = (unitlayer & 0xFF00) >> 8;
                    } else if (texname == "wtrtyl") {
                        // water tile
                        texunit = 0;
                        texlayer = 0;
                    } else {
                        // else need to add it
                        auto thistexture = assets->getBitmap(texname);
                        int width = thistexture->width();
                        int height = thistexture->height();
                        // check size to see what unit it needs
                        int i;
                        for (i = 0; i < 16; i++) {
                            if ((bsptexturewidths[i] == width && bsptextureheights[i] == height) || bsptexturewidths[i] == 0) break;
                        }

                        if (i == 16) {
                            logger->warning("Texture unit full - draw Indoor faces!");
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
                                bsptexmap.insert(std::make_pair(texname, encode));
                                bsptexloaded[i]++;
                            } else {
                                logger->warning("Texture layer full - draw indoor faces!");
                                texunit = 0;
                                texlayer = 0;
                            }
                        }
                    }

                    if (face->IsTextureFrameTable()) {
                        // TODO(pskelton): any instances where animTime is not consistent would need checking
                        frame += pTextureFrameTable->textureFrameAnimTime((int64_t)face->resource);
                        tex = pTextureFrameTable->GetFrameTexture((int64_t)face->resource, frame);
                        if (!tex) break;
                        texname = tex->GetName();
                    }
                } while (animLength > frame);

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
                        auto texture = assets->getBitmap(it->first);

                        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                            0,
                            0, 0, tlayer,
                            bsptexturewidths[unit], bsptextureheights[unit], 1,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            texture->rgba().pixels().data());

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

            for (unsigned i = 0; i < pBspRenderer->num_faces; ++i) {
                int uFaceID = pBspRenderer->faces[i].uFaceID;
                if (uFaceID >= pIndoor->pFaces.size())
                    continue;
                BLVFace *face = &pIndoor->pFaces[uFaceID];

                if (face->isPortal()) {
                    continue;
                }

                if (face->uNumVertices < 3) continue;

                if (face->Invisible()) {
                    continue;
                }

                if (!face->GetTexture()) {
                    continue;
                }

                Planef *portalfrustumnorm = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].ViewportNodeFrustum.data();
                unsigned int uNumFrustums = 4;
                RenderVertexSoft *pPortalBounding = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].pPortalBounding.data();

                // unsigned ColourMask;  // ebx@25
                unsigned int uNumVerticesa;  // [sp+24h] [bp-4h]@17
                // int LightLevel;                     // [sp+34h] [bp+Ch]@25

                static RenderVertexSoft static_vertices_buff_in[64];  // buff in
                static RenderVertexSoft static_vertices_calc_out[64];  // buff out - calc portal shape

                // moved face to camera check to avoid missing minimap outlines
                if (/*pCamera3D->is_face_faced_to_cameraBLV(face) ||*/ true) {
                    uNumVerticesa = face->uNumVertices;

                    // copy to buff in
                    for (unsigned i = 0; i < face->uNumVertices; ++i) {
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
                            // TODO(pskelton): check tickcount usage here
                            skymodtimex = (platform->tickCount() / 32.0f) - pCamera3D->vCameraPos.x;
                            skymodtimey = (platform->tickCount() / 32.0f) + pCamera3D->vCameraPos.y;
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

                            if (face->uAttributes & FACE_IsLava)
                                attribflags |= 0x4000;

                            if (face->uAttributes & FACE_OUTLINED || (face->uAttributes & FACE_IsSecret) && engine->is_saturate_faces)
                                attribflags |= 0x00010000;

                            if (face->IsTextureFrameTable()) {
                                texlayer = -1;
                                texunit = -1;
                            } else {
                                texlayer = face->texlayer;
                                texunit = face->texunit;
                            }

                            if (texlayer == -1) { // texture has been reset - see if its in the map
                                GraphicsImage *tex = face->GetTexture();
                                std::string texname = tex->GetName();
                                auto mapiter = bsptexmap.find(texname);
                                if (mapiter != bsptexmap.end()) {
                                    // if so, extract unit and layer
                                    int unitlayer = mapiter->second;
                                    face->texlayer = texlayer = unitlayer & 0xFF;
                                    face->texunit = texunit = (unitlayer & 0xFF00) >> 8;
                                } else {
                                    logger->warning("Texture not found in map!");
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
                                thisvert->normx = face->facePlane.normal.x;
                                thisvert->normy = face->facePlane.normal.y;
                                thisvert->normz = face->facePlane.normal.z;
                                thisvert->attribs = attribflags;
                                thisvert->sector = face->uSectorID;
                                thisvert++;

                                // copy other two (z+1)(z+2)
                                for (unsigned i = 1; i < 3; ++i) {
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
                                    thisvert->normx = face->facePlane.normal.x;
                                    thisvert->normy = face->facePlane.normal.y;
                                    thisvert->normz = face->facePlane.normal.z;
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
        if (engine->config->debug.Terrain.value())
            // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
            if (!OpenGLES)
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

        glUniform1i(glGetUniformLocation(bspshader.ID, "waterframe"), GLint(this->hd_water_current_frame));
        glUniform1i(glGetUniformLocation(bspshader.ID, "flowtimer"), GLint(pMiscTimer->time().realtimeMilliseconds() >> 4));
        glUniform1i(glGetUniformLocation(bspshader.ID, "flowtimerms"), GLint(pMiscTimer->time().realtimeMilliseconds()));

        glUniform1f(glGetUniformLocation(bspshader.ID, "gamma"), gamma);

        // set texture unit location
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray0"), GLint(0));
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray1"), GLint(1));
        glUniform1i(glGetUniformLocation(bspshader.ID, "textureArray2"), GLint(2));


        GLfloat camera[3] {};
        camera[0] = (float)(pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0f));
        camera[1] = (float)(pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0f));
        camera[2] = (float)(pParty->pos.z + pParty->eyeLevel);
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

        int uCurrentAmbientLightLevel = (DEFAULT_AMBIENT_LIGHT_LEVEL + mintest);

        float ambient = (248.0f - (uCurrentAmbientLightLevel << 3)) / 255.0f;
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

            MobileLight &test = pMobileLightsStack->pLights[i];
            std::string slotnum = std::to_string(num_lights);

            float x = pMobileLightsStack->pLights[i].vPosition.x;
            float y = pMobileLightsStack->pLights[i].vPosition.y;
            float z = pMobileLightsStack->pLights[i].vPosition.z;

            Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

        // stack the static lights next (wall torches)
        for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            StationaryLight &test = pStationaryLightsStack->pLights[i];

            // is this on the sector list
            bool onlist = false;
            for (unsigned i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
                if (test.uSectorID == listsector) {
                    onlist = true;
                    break;
                }
            }

            // does light sphere collide with current sector
            // expanded current sector
            bool fromexpanded{ false };
            if (pIndoor->pSectors[pBLVRenderParams->uPartySectorID].pBounding.intersectsCube(test.vPosition, test.uRadius)) {
                onlist = true;
                fromexpanded = true;
            }

            if (!onlist) continue;

            // cull through viewing frustum
            bool visinfrustum{ false };
            if (!fromexpanded) {
                for (int i = 0; i < pBspRenderer->num_nodes; ++i) {
                    if (pBspRenderer->nodes[i].uSectorID == test.uSectorID) {
                        if (IsSphereInFrustum(test.vPosition, test.uRadius, pBspRenderer->nodes[i].ViewportNodeFrustum.data()))
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

            Colorf color = test.uLightColor.toColorf();

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 1.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), test.uSectorID);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

        // whatevers left for mobile lights
        for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            // TODO(pskelton): nearest lights should be prioritsed
            MobileLight &test = pMobileLightsStack->pLights[i];
            if (!IsSphereInFrustum(test.vPosition, test.uRadius)) continue;

            std::string slotnum = std::to_string(num_lights);

            float x = pMobileLightsStack->pLights[i].vPosition.x;
            float y = pMobileLightsStack->pLights[i].vPosition.y;
            float z = pMobileLightsStack->pLights[i].vPosition.z;

            Colorf color = pMobileLightsStack->pLights[i].uLightColor.toColorf();

            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].type").c_str()), 2.0f);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].position").c_str()), x, y, z);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].sector").c_str()), 0);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].ambient").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].diffuse").c_str()), color.r, color.g, color.b);
            glUniform3f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].specular").c_str()), 0, 0, 0);
            glUniform1f(glGetUniformLocation(bspshader.ID, ("fspointlights[" + slotnum + "].radius").c_str()), test.uRadius);
            num_lights++;
        }

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
        glBindTexture(GL_TEXTURE_2D, 0);

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
        if (engine->config->debug.Terrain.value())
            // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
            if (!OpenGLES)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // stack decals start

        if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;
        static RenderVertexSoft static_vertices_buff_in[64];  // buff in

        // loop over faces
        for (int test = 0; test < pIndoor->pFaces.size(); test++) {
            BLVFace *pface = &pIndoor->pFaces[test];

            if (pface->isPortal()) continue;
            if (!pface->GetTexture()) continue;

            // check if faces is visible
            bool onlist = false;
            for (unsigned i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
                if (pface->uSectorID == listsector) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;


            decal_builder->ApplyBloodsplatDecals_IndoorFace(test);
            if (!decal_builder->uNumSplatsThisFace) continue;

            // copy to buff in
            for (unsigned i = 0; i < pface->uNumVertices; ++i) {
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
            decal_builder->BuildAndApplyDecals(uCurrentAmbientLightLevel, LocationIndoors, pface->facePlane,
                pface->uNumVertices, static_vertices_buff_in,
                0, pface->uSectorID);
        }



        ///////////////////////////////////////////////////////
        // stack decals end

        return;
}

bool OpenGLRenderer::SwitchToWindow() {
    // pViewport->ResetScreen();
    // CreateZBuffer();

    return true;
}


bool OpenGLRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }

    if (window != nullptr) {
        PlatformOpenGLOptions opts;

        // Set it only on startup as currently we don't support multiple contexts to be able to switch OpenGL<->OpenGLES in the middle of runtime.
        OpenGLES = config->graphics.Renderer.value() == RENDERER_OPENGL_ES;

        if (!OpenGLES) {
            //  Use OpenGL 4.1 core
            opts.versionMajor = 4;
            opts.versionMinor = 1;
            opts.profile = GL_PROFILE_CORE;
        } else {
            //  Use OpenGL ES 3.2
            opts.versionMajor = 3;
            opts.versionMinor = 2;
            opts.profile = GL_PROFILE_ES;
        }

        //  Turn on 24bit Z buffer.
        //  You may need to change this to 16 or 32 for your system
        opts.depthBits = 24;
        opts.stencilBits = 8;

        opts.vsyncMode = config->graphics.VSync.value() ? GL_VSYNC_ADAPTIVE : GL_VSYNC_NONE;

        application->initializeOpenGLContext(opts);

        auto gladLoadFunc = [](void *ptr, const char *name) {
            return reinterpret_cast<GLADapiproc>(static_cast<PlatformOpenGLContext *>(ptr)->getProcAddress(name));
        };

        int version;
        if (OpenGLES)
            version = gladLoadGLES2UserPtr(gladLoadFunc, openGLContext);
        else
            version = gladLoadGLUserPtr(gladLoadFunc, openGLContext);

        if (!version)
            logger->warning("GLAD: Failed to initialize the OpenGL loader");

        if (version) {
            logger->info("SDL2: supported OpenGL: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
            logger->info("SDL2: supported GLSL: {}", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
            logger->info("SDL2: OpenGL version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
        }

        gladSetGLPostCallback(GL_Check_Errors);

        _overlayRenderer = std::make_unique<NuklearOverlayRenderer>();

        return Reinitialize(true);
    }

    return false;
}

void OpenGLRenderer::FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth,
                                  unsigned int uHeight, Color uColor32) {
    Colorf cf = uColor32.toColorf();

    float depth = 0;
    int x = uX;
    int y = uY;
    int z = x + uWidth;
    int w = y + uHeight;

    // check bounds
    if (x >= outputRender.w || x >= this->clip_z || y >= outputRender.h || y >= this->clip_w) return;
    // check for overlap
    if (!(this->clip_x < z && this->clip_z > x && this->clip_y < w && this->clip_w > y)) return;

    static GraphicsImage *effpar03 = assets->getBitmap("effpar03");
    float gltexid = static_cast<float>(effpar03->renderId().value());

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
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    ////////////////////////////////

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = drawy;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texy;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawz;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texz;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    twodshaderstore[twodvertscnt].x = drawx;
    twodshaderstore[twodvertscnt].y = draww;
    twodshaderstore[twodvertscnt].z = 0;
    twodshaderstore[twodvertscnt].u = texx;
    twodshaderstore[twodvertscnt].v = texw;
    twodshaderstore[twodvertscnt].color = cf;
    twodshaderstore[twodvertscnt].texid = gltexid;
    twodshaderstore[twodvertscnt].paletteid = 0;
    twodvertscnt++;

    if (twodvertscnt > 490) DrawTwodVerts();
    return;
}

// gl shaders
bool OpenGLRenderer::InitShaders() {
    logger->info("initialising OpenGL shaders...");

    std::string title = "CRITICAL ERROR: shader compilation failure";
    std::string name = "Terrain";
    std::string message = "shader failed to compile!\nPlease consult the log and consider issuing a bug report!";
    terrainshader.build(name, "glterrain", OpenGLES);
    if (terrainshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }

    name = "Outdoor buildings";
    outbuildshader.build(name, "gloutbuild", OpenGLES);
    if (outbuildshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }

    name = "Indoor BSP";
    bspshader.build(name, "glbspshader", OpenGLES);
    if (bspshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }

    name = "Text";
    textshader.build(name, "gltextshader", OpenGLES);
    if (textshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    textVAO = 0;

    name = "Lines";
    lineshader.build(name, "gllinesshader", OpenGLES);
    if (lineshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    lineVAO = 0;

    name = "2D";
    twodshader.build(name, "gltwodshader", OpenGLES);
    if (twodshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    twodVAO = 0;

    name = "Billboards";
    billbshader.build(name, "glbillbshader", OpenGLES);
    if (billbshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    billbVAO = 0;
    palbuf = 0;

    name = "Decals";
    decalshader.build(name, "gldecalshader", OpenGLES);
    if (decalshader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    decalVAO = 0;

    name = "Forced perspective";
    forcepershader.build(name, "glforcepershader", OpenGLES);
    if (forcepershader.ID == 0) {
        platform->showMessageBox(title, fmt::format("{} {}", name, message));
        return false;
    }
    forceperVAO = 0;

    logger->info("shaders have been compiled successfully!");
    return true;
}

bool OpenGLRenderer::Reinitialize(bool firstInit) {
    BaseRenderer::Reinitialize(firstInit);

    if (!firstInit) {
        game_viewport_x = viewparams->uScreen_topL_X = engine->config->graphics.ViewPortX1.value(); //8
        game_viewport_y = viewparams->uScreen_topL_Y = engine->config->graphics.ViewPortY1.value(); //8
        game_viewport_z = viewparams->uScreen_BttmR_X = outputRender.w - engine->config->graphics.ViewPortX2.value(); //468;
        game_viewport_w = viewparams->uScreen_BttmR_Y = outputRender.h - engine->config->graphics.ViewPortY2.value(); //352;

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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // Black Background
    glClearDepthf(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if (firstInit) {
        // clear only on first init as it will introduce brief black artifacts on window resize
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(2, framebufferTextures);

    glGenFramebuffers(1, &framebuffer);
    glGenTextures(2, framebufferTextures);

    glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outputRender.w, outputRender.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, framebufferTextures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, outputRender.w, outputRender.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glViewport(0, 0, outputRender.w, outputRender.h);
    glScissor(0, 0, outputRender.w, outputRender.h);
    glEnable(GL_SCISSOR_TEST);

    // Swap Buffers (Double Buffering)
    openGLContext->swapBuffers();

    this->clip_x = this->clip_y = 0;
    this->clip_z = outputRender.w;
    this->clip_w = outputRender.h;

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
            logger->warning("shader initialisation has failed!");
            return false;
        }
    } // else {

    if (config->window.ReloadTex.value()) {
        // Added config option for this - may not always be required - #199 no longer replicates on windows??
        // TODO: invalidate all previously loaded textures and then load them again as they can be no longer alive on GPU (issue #199).
        // TODO(pskelton): Needs testings on other platforms
        assets->releaseAllTextures();
        ReleaseTerrain();
        ReleaseBSP();
    }

    //     ReloadShaders();
    // }

    return BaseRenderer::Reinitialize(firstInit);
}

void OpenGLRenderer::ReloadShaders() {
    logger->info("reloading Shaders...");
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::string name = "Terrain";
    std::string message = "shader failed to reload!\nPlease consult the log and issue a bug report!";
    if (!terrainshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    name = "Outdoor buildings";
    if (!outbuildshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    ReleaseTerrain();

    name = "Indoor BSP";
    if (!bspshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    ReleaseBSP();

    name = "Text";
    if (!textshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    textVAO = textVBO = 0;
    textvertscnt = 0;

    name = "Lines";
    if (!lineshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    lineVAO = lineVBO = 0;
    linevertscnt = 0;

    name = "2D";
    if (!twodshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &twodVAO);
    glDeleteBuffers(1, &twodVBO);
    twodVAO = twodVBO = 0;
    twodvertscnt = 0;

    name = "Billboards";
    if (!billbshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &billbVAO);
    glDeleteBuffers(1, &billbVBO);
    billbVAO = billbVBO = 0;
    glDeleteTextures(1, &paltex);
    glDeleteBuffers(1, &palbuf);
    paltex = palbuf = 0;
    billbstorecnt = 0;

    name = "Decals";
    if (!decalshader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &decalVAO);
    glDeleteBuffers(1, &decalVBO);
    decalVAO = decalVBO = 0;
    numdecalverts = 0;

    name = "Forced perspective";
    if (!forcepershader.reload(name, OpenGLES))
        logger->warning("{} {}", name, message);
    glDeleteVertexArrays(1, &forceperVAO);
    glDeleteBuffers(1, &forceperVBO);
    forceperVAO = forceperVBO = 0;
    forceperstorecnt = 0;

    if (_overlayRenderer) {
        _overlayRenderer->reloadShaders(OpenGLES);
    }
}

void OpenGLRenderer::drawOverlays(nk_context *context) {
    if (_overlayRenderer) {
        _overlayRenderer->render(context, outputPresent, OpenGLES, &drawcalls);
    }
}

void OpenGLRenderer::ReleaseTerrain() {
    /*GLuint terrainVBO, terrainVAO;
    GLuint terraintextures[8];
    unsigned numterraintexloaded[8];
    unsigned terraintexturesizes[8];
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
    unsigned numoutbuildtexloaded[8];
    unsigned outbuildtexturewidths[8];
    unsigned outbuildtextureheights[8];
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

void OpenGLRenderer::ReleaseBSP() {
    /*GLuint bspVBO, bspVAO;
    GLuint bsptextures[16];
    unsigned bsptexloaded[16];
    unsigned bsptexturewidths[16];
    unsigned bsptextureheights[16];
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


void OpenGLRenderer::DrawTwodVerts() {
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
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(twodverts), (void*)offsetof(twodverts, color));
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
        std::span<Color> palettes = pPaletteManager->paletteData();
        glGenBuffers(1, &palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, palbuf);
        glBufferData(GL_TEXTURE_BUFFER, palettes.size_bytes(), palettes.data(), GL_STATIC_DRAW);

        glGenTextures(1, &paltex);
        glBindTexture(GL_TEXTURE_BUFFER, paltex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, palbuf);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }

    // update buffer
    glBindBuffer(GL_ARRAY_BUFFER, twodVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(twodverts) * twodvertscnt, twodshaderstore);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
        if (twodshaderstore[offset].paletteid) {
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
