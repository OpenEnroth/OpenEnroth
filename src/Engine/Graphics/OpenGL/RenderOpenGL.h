#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "Engine/Graphics/FrameLimiter.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/RenderBase.h"
#include "Engine/MM7.h"
#include "Engine/Graphics/OpenGL/GLShaderLoader.h"

class PlatformOpenGLContext;

class RenderOpenGL : public RenderBase {
 public:
    RenderOpenGL(
        std::shared_ptr<Application::GameConfig> config,
        DecalBuilder* decal_builder,
        SpellFxRenderer* spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis* vis,
        Logger* logger
    );
    virtual ~RenderOpenGL();

    virtual bool Initialize() override;

    virtual bool NuklearInitialize(struct nk_tex_font *tfont) override;
    virtual bool NuklearCreateDevice() override;
    virtual bool NuklearRender(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer) override;
    virtual void NuklearRelease() override;
    virtual struct nk_tex_font *NuklearFontLoad(const char *font_path, size_t font_size) override;
    virtual void NuklearFontFree(struct nk_tex_font *tfont) override;
    virtual struct nk_image NuklearImageLoad(Image *img) override;
    virtual void NuklearImageFree(Image *img) override;

    virtual Texture *CreateTexture_Paletted(const std::string &name) override;
    virtual Texture *CreateTexture_ColorKey(const std::string &name, uint16_t colorkey) override;
    virtual Texture *CreateTexture_Solid(const std::string &name) override;
    virtual Texture *CreateTexture_Alpha(const std::string &name) override;

    virtual Texture *CreateTexture_PCXFromFile(const std::string &name) override;
    virtual Texture *CreateTexture_PCXFromIconsLOD(const std::string &name) override;
    virtual Texture *CreateTexture_PCXFromNewLOD(const std::string &name) override;
    virtual Texture *CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) override;

    virtual Texture *CreateTexture_Blank(unsigned int width, unsigned int height,
        IMAGE_FORMAT format, const void *pixels = nullptr) override;

    virtual Texture *CreateTexture(const std::string &name) override;
    virtual Texture *CreateSprite(
        const std::string &name, unsigned int palette_id,
        /*refactor*/ unsigned int lod_sprite_id) override;

    virtual uint8_t *ReadScreenPixels();
    virtual void SaveWinnersCertificate(const char *a1) override;
    virtual void ClearTarget(unsigned int uColor) override;
    virtual void Present() override;

    virtual bool InitializeFullscreen() override;

    virtual void CreateZBuffer() override;
    virtual void Release() override;

    virtual bool SwitchToWindow() override;

    virtual void BeginLines2D() override;
    virtual void EndLines2D() override;
    virtual void RasterLine2D(signed int uX, signed int uY, signed int uZ,
                              signed int uW, uint32_t uColor32) override;
    virtual void DrawLines(const RenderVertexD3D3* vertices,
        unsigned int num_vertices) override;

    virtual void ClearZBuffer() override;
    virtual void RestoreFrontBuffer() override;
    virtual void RestoreBackBuffer() override;
    virtual void BltBackToFontFast(int a2, int a3, Recti *pSrcRect) override;
    virtual void BeginScene3D() override;

    virtual void DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                    bool clampAtTextureBorders) override;

    virtual void DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                Texture *texture) override;

    virtual void RemoveTextureFromDevice(Texture* texture) override;
    virtual bool MoveTextureToDevice(Texture *texture) override;

    virtual void Update_Texture(Texture *texture) override;

    virtual void DeleteTexture(Texture *texture) override;

    virtual void BeginScene2D() override;
    virtual void ScreenFade(unsigned int color, float t) override;

    virtual void SetUIClipRect(unsigned int uX, unsigned int uY,
                               unsigned int uZ, unsigned int uW) override;
    virtual void ResetUIClipRect() override;

    virtual void DrawTextureNew(float u, float v, class Image *, uint32_t colourmask = 0xFFFFFFFF) override;

        virtual void DrawTextureCustomHeight(float u, float v, class Image *,
                                         int height) override;
    virtual void DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                                   Image *) override;
    virtual void DrawImage(Image *, const Recti &rect, uint paletteid = 0, uint32_t colourmask32 = 0xFFFFFFFF) override;

    virtual void ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal) override;
    virtual void BlendTextures(int a2, int a3, Image *a4, Image *a5, int t,
                               int start_opacity, int end_opacity) override;
    virtual void TexturePixelRotateDraw(float u, float v, Image *img, int time) override;

    virtual void BeginTextNew(Texture *main, Texture *shadow) override;
    virtual void EndTextNew() override;
    virtual void DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, uint16_t colour) override;

    virtual void FillRectFast(unsigned int uX, unsigned int uY,
                              unsigned int uWidth, unsigned int uHeight,
                              uint32_t uColor32) override;

    virtual void DrawOutdoorBuildings() override;

    virtual void DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID) override;
    virtual void DrawOutdoorSky() override;
    virtual void DrawOutdoorTerrain() override;

    virtual bool AreRenderSurfacesOk() override;

    virtual unsigned short *MakeScreenshot16(int width, int height) override;

    virtual std::vector<Actor*> getActorsInViewport(int pDepth) override;

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
    glm::mat4 projmat;
    glm::mat4 viewmat;
    void _set_3d_projection_matrix();
    void _set_3d_modelview_matrix();
    void _set_ortho_projection(bool gameviewport = false);
    void _set_ortho_modelview();

    int clip_x, clip_y;
    int clip_z, clip_w;

    int GL_lastboundtex;

    int GPU_MAX_TEX_SIZE;
    int GPU_MAX_TEX_LAYERS;
    int GPU_MAX_TEX_UNITS;
    int GPU_MAX_UNIFORM_COMP;
    int GPU_MAX_TOTAL_TEXTURES;

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
    GLuint terrainVBO, terrainVAO;
    // all terrain textures are square
    GLuint terraintextures[8];
    uint numterraintexloaded[8];
    uint terraintexturesizes[8];
    std::map<std::string, int> terraintexmap;

    // outside building shader
    GLuint outbuildVBO[16], outbuildVAO[16];
    GLuint outbuildtextures[16];
    uint numoutbuildtexloaded[16];
    uint outbuildtexturewidths[16];
    uint outbuildtextureheights[16];
    std::map<std::string, int> outbuildtexmap;

    // indoors bsp shader
    GLuint bspVBO[16], bspVAO[16];
    GLuint bsptextures[16];
    uint bsptexloaded[16];
    uint bsptexturewidths[16];
    uint bsptextureheights[16];
    std::map<std::string, int> bsptexmap;

    // text shader
    GLuint textVBO, textVAO;
    GLuint texmain, texshadow;

    // lines shader
    GLuint lineVBO, lineVAO;

    // two d shader
    GLuint twodVBO, twodVAO;

    // billboards shader
    GLuint billbVBO, billbVAO;
    GLuint palbuf, paltex;

    // decal shader
    GLuint decalVBO, decalVAO;

    // forced perspective shader
    GLuint forceperVBO, forceperVAO;

    // Fog parameters
    float fogr{}, fogg{}, fogb{};
    int fogstart{};
    int fogmiddle{};
    int fogend{};

    float gamma{};

    struct nk_vertex {
        float position[2];
        float uv[2];
        nk_byte col[4];
    } nk_vertex;
    struct nk_device {
        struct nk_buffer cmds;
        struct nk_draw_null_texture null;
        struct nk_font_atlas atlas;
        uint32_t vbo, vao, ebo;
        int32_t attrib_pos;
        int32_t attrib_uv;
        int32_t attrib_col;
        int32_t uniform_tex;
        int32_t uniform_proj;
    } nk_dev;
};


