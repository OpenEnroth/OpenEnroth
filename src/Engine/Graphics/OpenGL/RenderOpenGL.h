#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <glad/gl.h> // NOLINT: this is not a C system include.
#include <glm/glm.hpp>

#include "Engine/Graphics/FrameLimiter.h"
#include "Engine/Graphics/RenderBase.h"

#include "Library/Color/Colorf.h"

#include "GLShaderLoader.h"

class PlatformOpenGLContext;
struct nk_state;

class RenderOpenGL : public RenderBase {
 public:
    RenderOpenGL(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis,
        Logger *logger
    );
    virtual ~RenderOpenGL();

    virtual bool Initialize() override;

    virtual bool NuklearInitialize(struct nk_tex_font *tfont) override;
    virtual bool NuklearCreateDevice() override;
    virtual bool NuklearRender(/*enum nk_anti_aliasing*/ int AA, int max_vertex_buffer, int max_element_buffer) override;
    virtual void NuklearRelease() override;
    virtual struct nk_tex_font *NuklearFontLoad(const char *font_path, size_t font_size) override;
    virtual void NuklearFontFree(struct nk_tex_font *tfont) override;
    virtual struct nk_image NuklearImageLoad(GraphicsImage *img) override;
    virtual void NuklearImageFree(GraphicsImage *img) override;

    virtual RgbaImage ReadScreenPixels() override;
    virtual void SaveWinnersCertificate(const std::string &filePath) override;
    virtual void ClearTarget(Color uColor) override;
    virtual void Present() override;

    virtual bool InitializeFullscreen() override;

    virtual void Release() override;

    virtual bool SwitchToWindow() override;

    virtual void BeginLines2D() override;
    virtual void EndLines2D() override;
    virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ,
                              signed int uW, Color uColor32) override;
    virtual void DrawLines(const RenderVertexD3D3 *vertices,
        unsigned int num_vertices) override;

    virtual void RestoreFrontBuffer() override;
    virtual void RestoreBackBuffer() override;
    virtual void BltBackToFontFast(int a2, int a3, Recti *pSrcRect) override;
    virtual void BeginScene3D() override;

    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) override;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                GraphicsImage *texture) override;

    virtual TextureRenderId CreateTexture(RgbaImageView image) override;
    virtual void DeleteTexture(TextureRenderId id) override;
    virtual void UpdateTexture(TextureRenderId id, RgbaImageView image) override;

    virtual void Update_Texture(GraphicsImage *texture) override;

    virtual void BeginScene2D() override;
    virtual void ScreenFade(Color color, float t) override;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW) override;
    virtual void ResetUIClipRect() override;

    virtual void DrawTextureNew(float u, float v, class GraphicsImage *, Color colourmask = colorTable.White) override;

        virtual void DrawTextureCustomHeight(float u, float v, class GraphicsImage *,
                                         int height) override;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                                   GraphicsImage *) override;
    virtual void DrawImage(GraphicsImage *, const Recti &rect, unsigned int paletteid = 0, Color colourmask = colorTable.White) override;

    virtual void BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                               int start_opacity, int end_opacity) override;
    virtual void TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) override;

    virtual void BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) override;
    virtual void EndTextNew() override;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) override;

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              Color uColor32) override;

    virtual void DrawOutdoorBuildings() override;

    virtual void DrawIndoorSky(unsigned int uNumVertices, int uFaceID) override;
    virtual void DrawOutdoorSky() override;
    virtual void DrawOutdoorTerrain() override;

    virtual bool AreRenderSurfacesOk() override;

    virtual RgbaImage MakeScreenshot32(const int width, const int height) override;

    virtual void BeginLightmaps() override;
    virtual void EndLightmaps() override;
    virtual void BeginLightmaps2() override;
    virtual void EndLightmaps2() override;
    virtual bool DrawLightmap(struct Lightmap *pLightmap,
                              Vec3f *pColorMult, float z_bias) override;

    virtual void BeginDecals() override;
    virtual void EndDecals() override;
    virtual void DrawDecal(struct Decal *pDecal, float z_bias) override;

    virtual void DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3,
                               int blend_mode) override;

    virtual void DrawIndoorFaces() override;

    virtual void ReleaseTerrain() override;
    virtual void ReleaseBSP() override;

    virtual void DrawTwodVerts() override;
    void DrawBillboards();

    virtual Sizei GetRenderDimensions() override;
    virtual Sizei GetPresentDimensions() override;
    virtual bool Reinitialize(bool firstInit) override;
    virtual void ReloadShaders() override;

 protected:
    virtual void DoRenderBillboards_D3D() override;
    void SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1);

    void DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon);
    void DrawIndoorSkyPolygon(signed int uNumVertices,
                              struct Polygon *pSkyPolygon);
    void DrawForcePerVerts();

    void SetFogParametersGL();

    FrameLimiter _frameLimiter;

    // these are the view and projection matrices for submission to shaders
    glm::mat4 projmat = glm::mat4x4(1);
    glm::mat4 viewmat = glm::mat4x4(1);
    void _set_3d_projection_matrix();
    void _set_3d_modelview_matrix();
    void _set_ortho_projection(bool gameviewport = false);
    void _set_ortho_modelview();

    int clip_x{}, clip_y{};
    int clip_z{}, clip_w{};

    int GL_lastboundtex{};

    int GPU_MAX_TEX_SIZE{};
    int GPU_MAX_TEX_LAYERS{};
    int GPU_MAX_TEX_UNITS{};
    int GPU_MAX_UNIFORM_COMP{};
    int GPU_MAX_TOTAL_TEXTURES{};

    bool InitShaders();
    GLShader terrainshader;
    GLShader outbuildshader;
    GLShader bspshader;
    GLShader textshader;
    GLShader lineshader;
    GLShader twodshader;
    GLShader billbshader;
    GLShader decalshader;
    GLShader forcepershader;
    GLShader nuklearshader;

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

    // text shader
    GLuint textVBO{}, textVAO{};
    GLuint texmain{}, texshadow{};

    // lines shader
    GLuint lineVBO{}, lineVAO{};

    // two d shader
    GLuint twodVBO{}, twodVAO{};

    // billboards shader
    GLuint billbVBO{}, billbVAO{};
    GLuint palbuf{}, paltex{};

    // decal shader
    GLuint decalVBO{}, decalVAO{};

    // forced perspective shader
    GLuint forceperVBO{}, forceperVAO{};

    // Fog parameters
    Colorf fog;
    int fogstart{};
    int fogmiddle{};
    int fogend{};

    float gamma{};

    std::unique_ptr<nk_state> nk;
};


