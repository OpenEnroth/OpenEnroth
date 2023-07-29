#include "RenderNull.h"

#include <memory>

#include <nuklear_config.h> // NOLINT: not a C system header.

#include "Engine/EngineGlobals.h"

#include "Library/Application/PlatformApplication.h"

bool RenderNull::Initialize() {
    application->initializeOpenGLContext(PlatformOpenGLOptions());
    return RenderBase::Initialize();
}

bool RenderNull::NuklearInitialize(struct nk_tex_font *tfont) {
    return false;
}

bool RenderNull::NuklearCreateDevice() {
    return true;
}

bool RenderNull::NuklearRender(/*enum nk_anti_aliasing*/ int AA, int max_vertex_buffer, int max_element_buffer) {
    return true;
}

void RenderNull::NuklearRelease() {}

struct nk_tex_font *RenderNull::NuklearFontLoad(const char *font_path, size_t font_size) {
    return nullptr;
}

void RenderNull::NuklearFontFree(struct nk_tex_font *tfont) {}

struct nk_image RenderNull::NuklearImageLoad(GraphicsImage *img) {
    return {};
}

void RenderNull::NuklearImageFree(GraphicsImage *img) {}

RgbaImage RenderNull::ReadScreenPixels() {
    return RgbaImage::solid(640, 480, Color());
}

void RenderNull::ClearTarget(Color uColor) {}

void RenderNull::Present() {
    openGLContext->swapBuffers();
}

bool RenderNull::InitializeFullscreen() {
    return true;
}

void RenderNull::Release() {}

bool RenderNull::SwitchToWindow() {
    return true;
}

void RenderNull::BeginLines2D() {}
void RenderNull::EndLines2D() {}
void RenderNull::RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, Color uColor32) {}
void RenderNull::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {}

void RenderNull::RestoreFrontBuffer() {}
void RenderNull::RestoreBackBuffer() {}
void RenderNull::BltBackToFontFast(int a2, int a3, Recti *pSrcRect) {}
void RenderNull::BeginScene3D() {
    // TODO(captainurist): doesn't belong here.
    uNumBillboardsToDraw = 0;
}

void RenderNull::DrawTerrainPolygon(struct Polygon *a4, bool transparent, bool clampAtTextureBorders) {}

void RenderNull::DrawProjectile(float srcX, float srcY, float a3, float a4,
                                float dstX, float dstY, float a7, float a8,
                                GraphicsImage *texture) {}

TextureRenderId RenderNull::CreateTexture(RgbaImageView image) {
    return TextureRenderId(1);
}

void RenderNull::DeleteTexture(TextureRenderId id) {}
void RenderNull::UpdateTexture(TextureRenderId id, RgbaImageView image) {}

void RenderNull::Update_Texture(GraphicsImage *texture) {}

void RenderNull::BeginScene2D() {}
void RenderNull::ScreenFade(Color color, float t) {}

void RenderNull::SetUIClipRect(unsigned int uX, unsigned int uY,
                           unsigned int uZ, unsigned int uW) {}
void RenderNull::ResetUIClipRect() {}

void RenderNull::DrawTextureNew(float u, float v, class GraphicsImage *, Color colourmask) {}

void RenderNull::DrawTextureCustomHeight(float u, float v, class GraphicsImage *,
                                     int height) {}
void RenderNull::DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                               GraphicsImage *) {}
void RenderNull::DrawImage(GraphicsImage *, const Recti &rect, unsigned int paletteid, Color colourmask) {}

void RenderNull::BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                           int start_opacity, int end_opacity) {}
void RenderNull::TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) {}

void RenderNull::BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) {}
void RenderNull::EndTextNew() {}
void RenderNull::DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) {}

void RenderNull::FillRectFast(unsigned int uX, unsigned int uY,
                          unsigned int uWidth, unsigned int uHeight,
                          Color uColor32) {}

void RenderNull::DrawOutdoorBuildings() {}

void RenderNull::DrawIndoorSky(unsigned int uNumVertices, int uFaceID) {}
void RenderNull::DrawOutdoorSky() {}
void RenderNull::DrawOutdoorTerrain() {}

bool RenderNull::AreRenderSurfacesOk() {
    return true;
}

RgbaImage RenderNull::MakeScreenshot32(const int width, const int height) {
    return RgbaImage::solid(640, 480, Color());
}

void RenderNull::BeginLightmaps() {}
void RenderNull::EndLightmaps() {}
void RenderNull::BeginLightmaps2() {}
void RenderNull::EndLightmaps2() {}
bool RenderNull::DrawLightmap(struct Lightmap *pLightmap, Vec3f *pColorMult, float z_bias) {
    return true;
}

void RenderNull::BeginDecals() {}
void RenderNull::EndDecals() {}
void RenderNull::DrawDecal(struct Decal *pDecal, float z_bias) {}

void RenderNull::DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3,
                                 int blend_mode) {}

void RenderNull::DrawIndoorFaces() {}

void RenderNull::ReleaseTerrain() {}
void RenderNull::ReleaseBSP() {}

void RenderNull::DrawTwodVerts() {}

void RenderNull::ReloadShaders() {}

void RenderNull::DoRenderBillboards_D3D() {}
