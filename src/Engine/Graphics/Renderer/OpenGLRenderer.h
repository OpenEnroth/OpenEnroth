#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <glad/gl.h> // NOLINT: this is not a C system include.
#include <glm/glm.hpp>

#include "Engine/Graphics/FrameLimiter.h"
#include "BaseRenderer.h"

#include "Library/Color/Colorf.h"
#include "Library/Geometry/Vec.h"

#include "OpenGLVertexBuffer.h"
#include "OpenGLShader.h"

class PlatformOpenGLContext;

struct LineVertex {
    Vec2f pos;
    Colorf color;
};

struct ForcePerVertex {
    Vec3f pos;
    GLfloat w;
    Vec2f texuv;
    GLfloat texw;
    GLfloat screenspace;
    Colorf color;
    GLfloat texid;
};

struct TwoDVertex {
    Vec3f pos;
    Vec2f texuv;
    Colorf color;
    GLfloat texid;
    GLfloat paletteid;
};

struct DecalVertex {
    Vec3f pos;
    Vec2f texuv;
    GLfloat texunit;
    Colorf color;
};

struct BillboardVertex {
    Vec3f pos;
    Vec2f texuv;
    Colorf color;
    GLfloat screenspace;
    GLfloat texid;
    GLfloat blend;
    GLfloat paletteId;
};

class OpenGLRenderer : public BaseRenderer {
 public:
    OpenGLRenderer(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis
    );
    virtual ~OpenGLRenderer();

    virtual bool Initialize() override;

    virtual RgbaImage ReadScreenPixels() override;
    virtual void ClearTarget(Color uColor) override;
    virtual void Present() override;

    virtual void BeginLines2D() override;
    virtual void EndLines2D() override;
    virtual void RasterLine2D(Pointi a, Pointi b, Color acolor, Color bcolor) override;

    virtual void BeginScene3D() override;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                GraphicsImage *texture) override;

    virtual TextureRenderId CreateTexture(RgbaImageView image) override;
    virtual void DeleteTexture(TextureRenderId id) override;

    virtual void BeginScene2D() override;
    virtual void ScreenFade(Color color, float t) override;

    virtual void SetUIClipRect(const Recti &rect) override;
    virtual void ResetUIClipRect() override;

    virtual void DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, const Recti &dstRect, Color color) override;
    virtual void DrawImage(GraphicsImage *, const Recti &rect, int paletteid = 0, Color colourmask = colorTable.White) override;

    virtual void BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                               int start_opacity, int end_opacity) override;

    virtual void BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) override;
    virtual void EndTextNew() override;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) override;

    virtual void DrawOutdoorBuildings() override;

    virtual void DrawIndoorSky(int uNumVertices, int uFaceID) override;
    virtual void DrawOutdoorSky() override;
    virtual void DrawOutdoorTerrain() override;

    virtual RgbaImage MakeViewportScreenshot(const int width, const int height) override;
    virtual RgbaImage MakeFullScreenshot() override;

    virtual void BeginDecals() override;
    virtual void EndDecals() override;
    virtual void DrawDecal(Decal *pDecal, float z_bias) override;

    virtual void DrawIndoorFaces() override;

    virtual void ReleaseTerrain() override;
    virtual void ReleaseBSP() override;

    virtual void DrawTwodVerts() override;
    void DrawBillboards();

    virtual bool Reinitialize(bool firstInit) override;
    virtual bool ReloadShaders() override;

    virtual void flushAndScale() override;
    virtual void swapBuffers() override;

    virtual void beginOverlays() override;
    virtual void endOverlays() override;

 protected:
    virtual void DoRenderBillboards_D3D() override;
    void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);

    void DrawOutdoorSkyPolygon(int numVertices, GraphicsImage *texture, int dimmingLevel);
    void DrawIndoorSkyPolygon(int uNumVertices, GraphicsImage *texture, int dimmingLevel);
    void DrawForcePerVerts();

    void SetFogParametersGL();

    void _initImGui();
    void _shutdownImGui();
    void _initWaterTiles();

    FrameLimiter _frameLimiter;

    // these are the view and projection matrices for submission to shaders
    glm::mat4 projmat = glm::mat4x4(1);
    glm::mat4 viewmat = glm::mat4x4(1);
    void _set_3d_projection_matrix();
    void _set_3d_modelview_matrix();
    void _set_ortho_projection(bool gameviewport = false);
    void _set_ortho_modelview();

    Recti clipRect;

    int GL_lastboundtex{};

    int GPU_MAX_TEX_SIZE{};
    int GPU_MAX_TEX_LAYERS{};
    int GPU_MAX_TEX_UNITS{};
    int GPU_MAX_UNIFORM_COMP{};
    int GPU_MAX_TOTAL_TEXTURES{};

    OpenGLShader terrainshader;
    OpenGLShader outbuildshader;
    OpenGLShader bspshader;
    OpenGLShader textshader;
    OpenGLShader lineshader;
    OpenGLShader twodshader;
    OpenGLShader billbshader;
    OpenGLShader decalshader;
    OpenGLShader forcepershader;

    // terrain shader
    GLuint terrainVBO{}, terrainVAO{};
    // all terrain textures are square
    GLuint terraintextures[8]{};
    unsigned int numterraintexloaded[8]{};
    unsigned int terraintexturesizes[8]{};
    std::map<std::string, int> terraintexmap;

    // outside building shader
    GLuint outbuildVBO[16]{}, outbuildVAO[16]{};
    GLuint outbuildtextures[16]{};
    unsigned int numoutbuildtexloaded[16]{};
    unsigned int outbuildtexturewidths[16]{};
    unsigned int outbuildtextureheights[16]{};
    std::map<std::string, int> outbuildtexmap;

    // indoors bsp shader
    GLuint bspVBO[16]{}, bspVAO[16]{};
    GLuint bsptextures[16]{};
    unsigned int bsptexloaded[16]{};
    unsigned int bsptexturewidths[16]{};
    unsigned int bsptextureheights[16]{};
    std::map<std::string, int> bsptexmap;

    // line shader
    OpenGLVertexBuffer<LineVertex> _lineBuffer;

    // forced perspective shader
    OpenGLVertexBuffer<ForcePerVertex> _forcePerBuffer;

    // two d shader
    OpenGLVertexBuffer<TwoDVertex> _twodBuffer;

    // text shader
    OpenGLVertexBuffer<TwoDVertex> _textBuffer;
    GLuint texmain{}, texshadow{};

    // billboards shader
    OpenGLVertexBuffer<BillboardVertex> _billboardBuffer;
    GLuint paltex2D{};

    // decal shader
    OpenGLVertexBuffer<DecalVertex> _decalBuffer;

    // Fog parameters
    Colorf fog;
    int fogstart{};
    int fogmiddle{};
    int fogend{};

    float gamma{};

    std::array<GraphicsImage *, 7> hd_water_tile_anim = {{}}; // Water animation textures.

    std::vector<GLuint> _texturesForDeletion;
};


