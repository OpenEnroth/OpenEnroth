#include "OpenGLRenderer.h"

#include "OpenGLVertexBuffer.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <map>
#include <string>
#include <tuple>

#include <glad/gl.h> // NOLINT: not a C system header.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/backends/imgui_impl_opengl3.h> // NOLINT: not a C system header.
#include <imgui/backends/imgui_impl_sdl3.h> // NOLINT: not a C system header.

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Tables/TextureFrameTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineCallObserver.h"

#include "Library/FileSystem/Sub/SubFileSystem.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Color/Colorf.h"
#include "Library/Logger/Logger.h"
#include "Library/Geometry/Size.h"
#include "Library/Geometry/Vec.h"
#include "Library/Image/ImageFunctions.h"

#include "Utility/String/Format.h"

#include "OpenGLShader.h"

#ifndef LOWORD
    #define LOWORD(l) ((unsigned short)(((std::uintptr_t)(l)) & 0xFFFF))
#endif

static constexpr int DEFAULT_AMBIENT_LIGHT_LEVEL = 0;

// globals
//TODO(pskelton): Combine and contain
RenderBillboard pBillboardRenderList[500];
int uNumBillboardsToDraw;
int uNumDecorationsDrawnThisFrame;
int uNumSpritesDrawnThisFrame;
RenderVertexSoft array_73D150[20];
RenderVertexSoft VertexRenderList[50];

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

static int waterAnimationFrame() {
    // Water animation in vanilla was borked, or so it seems. Water has 7 frames, frame durations are:
    //
    // Frame    0       1       2       3       4       5       6       Total
    // Vanilla  1/12s   1/6s    1/6s    1/6s    1/6s    1/6s    1/12s   1s
    // OE       1/7s    1/7s    1/7s    1/7s    1/7s    1/7s    1/7s    1s
    return static_cast<int>(std::floor(std::fmod(pMiscTimer->time().realtimeMillisecondsFloat(), 1.0f) * 7.0f));
}

OpenGLRenderer::OpenGLRenderer(
    std::shared_ptr<GameConfig> config,
    DecalBuilder *decal_builder,
    SpellFxRenderer *spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis *vis
) : BaseRenderer(config, decal_builder, spellfx, particle_engine, vis) {}

OpenGLRenderer::~OpenGLRenderer() {
    logger->info("RenderGl - Destructor");
    _shutdownImGui();
}

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

void OpenGLRenderer::BeginLines2D() {
    if (!_lineVertices.empty())
        logger->trace("BeginLines with points still stored in buffer");

    DrawTwodVerts();

    if (!_lineBuffer) {
        _lineBuffer.reset(GL_DYNAMIC_DRAW,
            &LineVertex::pos,
            &LineVertex::color);
    }
}

void OpenGLRenderer::EndLines2D() {
    if (_lineVertices.empty()) return;

    _lineBuffer.update(_lineVertices);
    _lineBuffer.bind();

    lineshader.use();

    LineUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.submit(lineshader);

    glDrawArrays(GL_LINES, 0, _lineVertices.size());
    drawcalls++;

    lineshader.unuse();
    _lineBuffer.unbind();

    _lineVertices.clear();
}

void OpenGLRenderer::RasterLine2D(Pointi a, Pointi b, Color acolor, Color bcolor) {
    LineVertex &v1 = _lineVertices.emplace_back();
    v1.pos = a.toFloat();
    v1.color = acolor.toColorf();

    LineVertex &v2 = _lineVertices.emplace_back();
    v2.pos = b.toFloat();
    v2.color = bcolor.toColorf();
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
    v29[0].texcoord.x = 1.0;
    v29[0].texcoord.y = 0.0;

    v29[1].pos.x = dstxmod + dstX;
    v29[1].pos.y = dstY - dstymod;
    v29[1].pos.z = dstz;
    v29[1].rhw = dstrhw;
    v29[1].diffuse = colorTable.White;
    v29[1].texcoord.x = 1.0;
    v29[1].texcoord.y = 1.0;

    v29[2].pos.x = dstX - dstxmod;
    v29[2].pos.y = dstymod + dstY;
    v29[2].pos.z = dstz;
    v29[2].rhw = dstrhw;
    v29[2].diffuse = colorTable.White;
    v29[2].texcoord.x = 0.0;
    v29[2].texcoord.y = 1.0;

    v29[3].pos.x = srcX - srcxmod;
    v29[3].pos.y = srcymod + srcY;
    v29[3].pos.z = srcz;
    v29[3].rhw = srcrhw;
    v29[3].diffuse = colorTable.White;
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
        // copy first
        ForcePerVertex &v0 = _forcePerVertices.emplace_back();
        v0.pos = v29[0].pos;
        v0.w = 1.0f;
        v0.texuv = v29[0].texcoord;
        v0.texw = v29[0].rhw;
        v0.screenspace = srcworldview;
        v0.color = colorTable.White.toColorf();
        v0.texid = texid;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            ForcePerVertex &v = _forcePerVertices.emplace_back();
            v.pos = v29[z + i].pos;
            v.w = 1.0f;
            v.texuv = v29[z + i].texcoord;
            v.texw = v29[z + i].rhw;
            v.screenspace = (z + i == 3) ? srcworldview: dstworldview;
            v.color = colorTable.White.toColorf();
            v.texid = texid;
        }
    }

    // TODO(pskelton): do these need batching?
    DrawForcePerVerts();

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    //ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void OpenGLRenderer::ScreenFade(Color color, float t) {
    Colorf cf = color.toColorf();
    cf.a = std::clamp(t, 0.0f, 1.0f);

    float drawx = static_cast<float>(pViewport.x);
    float drawy = static_cast<float>(pViewport.y);
    float drawz = static_cast<float>(pViewport.x + pViewport.w - 1);
    float draww = static_cast<float>(pViewport.y + pViewport.h - 1);

    float gltexid = static_cast<float>(solidFillTexture()->renderId().value());

    // 0 1 2 / 0 2 3

    TwoDVertex &v0 = _twodVertices.emplace_back();
    v0.pos = Vec3f(drawx, drawy, 0);
    v0.texuv = Vec2f(0.5f, 0.5f);
    v0.color = cf;
    v0.texid = gltexid;
    v0.paletteid = 0;

    TwoDVertex &v1 = _twodVertices.emplace_back();
    v1.pos = Vec3f(drawz, drawy, 0);
    v1.texuv = Vec2f(0.5f, 0.5f);
    v1.color = cf;
    v1.texid = gltexid;
    v1.paletteid = 0;

    TwoDVertex &v2 = _twodVertices.emplace_back();
    v2.pos = Vec3f(drawz, draww, 0);
    v2.texuv = Vec2f(0.5f, 0.5f);
    v2.color = cf;
    v2.texid = gltexid;
    v2.paletteid = 0;

    ////////////////////////////////

    TwoDVertex &v3 = _twodVertices.emplace_back();
    v3.pos = Vec3f(drawx, drawy, 0);
    v3.texuv = Vec2f(0.5f, 0.5f);
    v3.color = cf;
    v3.texid = 0;
    v3.paletteid = 0;

    TwoDVertex &v4 = _twodVertices.emplace_back();
    v4.pos = Vec3f(drawz, draww, 0);
    v4.texuv = Vec2f(0.5f, 0.5f);
    v4.color = cf;
    v4.texid = gltexid;
    v4.paletteid = 0;

    TwoDVertex &v5 = _twodVertices.emplace_back();
    v5.pos = Vec3f(drawx, draww, 0);
    v5.texuv = Vec2f(0.5f, 0.5f);
    v5.color = cf;
    v5.texid = gltexid;
    v5.paletteid = 0;

    return;
}


void OpenGLRenderer::DrawImage(GraphicsImage *img, const Recti &rect, int paletteid, Color uColor32) {
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
    if (x >= outputRender.w || y >= outputRender.h)
        return;

    // check for overlap
    Recti clippedRect = rect & this->clipRect;
    if (clippedRect.isEmpty())
        return;

    float gltexid = static_cast<float>(img->renderId().value());

    float drawx = clippedRect.x;
    float drawy = clippedRect.y;
    float drawz = clippedRect.x + clippedRect.w;
    float draww = clippedRect.y + clippedRect.h;

    float texx = (drawx - x) / float(z - x);
    float texy = (drawy - y) / float(w - y);
    float texz = (drawz - x) / float(z - x);
    float texw = (draww - y) / float(w - y);

    // 0 1 2 / 0 2 3

    TwoDVertex &v0 = _twodVertices.emplace_back();
    v0.pos = Vec3f(drawx, drawy, 0);
    v0.texuv = Vec2f(texx, texy);
    v0.color = cf;
    v0.texid = gltexid;
    v0.paletteid = paletteid;

    TwoDVertex &v1 = _twodVertices.emplace_back();
    v1.pos = Vec3f(drawz, drawy, 0);
    v1.texuv = Vec2f(texz, texy);
    v1.color = cf;
    v1.texid = gltexid;
    v1.paletteid = paletteid;

    TwoDVertex &v2 = _twodVertices.emplace_back();
    v2.pos = Vec3f(drawz, draww, 0);
    v2.texuv = Vec2f(texz, texw);
    v2.color = cf;
    v2.texid = gltexid;
    v2.paletteid = paletteid;

    ////////////////////////////////

    TwoDVertex &v3 = _twodVertices.emplace_back();
    v3.pos = Vec3f(drawx, drawy, 0);
    v3.texuv = Vec2f(texx, texy);
    v3.color = cf;
    v3.texid = gltexid;
    v3.paletteid = paletteid;

    TwoDVertex &v4 = _twodVertices.emplace_back();
    v4.pos = Vec3f(drawz, draww, 0);
    v4.texuv = Vec2f(texz, texw);
    v4.color = cf;
    v4.texid = gltexid;
    v4.paletteid = paletteid;

    TwoDVertex &v5 = _twodVertices.emplace_back();
    v5.pos = Vec3f(drawx, draww, 0);
    v5.texuv = Vec2f(texx, texw);
    v5.color = cf;
    v5.texid = gltexid;
    v5.paletteid = paletteid;

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

        RgbaImage dstImage = RgbaImage::solid(Color(), imgin->size());

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

        int w = imgin->width();
        int h = imgin->height();
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
        GraphicsImage *temp = GraphicsImage::Create(std::move(dstImage));
        render->DrawQuad2D(temp, {x, y});

        render->DrawTwodVerts();

        temp->release();
    }
}

// TODO(pskelton): renderbase
void OpenGLRenderer::DrawIndoorSky(int /*uNumVertices*/, int uFaceID) {
    BLVFace *pFace = &pIndoor->faces[uFaceID];
    if (pFace->numVertices <= 0) return;

    // TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
    if (!pFace->GetTexture()) return;

    int dimming_level = 0;
    unsigned int uNumVertices = pFace->numVertices;


    // TODO(pskelton): repeated maths could be saved when calculating sky planes
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  blv_horizon_height_offset = ((pCamera3D->ViewPlaneDistPixels * pCamera3D->vCameraPos.z)
        / (pCamera3D->ViewPlaneDistPixels + pCamera3D->GetFarClip())
        + (pBLVRenderParams->viewportRect.center().y));

    double cam_y_rot_rad = (double)pCamera3D->_viewPitch * rot_to_rads;

    float depth_to_far_clip = std::cos(pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = std::sin(pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();

    float blv_bottom_y_proj = ((pBLVRenderParams->viewportRect.center().y) -
        pCamera3D->ViewPlaneDistPixels /
        (depth_to_far_clip + 0.0000001f) *
        (height_to_far_clip - pCamera3D->vCameraPos.z));

    // rotation vec for sky plane - pitch
    float v_18x = -std::sin((-pCamera3D->_viewPitch + 16) * rot_to_rads);
    float v_18y = 0.0f;
    float v_18z = -std::cos((pCamera3D->_viewPitch + 16) * rot_to_rads);

    float inv_viewplanedist = 1.0f / pCamera3D->ViewPlaneDistPixels;

    RenderVertexSoft originalVertices[50];

    // copy to buff in
    for (unsigned i = 0; i < pFace->numVertices; ++i) {
        originalVertices[i].vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[i]].x;
        originalVertices[i].vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[i]].y;
        originalVertices[i].vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[i]].z;
        originalVertices[i].u = (signed short)pFace->textureUs[i];
        originalVertices[i].v = (signed short)pFace->textureVs[i];
    }

    // clip accurately to camera
    pCamera3D->ClipFaceToFrustum(originalVertices, &uNumVertices, VertexRenderList, pBspRenderer->nodes[0].ViewportNodeFrustum.data());
    if (!uNumVertices) return;

    pCamera3D->ViewTransform(VertexRenderList, uNumVertices);
    pCamera3D->Project(VertexRenderList, uNumVertices, false);

    unsigned _507D30_idx = 0;
    for (; _507D30_idx < uNumVertices; _507D30_idx++) {
        // outbound screen x dist
        float x_dist = inv_viewplanedist * (pBLVRenderParams->viewportRect.center().x - VertexRenderList[_507D30_idx].vWorldViewProj.x);
        // outbound screen y dist
        float y_dist = inv_viewplanedist * (blv_horizon_height_offset - VertexRenderList[_507D30_idx].vWorldViewProj.y);

        // rotate vectors to cam facing
        float skyfinalleft = (SkyBillboard.CamVecLeft_X * x_dist) + (SkyBillboard.CamVecLeft_Z * y_dist) + SkyBillboard.CamVecLeft_Y;
        float skyfinalfront = (SkyBillboard.CamVecFront_X * x_dist) + (SkyBillboard.CamVecFront_Z * y_dist) + SkyBillboard.CamVecFront_Y;

        // pitch rotate sky to get top projection
        float newX = v_18x + v_18y + (v_18z * y_dist);
        float worldviewdepth = -512.0f / newX;

        // offset tex coords
        float texoffset_U = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalleft * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].u = texoffset_U / (pFace->GetTexture()->width());
        float texoffset_V = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalfront * worldviewdepth) / 16.0f);
        VertexRenderList[_507D30_idx].v = texoffset_V / (pFace->GetTexture()->height());

        // this basically acts as texture perspective correction
        VertexRenderList[_507D30_idx]._rhw = worldviewdepth;
    }

    // no clipped polygon so draw and return??
    if (_507D30_idx >= uNumVertices) {
        DrawIndoorSkyPolygon(uNumVertices, pFace->GetTexture(), dimming_level);
        return;
    }
}

void OpenGLRenderer::DrawIndoorSkyPolygon(int uNumVertices, GraphicsImage *texture, int dimmingLevel) {
    int texid = texture->renderId().value();

    Colorf uTint = GetActorTintColor(dimmingLevel, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0).toColorf();
    float scrspace{ pCamera3D->GetFarClip() };

    float oneon = 1.0f / (pCamera3D->GetNearClip() * 2.0f);
    float oneof = 1.0f / (pCamera3D->GetFarClip());

    // load up poly
    for (int z = 0; z < (uNumVertices - 2); z++) {
        // 123, 134, 145, 156..
        float oneoz = 1.0f / VertexRenderList[0].vWorldViewPosition.x;
        float thisdepth = (oneoz - oneon) / (oneof - oneon);
        // copy first
        ForcePerVertex &v0 = _forcePerVertices.emplace_back();
        v0.pos = Vec3f(VertexRenderList[0].vWorldViewProj.x, VertexRenderList[0].vWorldViewProj.y, thisdepth);
        v0.w = VertexRenderList[0]._rhw;
        v0.texuv = Vec2f(VertexRenderList[0].u, VertexRenderList[0].v);
        v0.texw = 1.0f;
        v0.screenspace = scrspace;
        v0.color = uTint;
        v0.texid = texid;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            oneoz = 1.0f / VertexRenderList[z + i].vWorldViewPosition.x;
            thisdepth = (oneoz - oneon) / (oneof - oneon);
            ForcePerVertex &v = _forcePerVertices.emplace_back();
            v.pos = Vec3f(VertexRenderList[z + i].vWorldViewProj.x, VertexRenderList[z + i].vWorldViewProj.y, thisdepth);
            v.w = VertexRenderList[z + i]._rhw;
            v.texuv = Vec2f(VertexRenderList[z + i].u, VertexRenderList[z + i].v);
            v.texw = 1.0f;
            v.screenspace = scrspace;
            v.color = uTint;
            v.texid = texid;
        }
        // TODO (pskelton): should force drawing if buffer is full
    }
}

RgbaImage OpenGLRenderer::MakeViewportScreenshot(const int width, const int height) {
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
    float interval_x = static_cast<float>(pViewport.w) / width;
    float interval_y = static_cast<float>(pViewport.h) / height;

    RgbaImage pPixels = RgbaImage::solid(Color(), width, height);

    if (uCurrentlyLoadedLevelType != LEVEL_NULL) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                pPixels[y][x] = sPixels[outputRender.h - (y + 1) * interval_y - pViewport.y][x * interval_x + pViewport.x];
            }
        }
    }

    return pPixels;
}

RgbaImage OpenGLRenderer::MakeFullScreenshot() {
    return flipVertically(ReadScreenPixels());
}

void OpenGLRenderer::BeginDecals() {
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // gen buffers

    if (!_decalBuffer) {
        _decalBuffer.reset(GL_DYNAMIC_DRAW,
            &DecalVertex::pos,
            &DecalVertex::texuv,
            &DecalVertex::texunit,
            &DecalVertex::color);
    }

    _decalVertices.clear();
}

void OpenGLRenderer::EndDecals() {
    if (_decalVertices.empty()) return;

    _decalBuffer.update(_decalVertices);

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    decalshader.use();

    DecalUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.fogColor = fog;
    uniforms.fogStart = fogstart;
    uniforms.fogMiddle = fogmiddle;
    uniforms.fogEnd = fogend;
    uniforms.submit(decalshader);

    glActiveTexture(GL_TEXTURE0);

    GraphicsImage *texture = assets->getBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->renderId().value());

    _decalBuffer.bind();
    glDrawArrays(GL_TRIANGLES, 0, _decalVertices.size());
    drawcalls++;

    // unload
    decalshader.unuse();
    _decalBuffer.unbind();
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
        Colorf uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[0].vWorldViewPosition.x, 0, nullptr).toColorf();

        float uFinalR = uTint.r * color_mult * decalColorMult.r;
        float uFinalG = uTint.g * color_mult * decalColorMult.g;
        float uFinalB = uTint.b * color_mult * decalColorMult.b;

        // copy first
        DecalVertex &v0 = _decalVertices.emplace_back();
        v0.pos = pDecal->pVertices[0].vWorldPosition;
        v0.texuv = Vec2f(pDecal->pVertices[0].u, pDecal->pVertices[0].v);
        v0.texunit = 0;
        v0.color = Colorf(uFinalR, uFinalG, uFinalB, 1.0f);

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            uTint = GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[z + i].vWorldViewPosition.x, 0, nullptr).toColorf();
            uFinalR = uTint.r * color_mult * decalColorMult.r;
            uFinalG = uTint.g * color_mult * decalColorMult.g;
            uFinalB = uTint.b * color_mult * decalColorMult.b;

            DecalVertex &v = _decalVertices.emplace_back();
            v.pos = pDecal->pVertices[z + i].vWorldPosition;
            v.texuv = Vec2f(pDecal->pVertices[z + i].u, pDecal->pVertices[z + i].v);
            v.texunit = 0;
            v.color = Colorf(uFinalR, uFinalG, uFinalB, 1.0f);
        }
    }
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

    // Texture ids will be released after swapBuffers(). We might have the passed id saved in the render lists, so
    // can't release it yet.
    _texturesForDeletion.push_back(id.value());
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
        glViewport(pViewport.x, outputRender.h - (pViewport.y + pViewport.h - 1) - 1, pViewport.w, pViewport.h);
        projmat = glm::ortho(float(pViewport.x), float(pViewport.x + pViewport.w - 1), float(pViewport.y + pViewport.h - 1), float(pViewport.y), float(1), float(-1));
    }
}

// TODO(pskelton): to camera?
void OpenGLRenderer::_set_ortho_modelview() {
    // load identity matrix
    viewmat = glm::mat4x4(1);
}


// ---------------------- terrain -----------------------
void OpenGLRenderer::DrawOutdoorTerrain() {
    _initWaterTiles();
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
    if (!_terrainBuffer) {
        static RenderVertexSoft pTerrainVertices[128 * 128];

        // generate vertex locations
        for (int y = 0; y < 128; ++y)
            for (int x = 0; x < 128; ++x)
                pTerrainVertices[y * 128 + x].vWorldPosition = pOutdoor->pTerrain.vertexByGridUnsafe({x, y}).toFloat();

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
                const auto &tile = pOutdoor->pTerrain.tileDataByGrid({x, y});
                int tileunit = 0;
                int tilelayer = 0;
                bool isWater = (tile.textureName == "wtrtyl");

                // check if tile->name is already in list
                auto mapiter = terraintexmap.find(tile.textureName);
                if (mapiter != terraintexmap.end()) {
                    // if so, extract unit and layer
                    int unitlayer = mapiter->second;
                    tilelayer = unitlayer & 0xFF;
                    tileunit = (unitlayer & 0xFF00) >> 8;
                } else if (isWater) {
                    // water tile
                    tilelayer = 0;
                } else {
                    // else need to add it
                    auto thistexture = assets->getBitmap(tile.textureName, tile.flags & TILE_GENERATED_TRANSITION);
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
                            terraintexmap.insert(std::make_pair(tile.textureName, encode));
                            numterraintexloaded[i]++;
                        } else {
                            logger->warning("Texture layer full - draw terrain!");
                            tileunit = 0;
                            tilelayer = 0;
                        }
                    }
                }

                // next calculate all vertices vertices
                const auto &[norm, norm2] = pOutdoor->pTerrain.normalsByGridUnsafe({x, y});

                GLfloat waterAttrib = isWater ? 0x1 : 0;

                // calc each vertex
                // [0] - x,y        n1
                _terrainVertices[6 * (x + (127 * y))].pos = pTerrainVertices[y * 128 + x].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y))].texuv = Vec2f(0, 0);
                _terrainVertices[6 * (x + (127 * y))].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y))].normal = norm;
                _terrainVertices[6 * (x + (127 * y))].attribs = waterAttrib;

                // [1] - x+1,y+1    n1
                _terrainVertices[6 * (x + (127 * y)) + 1].pos = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y)) + 1].texuv = Vec2f(1, 1);
                _terrainVertices[6 * (x + (127 * y)) + 1].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y)) + 1].normal = norm;
                _terrainVertices[6 * (x + (127 * y)) + 1].attribs = waterAttrib;

                // [2] - x+1,y      n1
                _terrainVertices[6 * (x + (127 * y)) + 2].pos = pTerrainVertices[y * 128 + x + 1].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y)) + 2].texuv = Vec2f(1, 0);
                _terrainVertices[6 * (x + (127 * y)) + 2].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y)) + 2].normal = norm;
                _terrainVertices[6 * (x + (127 * y)) + 2].attribs = waterAttrib;

                // [3] - x,y        n2
                _terrainVertices[6 * (x + (127 * y)) + 3].pos = pTerrainVertices[y * 128 + x].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y)) + 3].texuv = Vec2f(0, 0);
                _terrainVertices[6 * (x + (127 * y)) + 3].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y)) + 3].normal = norm2;
                _terrainVertices[6 * (x + (127 * y)) + 3].attribs = waterAttrib;

                // [4] - x,y+1      n2
                _terrainVertices[6 * (x + (127 * y)) + 4].pos = pTerrainVertices[(y + 1) * 128 + x].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y)) + 4].texuv = Vec2f(0, 1);
                _terrainVertices[6 * (x + (127 * y)) + 4].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y)) + 4].normal = norm2;
                _terrainVertices[6 * (x + (127 * y)) + 4].attribs = waterAttrib;

                // [5] - x+1,y+1    n2
                _terrainVertices[6 * (x + (127 * y)) + 5].pos = pTerrainVertices[(y + 1) * 128 + x + 1].vWorldPosition;
                _terrainVertices[6 * (x + (127 * y)) + 5].texuv = Vec2f(1, 1);
                _terrainVertices[6 * (x + (127 * y)) + 5].texturelayer = tilelayer;
                _terrainVertices[6 * (x + (127 * y)) + 5].normal = norm2;
                _terrainVertices[6 * (x + (127 * y)) + 5].attribs = waterAttrib;
            }
        }

        // generate VAO/VBO
        _terrainBuffer.reset(GL_STATIC_DRAW,
            &ShaderVertex::pos, &ShaderVertex::texuv, &ShaderVertex::texturelayer,
            &ShaderVertex::normal, &ShaderVertex::attribs);
        _terrainBuffer.update(_terrainVertices);

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
                    auto texture = assets->getBitmap(it->first, it->first.starts_with("generated")); // TODO(captainurist): terrible, terrible hack, redo this.
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
    if (config->debug.Terrain.value())
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
    _terrainBuffer.bind();

    // use the terrain shader
    terrainshader.use();

    // set base uniforms
    TerrainUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.cameraPos.x = pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
    uniforms.cameraPos.y = pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
    uniforms.cameraPos.z = pParty->pos.z + pParty->eyeLevel;
    uniforms.fogColor = fog;
    uniforms.fogStart = fogstart;
    uniforms.fogMiddle = fogmiddle;
    uniforms.fogEnd = fogend;
    uniforms.gamma = gamma;
    uniforms.waterframe = waterAnimationFrame();

    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0) * 2 * pi_double) / 1440) + 1) * 0.27;
    float diffuseon = pWeather->bNight ? 0 : 1;

    uniforms.sun.direction = pOutdoor->vSunlight;
    uniforms.sun.ambient = Colorf(ambient, ambient, ambient);
    uniforms.sun.diffuse = Colorf(diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3), diffuseon * (ambient + 0.3));
    uniforms.sun.specular = Colorf(0.0f, 0.0f, 0.0f);

    // red colouring
    if (pParty->armageddon_timer) {
        uniforms.sun.ambient = Colorf(1.0f, 0.0f, 0.0f);
        uniforms.sun.diffuse = Colorf(1.0f, 0.0f, 0.0f);
        uniforms.sun.specular = Colorf(0.0f, 0.0f, 0.0f);
    }

    // Sea colouring
    if (engine->IsUnderwater()) {
        Colorf sea = colorTable.Eucalyptus.toColorf();
        uniforms.sun.ambient = Colorf(sea.r * ambient, sea.g * ambient, sea.b * ambient);
        uniforms.sun.diffuse = Colorf(sea.r * (ambient + 0.3), sea.g * (ambient + 0.3), sea.b * (ambient + 0.3));
    }

    // TODO(pskelton): this should be a separate function
    // rest of lights stacking
    int num_lights = 0;

    // get party torchlight as priority - can be radius == 0
    if (pMobileLightsStack->uNumLightsActive >= 1) {
        MobileLight &test = pMobileLightsStack->pLights[0];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 2.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = Colorf(0.0f, 0.0f, 0.0f);
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        StationaryLight &test = pStationaryLightsStack->pLights[i];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 1.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = color;
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        MobileLight &test = pMobileLightsStack->pLights[i];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 2.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = color;
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    // remaining lights are default-initialized (type = 0)
    uniforms.submit(terrainshader);

    // actually draw the whole terrain
    glDrawArrays(GL_TRIANGLES, 0, (127 * 127 * 6));
    drawcalls++;

    // unload
    terrainshader.unuse();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //end terrain debug
    if (config->debug.Terrain.value())
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
        Vec2i gridPos = worldToGrid(decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos);
        // use terrain squares in block surrounding to try and stack faces

        int scope = std::ceil(decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].radius / 512);

        for (int loopy = (gridPos.y - scope); loopy <= (gridPos.y + scope); ++loopy) {
            for (int loopx = (gridPos.x - scope); loopx <= (gridPos.x + scope); ++loopx) {
                if (loopy < 0) continue;
                if (loopy > 127) continue;
                if (loopx < 0) continue;
                if (loopx > 127) continue;


                // top tri
                // x, y
                VertexRenderList[0].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy))].pos;
                // x + 1, y + 1
                VertexRenderList[1].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy)) + 1].pos;
                // x + 1, y
                VertexRenderList[2].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy)) + 2].pos;

                // bottom tri
                // x, y
                VertexRenderList[3].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy)) + 3].pos;
                // x, y + 1
                VertexRenderList[4].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy)) + 4].pos;
                // x + 1, y + 1
                VertexRenderList[5].vWorldPosition = _terrainVertices[6 * (loopx + (127 * loopy)) + 5].pos;

                float WorldMinZ = pOutdoor->GetPolygonMinZ(VertexRenderList, 6);
                float WorldMaxZ = pOutdoor->GetPolygonMaxZ(VertexRenderList, 6);

                // TODO(pskelton): terrain and boxes should be saved for easier retrieval
                // test expanded box against bloodsplat
                BBoxf thissquare{ _terrainVertices[6 * (loopx + (127 * loopy))].pos.x ,
                                  _terrainVertices[6 * (loopx + (127 * loopy)) + 1].pos.x,
                                  _terrainVertices[6 * (loopx + (127 * loopy)) + 1].pos.y,
                                  _terrainVertices[6 * (loopx + (127 * loopy))].pos.y,
                                  WorldMinZ,
                                  WorldMaxZ };

                // skip this square if no splat over lap
                if (!thissquare.intersectsCube(decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].pos, decal_builder->bloodsplat_container->pBloodsplats_to_apply[i].radius))
                    continue;

                // splat hits this square of terrain
                bool fading = pOutdoor->pTerrain.isWaterOrShoreByGrid({loopx, loopy});

                const auto &[norm, norm2] = pOutdoor->pTerrain.normalsByGridUnsafe({loopx, loopy});

                float Light_tile_dist = 0.0;

                // top tri
                float _f1 = norm.x * pOutdoor->vSunlight.x + norm.y * pOutdoor->vSunlight.y + norm.z * pOutdoor->vSunlight.z;
                int dimming_level = std::clamp(static_cast<int>(20.0f - floorf(20.0f * _f1 + 0.5f)), 0, 31);

                decal_builder->ApplyBloodSplatToTerrain(fading, norm, &Light_tile_dist, VertexRenderList, i);
                Planef plane;
                plane.normal = norm;
                plane.dist = Light_tile_dist;
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - dimming_level, LocationTerrain, plane, 3, VertexRenderList, 0, -1);

                //bottom tri
                float _f = norm2.x * pOutdoor->vSunlight.x + norm2.y * pOutdoor->vSunlight.y + norm2.z * pOutdoor->vSunlight.z;
                dimming_level = std::clamp(static_cast<int>(20.0 - floorf(20.0 * _f + 0.5f)), 0, 31);

                decal_builder->ApplyBloodSplatToTerrain(fading, norm2, &Light_tile_dist, (VertexRenderList + 3), i);
                plane.normal = norm2;
                plane.dist = Light_tile_dist;
                if (decal_builder->uNumSplatsThisFace > 0)
                    decal_builder->BuildAndApplyDecals(31 - dimming_level, LocationTerrain, plane, 3, (VertexRenderList + 3), 0, -1);
            }
        }
    }

    // end of new system test
    return;

    // end shder version
}

// TODO(pskelton): renderbase
void OpenGLRenderer::DrawOutdoorSky() {
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  horizon_height_offset = ((double)(pCamera3D->ViewPlaneDistPixels * pCamera3D->vCameraPos.z)
        / ((double)pCamera3D->ViewPlaneDistPixels + pCamera3D->GetFarClip())
        + (double)(pViewport.center().y));

    float depth_to_far_clip = std::cos((double)pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = std::sin((double)pCamera3D->_viewPitch * rot_to_rads) * pCamera3D->GetFarClip();

    float bot_y_proj = ((double)(pViewport.center().y) -
        (double)pCamera3D->ViewPlaneDistPixels /
        (depth_to_far_clip + 0.0000001) *
        (height_to_far_clip - (double)pCamera3D->vCameraPos.z));

    // if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
    // pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
    // else
    // pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
    // pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ?
    // (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);

    if (!pOutdoor->sky_texture)
        pOutdoor->sky_texture = assets->getBitmap("plansky3"); // TODO(pskelton): do we need this?

    if (pOutdoor->sky_texture) {
        int dimming_level = (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)? 31 : 0;
        int uNumVertices = 4;

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
        VertexRenderList[0].vWorldViewProj.x = (double)pViewport.x;  // 8
        VertexRenderList[0].vWorldViewProj.y = (double)pViewport.y;  // 8

        VertexRenderList[1].vWorldViewProj.x = (double)pViewport.x;   // 8
        VertexRenderList[1].vWorldViewProj.y = (double)bot_y_proj + 1;  // 247

        VertexRenderList[2].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);   // 468
        VertexRenderList[2].vWorldViewProj.y = (double)bot_y_proj + 1;  // 247

        VertexRenderList[3].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);  // 468
        VertexRenderList[3].vWorldViewProj.y = (double)pViewport.y;  // 8

        float widthperpixel = 1 / pCamera3D->ViewPlaneDistPixels;

        for (unsigned i = 0; i < uNumVertices; ++i) {
            // outbound screen X dist
            float x_dist = widthperpixel * (pViewport.center().x - VertexRenderList[i].vWorldViewProj.x);
            // outbound screen y dist
            float y_dist = widthperpixel * (horizon_height_offset - VertexRenderList[i].vWorldViewProj.y);

            // rotate vectors to cam facing
            float skyfinalleft = (SkyBillboard.CamVecLeft_X * x_dist) + (SkyBillboard.CamVecLeft_Z * y_dist) + SkyBillboard.CamVecLeft_Y;
            float skyfinalfront = (SkyBillboard.CamVecFront_X * x_dist) + (SkyBillboard.CamVecFront_Z * y_dist) + SkyBillboard.CamVecFront_Y;

            // pitch rotate sky to get top
            float top_y_proj = v18x + v18y + v18z * y_dist;
            if (top_y_proj > 0.0f) top_y_proj = -0.0000001f;

            float worldviewdepth = -64.0 / top_y_proj;
            if (worldviewdepth < 0) worldviewdepth = pCamera3D->GetFarClip();

            // offset tex coords
            float texoffset_U = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalleft * worldviewdepth));
            VertexRenderList[i].u = texoffset_U / ((float) pOutdoor->sky_texture->width());
            float texoffset_V = pMiscTimer->time().realtimeMillisecondsFloat() + ((skyfinalfront * worldviewdepth));
            VertexRenderList[i].v = texoffset_V / ((float) pOutdoor->sky_texture->height());

            VertexRenderList[i].vWorldViewPosition.x = pCamera3D->GetFarClip();

            // this basically acts as texture perspective correction
            VertexRenderList[i]._rhw = (double)(worldviewdepth);
        }

        if (config->graphics.Fog.value()) {
            // fade sky
            VertexRenderList[4].vWorldViewProj.x = (double)pViewport.x;
            VertexRenderList[4].vWorldViewProj.y = (double)pViewport.y;
            VertexRenderList[5].vWorldViewProj.x = (double)pViewport.x;
            VertexRenderList[5].vWorldViewProj.y = (double)bot_y_proj - config->graphics.FogHorizon.value();
            VertexRenderList[6].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);
            VertexRenderList[6].vWorldViewProj.y = (double)bot_y_proj - config->graphics.FogHorizon.value();
            VertexRenderList[7].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);
            VertexRenderList[7].vWorldViewProj.y = (double)pViewport.y;

            // sub sky
            VertexRenderList[8].vWorldViewProj.x = (double)pViewport.x;
            VertexRenderList[8].vWorldViewProj.y = (double)bot_y_proj - config->graphics.FogHorizon.value();
            VertexRenderList[9].vWorldViewProj.x = (double)pViewport.x;
            VertexRenderList[9].vWorldViewProj.y = (double)(pViewport.y + pViewport.h - 1) + 1;
            VertexRenderList[10].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);
            VertexRenderList[10].vWorldViewProj.y = (double)(pViewport.y + pViewport.h - 1) + 1;
            VertexRenderList[11].vWorldViewProj.x = (double)(pViewport.x + pViewport.w - 1);
            VertexRenderList[11].vWorldViewProj.y = (double)bot_y_proj - config->graphics.FogHorizon.value();
        }

        _set_ortho_projection(1);
        _set_ortho_modelview();
        DrawOutdoorSkyPolygon(uNumVertices, pOutdoor->sky_texture, dimming_level);
    }
}



//----- (004A2DA3) --------------------------------------------------------
void OpenGLRenderer::DrawOutdoorSkyPolygon(int numVertices, GraphicsImage *texture, int dimmingLevel) {
    auto texid = texture->renderId().value();

    float texidsolid = static_cast<float>(solidFillTexture()->renderId().value());

    //glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    Colorf uTint = GetActorTintColor(dimmingLevel, 0, VertexRenderList[0].vWorldViewPosition.x, 1, 0).toColorf();
    float scrspace{ pCamera3D->GetFarClip() };

    // load up poly
    for (int z = 0; z < (numVertices - 2); z++) {
        // 123, 134, 145, 156..
        // copy first
        ForcePerVertex &v0 = _forcePerVertices.emplace_back();
        v0.pos = Vec3f(VertexRenderList[0].vWorldViewProj.x, VertexRenderList[0].vWorldViewProj.y, 1.0f);
        v0.w = VertexRenderList[0]._rhw;
        v0.texuv = Vec2f(VertexRenderList[0].u, VertexRenderList[0].v);
        v0.texw = 1.0f;
        v0.screenspace = scrspace;
        v0.color = uTint;
        v0.texid = texid;

        // copy other two (z+1)(z+2)
        for (unsigned i = 1; i < 3; ++i) {
            ForcePerVertex &v = _forcePerVertices.emplace_back();
            v.pos = Vec3f(VertexRenderList[z + i].vWorldViewProj.x, VertexRenderList[z + i].vWorldViewProj.y, 1.0f);
            v.w = VertexRenderList[z + i]._rhw;
            v.texuv = Vec2f(VertexRenderList[z + i].u, VertexRenderList[z + i].v);
            v.texw = 1.0f;
            v.screenspace = scrspace;
            v.color = uTint;
            v.texid = texid;
        }
    }

    if (config->graphics.Fog.value()) {
        // draw blend sky
        // load up poly
        for (int z = 4; z < 6; z++) {
            // 456, 467..
            // copy first
            ForcePerVertex &v0 = _forcePerVertices.emplace_back();
            v0.pos = Vec3f(VertexRenderList[4].vWorldViewProj.x, VertexRenderList[4].vWorldViewProj.y, 1.0f);
            v0.w = 1.0f;
            v0.texuv = Vec2f(0.5f, 0.5f);
            v0.texw = 1.0f;
            v0.screenspace = scrspace;
            v0.color = uTint;
            v0.color.a = 0;
            v0.texid = texidsolid;

            // copy other two (z+1)(z+2)
            for (unsigned i = 1; i < 3; ++i) {
                ForcePerVertex &v = _forcePerVertices.emplace_back();
                v.pos = Vec3f(VertexRenderList[z + i].vWorldViewProj.x, VertexRenderList[z + i].vWorldViewProj.y, 1.0f);
                v.w = 1.0f;
                v.texuv = Vec2f(0.5f, 0.5f);
                v.texw = 1.0f;
                v.screenspace = scrspace;
                v.color = uTint;
                v.color.a = ((z + i) == 7) ? 0.0f : 1.0f;
                v.texid = texidsolid;
            }
        }

        // draw sub sky
        // load up poly
        for (int z = 8; z < 10; z++) {
            // 456, 467..
            // copy first
            ForcePerVertex &v0 = _forcePerVertices.emplace_back();
            v0.pos = Vec3f(VertexRenderList[8].vWorldViewProj.x, VertexRenderList[8].vWorldViewProj.y, 1.0f);
            v0.w = 1.0f;
            v0.texuv = Vec2f(0.5f, 0.5f);
            v0.texw = 1.0f;
            v0.screenspace = scrspace;
            v0.color = uTint;
            v0.texid = texidsolid;

            // copy other two (z+1)(z+2)
            for (unsigned i = 1; i < 3; ++i) {
                ForcePerVertex &v = _forcePerVertices.emplace_back();
                v.pos = Vec3f(VertexRenderList[z + i].vWorldViewProj.x, VertexRenderList[z + i].vWorldViewProj.y, 1.0f);
                v.w = 1.0f;
                v.texuv = Vec2f(0.5f, 0.5f);
                v.texw = 1.0f;
                v.screenspace = scrspace;
                v.color = uTint;
                v.texid = texidsolid;
            }
        }
    }

    DrawForcePerVerts();
}

void OpenGLRenderer::DrawForcePerVerts() {
    if (_forcePerVertices.empty()) return;

    if (!_forcePerBuffer) {
        _forcePerBuffer.reset(GL_DYNAMIC_DRAW,
            &ForcePerVertex::pos,
            &ForcePerVertex::w,
            &ForcePerVertex::texuv,
            &ForcePerVertex::texw,
            &ForcePerVertex::screenspace,
            &ForcePerVertex::color);
    }

    _forcePerBuffer.update(_forcePerVertices);
    _forcePerBuffer.bind();

    forcepershader.use();

    // set fog - force perspective is handled slightly differently because of the sky
    ForcePerUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;

    Color fpfogcol = GetLevelFogColor();
    if (config->graphics.Fog.value() && uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        if (fpfogcol != Color()) {
            uniforms.fogStart = day_fogrange_1;
            uniforms.fogMiddle = day_fogrange_2;
            uniforms.fogEnd = day_fogrange_3;
            uniforms.fogColor = Colorf(fpfogcol.r / 255.0f, fpfogcol.g / 255.0f, fpfogcol.b / 255.0f);
        } else {
            uniforms.fogStart = pCamera3D->GetFarClip();
            uniforms.fogMiddle = 0.0f;
            uniforms.fogEnd = uniforms.fogStart + 1;
            float fogVal = _forcePerVertices[0].color.r;
            uniforms.fogColor = Colorf(fogVal, fogVal, fogVal);
        }
    } else {
        uniforms.fogStart = pCamera3D->GetFarClip();
        uniforms.fogMiddle = 0.0f;
        uniforms.fogEnd = uniforms.fogStart;
    }

    uniforms.submit(forcepershader);

    // draw all similar textures in batches
    size_t offset = 0;
    while (offset < _forcePerVertices.size()) {
        // set texture
        GLfloat thistex = _forcePerVertices[offset].texid;
        glBindTexture(GL_TEXTURE_2D, thistex);

        size_t cnt = 0;
        do {
            cnt++;
            if (offset + (3 * cnt) >= _forcePerVertices.size()) {
                break;
            }
        } while (_forcePerVertices[offset + (cnt * 3)].texid == thistex);

        glDrawArrays(GL_TRIANGLES, offset, (3 * cnt));
        drawcalls++;

        offset += (3 * cnt);
    }

    forcepershader.unuse();
    _forcePerBuffer.unbind();

    _forcePerVertices.clear();
}

// TODO(pskelton): move ?
void OpenGLRenderer::SetFogParametersGL() {
    Color fogcol = GetLevelFogColor();

    if (config->graphics.Fog.value() && uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        if (fogcol != Color()) {
            fogstart = day_fogrange_1;
            fogmiddle = day_fogrange_2;
            fogend = day_fogrange_3;
            fog.r = fogcol.r / 255.0f;
            fog.g = fogcol.g / 255.0f;
            fog.b = fogcol.b / 255.0f;
        } else {
            fogend = pCamera3D->GetFarClip();
            fogmiddle = 0.0f;
            fogstart = fogend * config->graphics.FogDepthRatio.value();

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

//----- (004A1C1E) --------------------------------------------------------
void OpenGLRenderer::DoRenderBillboards_D3D() {
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);  // in theory billboards all sorted by depth so dont cull by depth test
    glDisable(GL_CULL_FACE);  // some quads are reversed to reuse sprites opposite hand

    _set_ortho_projection(1);
    _set_ortho_modelview();

    if (!_billboardVertices.empty())
        logger->trace("Billboard shader store isnt empty!");

    // track loaded tex
    float gltexid{ 0 };
    // track blend mode
    //RenderBillboardD3D::OpacityType blendtrack{ RenderBillboardD3D::NoBlend };

    float oneon = 1.0f / (pCamera3D->GetNearClip() * 2.0f);
    float oneof = 1.0f / (pCamera3D->GetFarClip());

    SortBillboards();

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i) {
        //if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend) {
        //    if (blendtrack != pBillboardRenderListD3D[i].opacity) {
        //        blendtrack = pBillboardRenderListD3D[i].opacity;
        //        SetBillboardBlendOptions(blendtrack);
        //    }
        //}

        RenderBillboardD3D* billboard = pSortedBillboardRenderListD3D[i];

        //int palette{ pBillboardRenderListD3D[i].PaletteID};
        int paletteId = billboard->paletteId;

        if (billboard->texture) {
            auto texture = billboard->texture;
            gltexid = texture->renderId().value();
        } else {
            static GraphicsImage *effpar03 = assets->getBitmap("effpar03");
            gltexid = static_cast<float>(effpar03->renderId().value());
        }

        //if (gltexid != testtexid) {
        //    gltexid = testtexid;
        //    glBindTexture(GL_TEXTURE_2D, gltexid);
        //}


        float oneoz = 1.0f / billboard->view_space_z;
        float thisdepth = (oneoz - oneon) / (oneof - oneon);

        float thisblend = static_cast<float>(billboard->opacity);

        // Triangle 1: vertices 0, 1, 2
        BillboardVertex &v0 = _billboardVertices.emplace_back();
        v0.pos = Vec3f(billboard->pQuads[0].pos.x, billboard->pQuads[0].pos.y, thisdepth);
        v0.texuv = Vec2f(std::clamp(billboard->pQuads[0].texcoord.x, 0.01f, 0.99f),
                         std::clamp(billboard->pQuads[0].texcoord.y, 0.01f, 0.99f));
        v0.color = billboard->pQuads[0].diffuse.toColorf();
        v0.screenspace = billboard->view_space_L2;
        v0.texid = gltexid;
        v0.blend = thisblend;
        v0.paletteId = paletteId;

        BillboardVertex &v1 = _billboardVertices.emplace_back();
        v1.pos = Vec3f(billboard->pQuads[1].pos.x, billboard->pQuads[1].pos.y, thisdepth);
        v1.texuv = Vec2f(std::clamp(billboard->pQuads[1].texcoord.x, 0.01f, 0.99f),
                         std::clamp(billboard->pQuads[1].texcoord.y, 0.01f, 0.99f));
        v1.color = billboard->pQuads[1].diffuse.toColorf();
        v1.screenspace = billboard->view_space_L2;
        v1.texid = gltexid;
        v1.blend = thisblend;
        v1.paletteId = paletteId;

        BillboardVertex &v2 = _billboardVertices.emplace_back();
        v2.pos = Vec3f(billboard->pQuads[2].pos.x, billboard->pQuads[2].pos.y, thisdepth);
        v2.texuv = Vec2f(std::clamp(billboard->pQuads[2].texcoord.x, 0.01f, 0.99f),
                         std::clamp(billboard->pQuads[2].texcoord.y, 0.01f, 0.99f));
        v2.color = billboard->pQuads[2].diffuse.toColorf();
        v2.screenspace = billboard->view_space_L2;
        v2.texid = gltexid;
        v2.blend = thisblend;
        v2.paletteId = paletteId;

        // Triangle 2: vertices 0, 2, 3 (if quad has 4 vertices)
        if (billboard->pQuads[3].pos.x != 0.0f && billboard->pQuads[3].pos.y != 0.0f && billboard->pQuads[3].pos.z != 0.0f) {
            BillboardVertex &v3 = _billboardVertices.emplace_back();
            v3.pos = Vec3f(billboard->pQuads[0].pos.x, billboard->pQuads[0].pos.y, thisdepth);
            v3.texuv = Vec2f(std::clamp(billboard->pQuads[0].texcoord.x, 0.01f, 0.99f),
                             std::clamp(billboard->pQuads[0].texcoord.y, 0.01f, 0.99f));
            v3.color = billboard->pQuads[0].diffuse.toColorf();
            v3.screenspace = billboard->view_space_L2;
            v3.texid = gltexid;
            v3.blend = thisblend;
            v3.paletteId = paletteId;

            BillboardVertex &v4 = _billboardVertices.emplace_back();
            v4.pos = Vec3f(billboard->pQuads[2].pos.x, billboard->pQuads[2].pos.y, thisdepth);
            v4.texuv = Vec2f(std::clamp(billboard->pQuads[2].texcoord.x, 0.01f, 0.99f),
                             std::clamp(billboard->pQuads[2].texcoord.y, 0.01f, 0.99f));
            v4.color = billboard->pQuads[2].diffuse.toColorf();
            v4.screenspace = billboard->view_space_L2;
            v4.texid = gltexid;
            v4.blend = thisblend;
            v4.paletteId = paletteId;

            BillboardVertex &v5 = _billboardVertices.emplace_back();
            v5.pos = Vec3f(billboard->pQuads[3].pos.x, billboard->pQuads[3].pos.y, thisdepth);
            v5.texuv = Vec2f(std::clamp(billboard->pQuads[3].texcoord.x, 0.01f, 0.99f),
                             std::clamp(billboard->pQuads[3].texcoord.y, 0.01f, 0.99f));
            v5.color = billboard->pQuads[3].diffuse.toColorf();
            v5.screenspace = billboard->view_space_L2;
            v5.texid = gltexid;
            v5.blend = thisblend;
            v5.paletteId = paletteId;
        }
    }

    DrawBillboards();

    //glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

// name better
void OpenGLRenderer::DrawBillboards() {
    if (_billboardVertices.empty()) return;

    if (!_billboardBuffer) {
        _billboardBuffer.reset(GL_DYNAMIC_DRAW,
            &BillboardVertex::pos,
            &BillboardVertex::texuv,
            &BillboardVertex::color,
            &BillboardVertex::screenspace,
            &BillboardVertex::texid,
            &BillboardVertex::paletteId);
    }

    constexpr GLint paltex2D_id = 1;
    if (paltex2D == 0) {
        std::span<Color> palettes = pPaletteManager->paletteData();
        glActiveTexture(GL_TEXTURE0 + paltex2D_id);
        glGenTextures(1, &paltex2D);
        glBindTexture(GL_TEXTURE_2D, paltex2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, palettes.size() / 256, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, palettes.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    _billboardBuffer.update(_billboardVertices);
    _billboardBuffer.bind();

    billbshader.use();

    // set sampler to palette
    glActiveTexture(GL_TEXTURE0 + paltex2D_id);
    glBindTexture(GL_TEXTURE_2D, paltex2D);

    glActiveTexture(GL_TEXTURE0);

    BillboardUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.fogColor = fog;
    uniforms.fogStart = fogstart;
    uniforms.fogMiddle = fogmiddle;
    uniforms.fogEnd = fogend;
    uniforms.gamma = gamma;
    uniforms.paltex2D = paltex2D_id;
    uniforms.submit(billbshader);

    size_t offset = 0;
    while (offset < _billboardVertices.size()) {
        // set texture
        GLfloat thistex = _billboardVertices[offset].texid;
        glBindTexture(GL_TEXTURE_2D, _billboardVertices[offset].texid);
        if (_billboardVertices[offset].paletteId) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        GLfloat thisblend = _billboardVertices[offset].blend;
        if (thisblend == 0.0) {
            // disable alpha blending and enable fog for opaque items
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform1f(billbshader.uniformLocation("fog.fogstart"), GLfloat(fogstart));
        } else {
            // enable blending and disable fog for transparent items
            glBlendFunc(GL_ONE, GL_ONE);
            glUniform1f(billbshader.uniformLocation("fog.fogstart"), GLfloat(fogend));
        }


        size_t cnt = 0;
        do {
            cnt++;
            if (offset + (3 * cnt) >= _billboardVertices.size()) {
                break;
            }
        } while (_billboardVertices[offset + (cnt * 3)].texid == thistex && _billboardVertices[offset + (cnt * 3)].blend == thisblend);

        glDrawArrays(GL_TRIANGLES, offset, (3 * cnt));
        drawcalls++;

        offset += (3 * cnt);
    }

    billbshader.unuse();
    _billboardBuffer.unbind();
    _billboardVertices.clear();
}

//----- (004A1DA8) --------------------------------------------------------
void OpenGLRenderer::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1) {
    return;
}

void OpenGLRenderer::SetUIClipRect(const Recti &rect) {
    this->clipRect = rect;
    glScissor(rect.x, outputRender.h - rect.y - rect.h, rect.w, rect.h);  // invert glscissor co-ords 0,0 is BL
}

void OpenGLRenderer::ResetUIClipRect() {
    this->SetUIClipRect(Recti(Pointi(0, 0), outputRender));
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

void OpenGLRenderer::DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, const Recti &dstRect, Color color) {
    if (!texture) {
        logger->trace("Null texture passed to DrawQuad2D");
        return;
    }

    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, texture->name());

    // Early out if destination is completely outside render area.
    if (dstRect.x >= outputRender.w || dstRect.y >= outputRender.h)
        return;

    // Clip destination rect against clip rect.
    Recti clippedDst = dstRect & this->clipRect;
    if (clippedDst.isEmpty())
        return;

    // Calculate how much was clipped from each side as a fraction of original size.
    float leftClip = (clippedDst.x - dstRect.x) / float(dstRect.w);
    float topClip = (clippedDst.y - dstRect.y) / float(dstRect.h);
    float rightClip = (clippedDst.x + clippedDst.w - dstRect.x) / float(dstRect.w);
    float bottomClip = (clippedDst.y + clippedDst.h - dstRect.y) / float(dstRect.h);

    // Convert srcRect to UV coordinates and apply proportional clipping.
    float texWidth = texture->width();
    float texHeight = texture->height();
    float srcU1 = srcRect.x / texWidth;
    float srcV1 = srcRect.y / texHeight;
    float srcU2 = (srcRect.x + srcRect.w) / texWidth;
    float srcV2 = (srcRect.y + srcRect.h) / texHeight;

    // Interpolate UVs based on clipping.
    float u1 = srcU1 + (srcU2 - srcU1) * leftClip;
    float v1 = srcV1 + (srcV2 - srcV1) * topClip;
    float u2 = srcU1 + (srcU2 - srcU1) * rightClip;
    float v2 = srcV1 + (srcV2 - srcV1) * bottomClip;

    Colorf cf = color.toColorf();
    int gltexid = texture->renderId().value();

    float drawx = clippedDst.x;
    float drawy = clippedDst.y;
    float drawz = clippedDst.x + clippedDst.w;
    float draww = clippedDst.y + clippedDst.h;

    // Triangle 1: top-left, top-right, bottom-right
    TwoDVertex &vert0 = _twodVertices.emplace_back();
    vert0.pos = Vec3f(drawx, drawy, 0);
    vert0.texuv = Vec2f(u1, v1);
    vert0.color = cf;
    vert0.texid = gltexid;
    vert0.paletteid = 0;

    TwoDVertex &vert1 = _twodVertices.emplace_back();
    vert1.pos = Vec3f(drawz, drawy, 0);
    vert1.texuv = Vec2f(u2, v1);
    vert1.color = cf;
    vert1.texid = gltexid;
    vert1.paletteid = 0;

    TwoDVertex &vert2 = _twodVertices.emplace_back();
    vert2.pos = Vec3f(drawz, draww, 0);
    vert2.texuv = Vec2f(u2, v2);
    vert2.color = cf;
    vert2.texid = gltexid;
    vert2.paletteid = 0;

    // Triangle 2: top-left, bottom-right, bottom-left
    TwoDVertex &vert3 = _twodVertices.emplace_back();
    vert3.pos = Vec3f(drawx, drawy, 0);
    vert3.texuv = Vec2f(u1, v1);
    vert3.color = cf;
    vert3.texid = gltexid;
    vert3.paletteid = 0;

    TwoDVertex &vert4 = _twodVertices.emplace_back();
    vert4.pos = Vec3f(drawz, draww, 0);
    vert4.texuv = Vec2f(u2, v2);
    vert4.color = cf;
    vert4.texid = gltexid;
    vert4.paletteid = 0;

    TwoDVertex &vert5 = _twodVertices.emplace_back();
    vert5.pos = Vec3f(drawx, draww, 0);
    vert5.texuv = Vec2f(u1, v2);
    vert5.color = cf;
    vert5.texid = gltexid;
    vert5.paletteid = 0;
}

void OpenGLRenderer::BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) {
    // draw any images in buffer
    if (!_twodVertices.empty()) {
        DrawTwodVerts();
    }

    GLuint texmainidcheck = main->renderId().value();

    // if we are changing font draw whats in the text buffer
    if (texmainidcheck != texmain) {
        EndTextNew();
    }

    texmain = main->renderId().value();
    texshadow = shadow->renderId().value();
    _textAtlasSize = main->size();
}

void OpenGLRenderer::EndTextNew() {
    if (_textVertices.empty()) return;

    if (!_twodVertices.empty()) {
        DrawTwodVerts();
    }

    if (!_textBuffer) {
        _textBuffer.reset(GL_DYNAMIC_DRAW,
            &TwoDVertex::pos,
            &TwoDVertex::texuv,
            &TwoDVertex::color,
            &TwoDVertex::texid);
    }

    _textBuffer.update(_textVertices);
    _textBuffer.bind();

    textshader.use();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    TextUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.submit(textshader);

    // set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texmain);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texshadow);

    glDrawArrays(GL_TRIANGLES, 0, _textVertices.size());
    drawcalls++;

    textshader.unuse();
    _textBuffer.unbind();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    _textVertices.clear();
    // texmain = 0;
    // texshadow = 0;
    return;
}

void OpenGLRenderer::DrawTextNew(const Recti &srcRect, const Recti &dstRect, bool isShadow, Color color) {
    Colorf cf = color.toColorf();
    // not 100% sure why this is required but it is
    if (cf.r == 0.0f)
        cf.r = 0.00392f;

    // check bounds
    if (dstRect.x >= outputRender.w || dstRect.y >= outputRender.h)
        return;

    // check for overlap
    if (!dstRect.intersects(this->clipRect))
        return;

    float drawx = static_cast<float>(dstRect.x);
    float drawy = static_cast<float>(dstRect.y);
    float drawz = static_cast<float>(dstRect.x + dstRect.w);
    float draww = static_cast<float>(dstRect.y + dstRect.h);

    float texx = static_cast<float>(srcRect.x) / _textAtlasSize.w;
    float texy = static_cast<float>(srcRect.y) / _textAtlasSize.h;
    float texz = static_cast<float>(srcRect.x + srcRect.w) / _textAtlasSize.w;
    float texw = static_cast<float>(srcRect.y + srcRect.h) / _textAtlasSize.h;

    int texid = isShadow ? 1 : 0;

    // Triangle 1: top-left, top-right, bottom-right
    TwoDVertex &vert0 = _textVertices.emplace_back();
    vert0.pos = Vec3f(drawx, drawy, 0);
    vert0.texuv = Vec2f(texx, texy);
    vert0.color = cf;
    vert0.texid = texid;
    vert0.paletteid = 0;

    TwoDVertex &vert1 = _textVertices.emplace_back();
    vert1.pos = Vec3f(drawz, drawy, 0);
    vert1.texuv = Vec2f(texz, texy);
    vert1.color = cf;
    vert1.texid = texid;
    vert1.paletteid = 0;

    TwoDVertex &vert2 = _textVertices.emplace_back();
    vert2.pos = Vec3f(drawz, draww, 0);
    vert2.texuv = Vec2f(texz, texw);
    vert2.color = cf;
    vert2.texid = texid;
    vert2.paletteid = 0;

    // Triangle 2: top-left, bottom-right, bottom-left
    TwoDVertex &vert3 = _textVertices.emplace_back();
    vert3.pos = Vec3f(drawx, drawy, 0);
    vert3.texuv = Vec2f(texx, texy);
    vert3.color = cf;
    vert3.texid = texid;
    vert3.paletteid = 0;

    TwoDVertex &vert4 = _textVertices.emplace_back();
    vert4.pos = Vec3f(drawz, draww, 0);
    vert4.texuv = Vec2f(texz, texw);
    vert4.color = cf;
    vert4.texid = texid;
    vert4.paletteid = 0;

    TwoDVertex &vert5 = _textVertices.emplace_back();
    vert5.pos = Vec3f(drawx, draww, 0);
    vert5.texuv = Vec2f(texx, texw);
    vert5.color = cf;
    vert5.texid = texid;
    vert5.paletteid = 0;
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

    if (!_texturesForDeletion.empty()) {
        glDeleteTextures(_texturesForDeletion.size(), _texturesForDeletion.data());
        _texturesForDeletion.clear();
    }

    if (config->graphics.FPSLimit.value() > 0)
        _frameLimiter.tick(config->graphics.FPSLimit.value());
}

void OpenGLRenderer::Present() {
    flushAndScale();
    swapBuffers();
}

void OpenGLRenderer::DrawOutdoorBuildings() {
    _initWaterTiles();
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

    if (!_outbuildBuffers[0]) {
        // initialize vertex vectors
        for (int i = 0; i < 16; i++) {
            _outbuildVertices[i].clear();
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
            if (!model.faces.empty()) {
                for (ODMFace &face : model.faces) {
                    if (!face.Invisible()) {
                        // TODO(pskelton): Same as indoors. When ODM and BLV face is combined - seperate out function

                        // TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
                        if (!face.GetTexture()) continue;
                        GraphicsImage *tex = face.GetTexture();

                        std::string texname = tex->name();

                        Duration animLength;
                        Duration frame;
                        if (face.IsAnimated()) {
                            tex = pTextureFrameTable->animationFrame(face.animationId, frame);
                            animLength = pTextureFrameTable->animationLength(face.animationId);
                            texname = tex->name();
                        }
                        // gather up all texture and shaderverts data

                        //auto tile = pOutdoor->getTileDescByGrid(x, y);

                        int texunit = 0;
                        int texlayer = 0;
                        int attribflags = 0;

                        if (face.attributes & FACE_IsFluid)
                            attribflags |= 2;
                        if (face.attributes & FACE_INDOOR_SKY)
                            attribflags |= 0x400;

                        if (face.attributes & FACE_FlowDown)
                            attribflags |= 0x400;
                        else if (face.attributes & FACE_FlowUp)
                            attribflags |= 0x800;

                        if (face.attributes & FACE_FlowRight)
                            attribflags |= 0x2000;
                        else if (face.attributes & FACE_FlowLeft)
                            attribflags |= 0x1000;

                        if (face.attributes & FACE_IsLava)
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

                            if (face.IsAnimated()) {
                                // TODO(pskelton): any instances where animTime is not consistent would need checking
                                frame += pTextureFrameTable->animationFrameLength(face.animationId);
                                tex = pTextureFrameTable->animationFrame(face.animationId, frame);
                                if (!tex) break;
                                texname = tex->name();
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
            _outbuildBuffers[l].reset(GL_DYNAMIC_DRAW,
                &ShaderVertex::pos, &ShaderVertex::texuv, &ShaderVertex::texturelayer,
                &ShaderVertex::normal, &ShaderVertex::attribs);
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
            _outbuildVertices[i].clear();
        }

        for (BSPModel &model : pOutdoor->pBModels) {
            bool reachable;
            if (IsBModelVisible(&model, 256, &reachable)) {
                //if (model.index == 35) continue;
                model.field_40 |= 1;
                if (!model.faces.empty()) {
                    for (ODMFace &face : model.faces) {
                        if (!face.Invisible()) {
                            array_73D150[0].vWorldPosition = model.vertices[face.vertexIds[0]];

                            if (pCamera3D->is_face_faced_to_cameraODM(&face, &array_73D150[0])) {
                                int texunit = 0;
                                int texlayer = 0;

                                if (face.IsAnimated()) {
                                    texlayer = -1;
                                    texunit = -1;
                                } else {
                                    texlayer = face.texlayer;
                                    texunit = face.texunit;
                                }

                                if (texlayer == -1) { // texture has been reset - see if its in the map
                                    GraphicsImage *tex = face.GetTexture();
                                    std::string texname = tex->name();
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

                                if (face.attributes & FACE_IsFluid)
                                    attribflags |= 2;
                                if (face.attributes & FACE_INDOOR_SKY)
                                    attribflags |= 0x400;

                                if (face.attributes & FACE_FlowDown)
                                    attribflags |= 0x400;
                                else if (face.attributes & FACE_FlowUp)
                                    attribflags |= 0x800;

                                if (face.attributes & FACE_FlowRight)
                                    attribflags |= 0x2000;
                                else if (face.attributes & FACE_FlowLeft)
                                    attribflags |= 0x1000;

                                if (face.attributes & FACE_IsLava)
                                    attribflags |= 0x4000;

                                if (face.attributes & FACE_OUTLINED || (face.attributes & FACE_IsSecret) && engine->is_saturate_faces)
                                    attribflags |= 0x00010000;

                                // load up verts here
                                for (int z = 0; z < (face.numVertices - 2); z++) {
                                    // 123, 134, 145, 156..

                                    // copy first
                                    ShaderVertex &v0 = _outbuildVertices[texunit].emplace_back();
                                    v0.pos = model.vertices[face.vertexIds[0]];
                                    v0.texuv = Vec2f(face.textureUs[0] + face.textureDeltaU,
                                                     face.textureVs[0] + face.textureDeltaV);
                                    v0.texturelayer = texlayer;
                                    v0.normal = face.facePlane.normal;
                                    v0.attribs = attribflags;

                                    // copy other two (z+1)(z+2)
                                    for (unsigned i = 1; i < 3; ++i) {
                                        ShaderVertex &v = _outbuildVertices[texunit].emplace_back();
                                        v.pos = model.vertices[face.vertexIds[z + i]];
                                        v.texuv = Vec2f(face.textureUs[z + i] + face.textureDeltaU,
                                                        face.textureVs[z + i] + face.textureDeltaV);
                                        v.texturelayer = texlayer;
                                        v.normal = face.facePlane.normal;
                                        v.attribs = attribflags;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int l = 0; l < 16; l++) {
            if (!_outbuildVertices[l].empty()) {
                _outbuildBuffers[l].update(_outbuildVertices[l]);
            }
        }

    // terrain debug
    if (config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    outbuildshader.use();

    // set base uniforms
    OutBuildUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.cameraPos.x = pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0f);
    uniforms.cameraPos.y = pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0f);
    uniforms.cameraPos.z = pParty->pos.z + pParty->eyeLevel;
    uniforms.fogColor = fog;
    uniforms.fogStart = fogstart;
    uniforms.fogMiddle = fogmiddle;
    uniforms.fogEnd = fogend;
    uniforms.gamma = gamma;
    uniforms.waterframe = waterAnimationFrame();
    uniforms.flowtimer = pMiscTimer->time().realtimeMilliseconds() >> 4;
    uniforms.flowtimerms = pMiscTimer->time().realtimeMilliseconds();

    // sun lighting stuff
    float ambient = pParty->uCurrentMinute + pParty->uCurrentHour * 60.0f;  // 0 - > 1439
    ambient = 0.15 + (sinf(((ambient - 360.0f) * 2 * pi_double) / 1440) + 1) * 0.27f;
    float diffuseon = pWeather->bNight ? 0.0f : 1.0f;

    uniforms.sun.direction = pOutdoor->vSunlight;
    uniforms.sun.ambient = Colorf(ambient, ambient, ambient);
    uniforms.sun.diffuse = Colorf(diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f));
    uniforms.sun.specular = Colorf(diffuseon * 0.35f * ambient, diffuseon * 0.28f * ambient, 0.0f);

    if (pParty->armageddon_timer) {
        uniforms.sun.ambient = Colorf(1.0f, 0.0f, 0.0f);
        uniforms.sun.diffuse = Colorf(1.0f, 0.0f, 0.0f);
        uniforms.sun.specular = Colorf(0.0f, 0.0f, 0.0f);
    }

    // TODO(pskelton): this should be a separate function
    // rest of lights stacking
    int num_lights = 0;

    // get party torchlight as priority - can be radius == 0
    if (pMobileLightsStack->uNumLightsActive >= 1) {
        MobileLight &test = pMobileLightsStack->pLights[0];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 2.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = Colorf(0.0f, 0.0f, 0.0f);
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        StationaryLight &test = pStationaryLightsStack->pLights[i];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 1.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = color;
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    // mobile
    for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
        // maximum 20 lights sent to shader at the moment
        // TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
        if (num_lights >= 20) break;

        MobileLight &test = pMobileLightsStack->pLights[i];
        Colorf color = test.uLightColor.toColorf();

        uniforms.pointLights[num_lights].type = 2.0f;
        uniforms.pointLights[num_lights].position = test.vPosition;
        uniforms.pointLights[num_lights].ambient = color;
        uniforms.pointLights[num_lights].diffuse = color;
        uniforms.pointLights[num_lights].specular = color;
        uniforms.pointLights[num_lights].radius = test.uRadius;
        num_lights++;
    }

    // remaining lights are default-initialized (type = 0)
    uniforms.submit(outbuildshader);

    glActiveTexture(GL_TEXTURE0);

    for (int unit = 0; unit < 16; unit++) {
        // skip if textures are empty
        //if (numoutbuildtexloaded[unit] > 0) {
            if (unit == 1) {
                glUniform1i(outbuildshader.uniformLocation("watertiles"), GLint(0));
            }

            // draw each set of triangles
            glBindTexture(GL_TEXTURE_2D_ARRAY, outbuildtextures[unit]);
            _outbuildBuffers[unit].bind();
            glDrawArrays(GL_TRIANGLES, 0, _outbuildVertices[unit].size());
            drawcalls++;
        //}
    }

    // unload
    outbuildshader.unuse();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //end terrain debug
    if (config->debug.Terrain.value())
        // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
        if (!OpenGLES)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // TODO(pskelton): clean up
    // need to stack decals
    if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;

    for (BSPModel &model : pOutdoor->pBModels) {
        if (model.faces.empty()) {
            continue;
        }

        // check for any splat in this models box - if not continue
        bool found{ false };
        for (int splat = 0; splat < decal_builder->bloodsplat_container->uNumBloodsplats; ++splat) {
            Bloodsplat *thissplat = &decal_builder->bloodsplat_container->pBloodsplats_to_apply[splat];
            if (model.boundingBox.intersectsCube(thissplat->pos, thissplat->radius)) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        for (ODMFace &face : model.faces) {
            if (face.Invisible()) {
                continue;
            }

            float _f1 = face.facePlane.normal.x * pOutdoor->vSunlight.x + face.facePlane.normal.y * pOutdoor->vSunlight.y + face.facePlane.normal.z * pOutdoor->vSunlight.z;
            int dimming_level = std::clamp(static_cast<int>(20.0 - floorf(20.0 * _f1 + 0.5f)), 0, 31);

            for (unsigned vertex_id = 1; vertex_id <= face.numVertices; vertex_id++) {
                array_73D150[vertex_id - 1].vWorldPosition.x =
                    model.vertices[face.vertexIds[vertex_id - 1]].x;
                array_73D150[vertex_id - 1].vWorldPosition.y =
                    model.vertices[face.vertexIds[vertex_id - 1]].y;
                array_73D150[vertex_id - 1].vWorldPosition.z =
                    model.vertices[face.vertexIds[vertex_id - 1]].z;
            }

            for (int vertex_id = 0; vertex_id < face.numVertices; ++vertex_id) {
                memcpy(&VertexRenderList[vertex_id], &array_73D150[vertex_id], sizeof(VertexRenderList[vertex_id]));
                VertexRenderList[vertex_id]._rhw = 1.0 / (array_73D150[vertex_id].vWorldViewPosition.x + 0.0000001);
            }

            decal_builder->ApplyBloodSplat_OutdoorFace(&face);
            if (decal_builder->uNumSplatsThisFace > 0) {
                decal_builder->BuildAndApplyDecals(
                    31 - dimming_level, LocationBuildings,
                    face.facePlane,
                    face.numVertices, VertexRenderList, 0, -1);
            }
        }
    }

    return;



    ///////////////// shader end
}

void OpenGLRenderer::DrawIndoorFaces() {
    _initWaterTiles();
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

        if (!_bspBuffers[0]) {
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

            // initialize vertex vectors
            for (int i = 0; i < 16; i++) {
                _bspVertices[i].clear();
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


            for (int test = 0; test < pIndoor->faces.size(); test++) {
                BLVFace *face = &pIndoor->faces[test];

                if (face->isPortal())
                    continue;
                // TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
                if (!face->GetTexture())
                    continue;

                // TODO(pskelton): Same as outdoors. When ODM and BLV face is combined - seperate out function
                GraphicsImage *tex = face->GetTexture();
                std::string texname = tex->name();

                Duration animLength;
                Duration frame;
                if (face->IsAnimated()) {
                    tex = pTextureFrameTable->animationFrame(face->animationId, frame);
                    animLength = pTextureFrameTable->animationLength(face->animationId);
                    texname = tex->name();
                }

                int texunit = 0;
                int texlayer = 0;

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

                    if (face->IsAnimated()) {
                        // TODO(pskelton): any instances where animTime is not consistent would need checking
                        frame += pTextureFrameTable->animationFrameLength(face->animationId);
                        tex = pTextureFrameTable->animationFrame(face->animationId, frame);
                        if (!tex) break;
                        texname = tex->name();
                    }
                } while (animLength > frame);

                face->texunit = texunit;
                face->texlayer = texlayer;
            }

            for (int l = 0; l < 16; l++) {
                _bspBuffers[l].reset(GL_DYNAMIC_DRAW,
                    &ShaderVertex::pos, &ShaderVertex::texuv, &ShaderVertex::texturelayer,
                    &ShaderVertex::normal, &ShaderVertex::attribs);
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
                _bspVertices[i].clear();
            }

            bool drawnsky = false;

            for (unsigned i = 0; i < pBspRenderer->num_faces; ++i) {
                int uFaceID = pBspRenderer->faces[i].uFaceID;
                BLVFace *face = &pIndoor->faces[uFaceID];

                float skymodtimex{};
                float skymodtimey{};
                if (face->Indoor_sky()) {
                    if (face->polygonType != POLYGON_InBetweenFloorAndWall && face->polygonType != POLYGON_Floor) {
                        // draw forced perspective sky
                        DrawIndoorSky(face->numVertices, uFaceID);
                        continue;
                    } else {
                        // TODO(pskelton): check tickcount usage here
                        skymodtimex = (platform->tickCount() / 32.0f) - pCamera3D->vCameraPos.x;
                        skymodtimey = (platform->tickCount() / 32.0f) + pCamera3D->vCameraPos.y;
                    }
                }

                ++pBLVRenderParams->uNumFacesRenderedThisFrame;
                // load up verts here
                int texlayer = 0;
                int texunit = 0;
                int attribflags = 0;

                if (face->attributes & FACE_IsFluid)
                    attribflags |= 2;

                if (face->attributes & FACE_FlowDown)
                    attribflags |= 0x400;
                else if (face->attributes & FACE_FlowUp)
                    attribflags |= 0x800;

                if (face->attributes & FACE_FlowRight)
                    attribflags |= 0x2000;
                else if (face->attributes & FACE_FlowLeft)
                    attribflags |= 0x1000;

                if (face->attributes & FACE_IsLava)
                    attribflags |= 0x4000;

                if (face->attributes & FACE_OUTLINED || (face->attributes & FACE_IsSecret) && engine->is_saturate_faces)
                    attribflags |= 0x00010000;

                if (face->IsAnimated()) {
                    texlayer = -1;
                    texunit = -1;
                } else {
                    texlayer = face->texlayer;
                    texunit = face->texunit;
                }

                if (texlayer == -1) { // texture has been reset - see if its in the map
                    GraphicsImage *tex = face->GetTexture();
                    std::string texname = tex->name();
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


                for (int z = 0; z < (face->numVertices - 2); z++) {
                    // 123, 134, 145, 156..

                    // copy first
                    ShaderVertex &v0 = _bspVertices[texunit].emplace_back();
                    v0.pos = pIndoor->vertices[face->vertexIds[0]];
                    // TODO(captainurist): adding in IDs below?
                    v0.texuv = Vec2f(face->textureUs[0] + pIndoor->faceExtras[face->faceExtraId].textureDeltaU,
                                     face->textureVs[0] + pIndoor->faceExtras[face->faceExtraId].textureDeltaV);
                    if (face->Indoor_sky()) {
                        v0.texuv.x = (skymodtimex + v0.texuv.x) * 0.25f;
                        v0.texuv.y = (skymodtimey + v0.texuv.y) * 0.25f;
                    }
                    v0.texturelayer = texlayer;
                    v0.normal = face->facePlane.normal;
                    v0.attribs = attribflags;

                    // copy other two (z+1)(z+2)
                    for (unsigned i = 1; i < 3; ++i) {
                        ShaderVertex &v = _bspVertices[texunit].emplace_back();
                        v.pos = pIndoor->vertices[face->vertexIds[z + i]];
                        // TODO(captainurist): adding in IDs???
                        v.texuv = Vec2f(face->textureUs[z + i] + pIndoor->faceExtras[face->faceExtraId].textureDeltaU,
                                        face->textureVs[z + i] + pIndoor->faceExtras[face->faceExtraId].textureDeltaV);
                        if (face->Indoor_sky()) {
                            v.texuv.x = (skymodtimex + v.texuv.x) * 0.25f;
                            v.texuv.y = (skymodtimey + v.texuv.y) * 0.25f;
                        }
                        v.texturelayer = texlayer;
                        v.normal = face->facePlane.normal;
                        v.attribs = attribflags;
                    }
                }
            }

            for (int l = 0; l < 16; l++) {
                if (!_bspVertices[l].empty()) {
                    _bspBuffers[l].update(_bspVertices[l]);
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);

        // terrain debug
        if (config->debug.Terrain.value())
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

        bspshader.use();

        // set base uniforms
        BSPUniforms uniforms;
        uniforms.projection = projmat;
        uniforms.view = viewmat;
        uniforms.cameraPos.x = pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0f);
        uniforms.cameraPos.y = pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0f);
        uniforms.cameraPos.z = pParty->pos.z + pParty->eyeLevel;
        uniforms.gamma = gamma;
        uniforms.waterframe = waterAnimationFrame();
        uniforms.flowtimer = pMiscTimer->time().realtimeMilliseconds() >> 4;
        uniforms.flowtimerms = pMiscTimer->time().realtimeMilliseconds();

        // lighting stuff
        int16_t mintest = 0;
        for (int i = 0; i < pIndoor->sectors.size(); i++) {
            mintest = std::max(mintest, pIndoor->sectors[i].minAmbientLightLevel);
        }
        int uCurrentAmbientLightLevel = (DEFAULT_AMBIENT_LIGHT_LEVEL + mintest);
        float ambient = (248.0f - (uCurrentAmbientLightLevel << 3)) / 255.0f;
        float diffuseon = 0.0f;

        uniforms.sun.direction = Vec3f(0, 0, 0);
        uniforms.sun.ambient = Colorf(ambient, ambient, ambient);
        uniforms.sun.diffuse = Colorf(diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f), diffuseon * (ambient + 0.3f));
        uniforms.sun.specular = Colorf(diffuseon * 1.0f, diffuseon * 0.8f, 0.0f);

        // point lights - fspointlights
        // rest of lights stacking
        int num_lights = 0;

        // get party torchlight as priority
        if (pMobileLightsStack->uNumLightsActive >= 1) {
            MobileLight &test = pMobileLightsStack->pLights[0];
            Colorf color = test.uLightColor.toColorf();

            uniforms.pointLights[num_lights].type = 2.0f;
            uniforms.pointLights[num_lights].position = test.vPosition;
            uniforms.pointLights[num_lights].ambient = color;
            uniforms.pointLights[num_lights].diffuse = color;
            uniforms.pointLights[num_lights].specular = Colorf(0.0f, 0.0f, 0.0f);
            uniforms.pointLights[num_lights].radius = test.uRadius;
            num_lights++;
        }

        // stack the static lights next (wall torches)
        for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            StationaryLight &test = pStationaryLightsStack->pLights[i];

            // is this on the sector list
            bool onlist = false;
            for (unsigned j = 0; j < pBspRenderer->uNumVisibleNotEmptySectors; ++j) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j];
                if (test.uSectorID == listsector) {
                    onlist = true;
                    break;
                }
            }

            // does light sphere collide with current sector
            // expanded current sector
            bool fromexpanded{ false };
            if (pIndoor->sectors[pBLVRenderParams->uPartySectorID].boundingBox.intersectsCube(test.vPosition, test.uRadius)) {
                onlist = true;
                fromexpanded = true;
            }

            if (!onlist) continue;

            // cull through viewing frustum
            bool visinfrustum{ false };
            if (!fromexpanded) {
                for (int j = 0; j < pBspRenderer->num_nodes; ++j) {
                    if (pBspRenderer->nodes[j].uSectorID == test.uSectorID) {
                        if (IsSphereInFrustum(test.vPosition, test.uRadius, pBspRenderer->nodes[j].ViewportNodeFrustum.data()))
                            visinfrustum = true;
                    }
                }
            } else {
                if (IsSphereInFrustum(test.vPosition, test.uRadius)) visinfrustum = true;
            }
            if (!visinfrustum) continue;

            Colorf color = test.uLightColor.toColorf();

            uniforms.pointLights[num_lights].type = 1.0f;
            uniforms.pointLights[num_lights].position = test.vPosition;
            uniforms.pointLights[num_lights].ambient = color;
            uniforms.pointLights[num_lights].diffuse = color;
            uniforms.pointLights[num_lights].specular = Colorf(0.0f, 0.0f, 0.0f);
            uniforms.pointLights[num_lights].radius = test.uRadius;
            num_lights++;
        }

        // whatevers left for mobile lights
        for (int i = 1; i < pMobileLightsStack->uNumLightsActive; ++i) {
            if (num_lights >= 40) break;

            // TODO(pskelton): nearest lights should be prioritised
            MobileLight &test = pMobileLightsStack->pLights[i];
            if (!IsSphereInFrustum(test.vPosition, test.uRadius)) continue;

            Colorf color = test.uLightColor.toColorf();

            uniforms.pointLights[num_lights].type = 2.0f;
            uniforms.pointLights[num_lights].position = test.vPosition;
            uniforms.pointLights[num_lights].ambient = color;
            uniforms.pointLights[num_lights].diffuse = color;
            uniforms.pointLights[num_lights].specular = Colorf(0.0f, 0.0f, 0.0f);
            uniforms.pointLights[num_lights].radius = test.uRadius;
            num_lights++;
        }

        // remaining lights are default-initialized (type = 0)
        uniforms.submit(bspshader);

        // toggle for water faces or not
        glUniform1i(bspshader.uniformLocation("watertiles"), GLint(1));

        glActiveTexture(GL_TEXTURE0);

        for (int unit = 0; unit < 16; unit++) {
            // skip if textures are empty
            //if (numoutbuildtexloaded[unit] > 0) {
            if (unit == 1) {
                glUniform1i(bspshader.uniformLocation("watertiles"), GLint(0));
            }

            // draw each set of triangles
            glBindTexture(GL_TEXTURE_2D_ARRAY, bsptextures[unit]);
            _bspBuffers[unit].bind();
            glDrawArrays(GL_TRIANGLES, 0, _bspVertices[unit].size());
            drawcalls++;
            //}
        }

        bspshader.unuse();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // indoor sky drawing
        if (!_forcePerVertices.empty()) {
            // set forced matrixs
            _set_ortho_projection(1);
            _set_ortho_modelview();

            SkyBillboard.CalcSkyFrustumVec(1, 0, 0, 0, 1, 0);

            DrawForcePerVerts();

            _set_3d_projection_matrix();
            _set_3d_modelview_matrix();
        }


        //end terrain debug
        if (config->debug.Terrain.value())
            // TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
            if (!OpenGLES)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // stack decals start

        if (!decal_builder->bloodsplat_container->uNumBloodsplats) return;
        static RenderVertexSoft static_vertices_buff_in[64];  // buff in

        // loop over faces
        for (int test = 0; test < pIndoor->faces.size(); test++) {
            BLVFace *pface = &pIndoor->faces[test];

            if (pface->isPortal()) continue;
            // TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
            if (!pface->GetTexture()) continue;

            // check if faces is visible
            bool onlist = false;
            for (unsigned i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
                int listsector = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
                if (pface->sectorId == listsector) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;


            decal_builder->ApplyBloodsplatDecals_IndoorFace(test);
            if (!decal_builder->uNumSplatsThisFace) continue;

            // copy to buff in
            for (unsigned i = 0; i < pface->numVertices; ++i) {
                static_vertices_buff_in[i].vWorldPosition.x =
                    pIndoor->vertices[pface->vertexIds[i]].x;
                static_vertices_buff_in[i].vWorldPosition.y =
                    pIndoor->vertices[pface->vertexIds[i]].y;
                static_vertices_buff_in[i].vWorldPosition.z =
                    pIndoor->vertices[pface->vertexIds[i]].z;
                static_vertices_buff_in[i].u = pface->textureUs[i];
                static_vertices_buff_in[i].v = pface->textureVs[i];
            }

            // blood draw
            decal_builder->BuildAndApplyDecals(uCurrentAmbientLightLevel, LocationIndoors, pface->facePlane,
                pface->numVertices, static_vertices_buff_in,
                0, pface->sectorId);
        }



        ///////////////////////////////////////////////////////
        // stack decals end

        return;
}

bool OpenGLRenderer::Initialize() {
    if (!BaseRenderer::Initialize())
        return false;

    if (!window)
        return false;

    PlatformOpenGLOptions opts;

    // Set it only on startup as currently we don't support multiple contexts to be able to switch OpenGL<->OpenGLES in the middle of runtime.
    OpenGLES = config->graphics.Renderer.value() == RENDERER_OPENGL_ES;

    if (!OpenGLES) {
        // Use OpenGL 4.1 core
        opts.versionMajor = 4;
        opts.versionMinor = 1;
        opts.profile = GL_PROFILE_CORE;
    } else {
        // Use OpenGL ES 3.2
        opts.versionMajor = 3;
        opts.versionMinor = 2;
        opts.profile = GL_PROFILE_ES;
    }

    // Turn on 24bit Z buffer.
    // You may need to change this to 16 or 32 for your system
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

    auto glGetStringSafe = [] (int id) {
        // Need this wrapper b/c glGetString can return nullptr, actually happens under OpenGL 1.1 when called for
        // GL_SHADING_LANGUAGE_VERSION.
        const char *result = reinterpret_cast<const char *>(glGetString(id));
        return result ? result : "???";
    };

    if (!version) {
        logger->error("GLAD: Failed to initialize the OpenGL loader");
    } else {
        logger->info("OpenGL version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
        logger->info("OpenGL version string: {}", glGetStringSafe(GL_VERSION));
        logger->info("GLSL version: {}", glGetStringSafe(GL_SHADING_LANGUAGE_VERSION));
        // TODO(captainurist): this is probably the place to check OpenGL version & exit.
        //                     openenroth requires opengl core 4.1 or opengles 3.2 capable gpu to run.
    }

    gladSetGLPostCallback(GL_Check_Errors);

    _initImGui();

    return Reinitialize(true);
}

void OpenGLRenderer::_initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;   // Dont allow ImGui to modify mouse cursor

    SDL_Window *sdlWindow = static_cast<SDL_Window *>(window->nativeHandle());
    ImGui_ImplSDL3_InitForOpenGL(sdlWindow, openGLContext->nativeHandle());
    ImGui_ImplOpenGL3_Init();

    // Set ImGui Style
    ImVec4 *colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
}

void OpenGLRenderer::_shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void OpenGLRenderer::_initWaterTiles() {
    if (hd_water_tile_anim[0])
        return;

    for (unsigned i = 0; i < 7; ++i) {
        std::string container_name = fmt::format("HDWTR{:03}", i);
        hd_water_tile_anim[i] = assets->getBitmap(container_name);
    }
}

bool OpenGLRenderer::Reinitialize(bool firstInit) {
    BaseRenderer::Reinitialize(firstInit);

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

    this->clipRect = Recti(Pointi(0, 0), outputRender);

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
        if (!ReloadShaders())
            return false;
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

bool OpenGLRenderer::ReloadShaders() {
    logger->info("Reloading shaders...");

    ReleaseTerrain();
    ReleaseBSP();

    _textBuffer.reset();
    _textVertices.clear();

    _lineBuffer.reset();
    _lineVertices.clear();

    _twodBuffer.reset();
    _twodVertices.clear();

    _billboardBuffer.reset();
    if (paltex2D) {
        glDeleteTextures(1, &paltex2D);
        paltex2D = 0;
    }

    _billboardBuffer.reset();
    _billboardVertices.clear();

    _decalBuffer.reset();
    _decalVertices.clear();

    _forcePerBuffer.reset();
    _forcePerVertices.clear();

    const std::initializer_list<std::tuple<OpenGLShader *, std::string_view, std::string_view>> shaders = {
        {&terrainshader,    "glterrain",        "Terrain"},
        {&outbuildshader,   "gloutbuild",       "Outdoor buildings"},
        {&bspshader,        "glbspshader",      "Indoor BSP"},
        {&textshader,       "gltextshader",     "Text"},
        {&lineshader,       "gllinesshader",    "Lines"},
        {&twodshader,       "gltwodshader",     "2D"},
        {&billbshader,      "glbillbshader",    "Billboards"},
        {&decalshader,      "gldecalshader",    "Decals"},
        {&forcepershader,   "glforcepershader", "Forced perspective"}
    };

    SubFileSystem shadersFs("shaders", dfs);

    for (const auto &[shader, fileName, readableName] : shaders) {
        if (!shader->load(shadersFs.read(fmt::format("{}.vert", fileName)),
                          shadersFs.read(fmt::format("{}.frag", fileName)), OpenGLES, &shadersFs)) {
            platform->showMessageBox("CRITICAL ERROR: shader compilation failure",
                                     fmt::format("{} shader failed to compile!\nPlease consult the log and consider issuing a bug report!", readableName));
            return false;
        }
    }

    logger->info("Shaders reloaded.");
    return true;
}

void OpenGLRenderer::beginOverlays() {
    ImGui_ImplOpenGL3_NewFrame();
    // we assume we're always running with SDL
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void OpenGLRenderer::endOverlays() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    _terrainBuffer.reset();

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
        _outbuildBuffers[i].reset();
        _outbuildVertices[i].clear();
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
        _bspBuffers[i].reset();
        _bspVertices[i].clear();
    }
}


void OpenGLRenderer::DrawTwodVerts() {
    if (_twodVertices.empty()) return;

    Recti savedClipRect = this->clipRect;
    render->ResetUIClipRect();

    if (!_twodBuffer) {
        _twodBuffer.reset(GL_DYNAMIC_DRAW,
            &TwoDVertex::pos,
            &TwoDVertex::texuv,
            &TwoDVertex::color,
            &TwoDVertex::texid,
            &TwoDVertex::paletteid);
    }

    constexpr GLint paltex2D_id = 1;
    if (paltex2D == 0) {
        std::span<Color> palettes = pPaletteManager->paletteData();
        glActiveTexture(GL_TEXTURE0 + paltex2D_id);
        glGenTextures(1, &paltex2D);
        glBindTexture(GL_TEXTURE_2D, paltex2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, palettes.size() / 256, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, palettes.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    _twodBuffer.update(_twodVertices);
    _twodBuffer.bind();

    twodshader.use();

    // set sampler to palette
    glActiveTexture(GL_TEXTURE0 + paltex2D_id);
    glBindTexture(GL_TEXTURE_2D, paltex2D);

    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    TwoDUniforms uniforms;
    uniforms.projection = projmat;
    uniforms.view = viewmat;
    uniforms.paltex2D = paltex2D_id;
    uniforms.submit(twodshader);

    size_t offset = 0;
    while (offset < _twodVertices.size()) {
        // set texture
        GLfloat thistex = _twodVertices[offset].texid;
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(_twodVertices[offset].texid));
        if (_twodVertices[offset].paletteid) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        size_t cnt = 0;
        do {
            cnt++;
            if (offset + (6 * cnt) >= _twodVertices.size()) {
                break;
            }
        } while (_twodVertices[offset + (cnt * 6)].texid == thistex);

        glDrawArrays(GL_TRIANGLES, offset, (6*cnt));
        drawcalls++;

        offset += (6*cnt);
    }

    twodshader.unuse();
    _twodBuffer.unbind();

    _twodVertices.clear();
    render->SetUIClipRect(savedClipRect);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
