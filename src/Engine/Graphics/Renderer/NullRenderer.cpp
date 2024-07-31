#include "NullRenderer.h"

#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/EngineCallObserver.h"
#include "Engine/Graphics/Image.h"

#include "Library/Platform/Application/PlatformApplication.h"

bool NullRenderer::Initialize() {
    application->initializeOpenGLContext(PlatformOpenGLOptions());
    return BaseRenderer::Initialize();
}

bool NullRenderer::Reinitialize(bool firstInit) {
    openGLContext->swapBuffers();
    return BaseRenderer::Reinitialize(firstInit);
}

RgbaImage NullRenderer::ReadScreenPixels() {
    return RgbaImage::solid(640, 480, Color());
}

void NullRenderer::ClearTarget(Color uColor) {}

void NullRenderer::Present() {
    swapBuffers();
}

bool NullRenderer::InitializeFullscreen() {
    return true;
}

void NullRenderer::Release() {}

bool NullRenderer::SwitchToWindow() {
    return true;
}

void NullRenderer::BeginLines2D() {}
void NullRenderer::EndLines2D() {}
void NullRenderer::RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, Color uColor32) {}
void NullRenderer::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {}

void NullRenderer::RestoreFrontBuffer() {}
void NullRenderer::RestoreBackBuffer() {}
void NullRenderer::BltBackToFontFast(int a2, int a3, Recti *pSrcRect) {}
void NullRenderer::BeginScene3D() {
    // TODO(captainurist): doesn't belong here.
    uNumBillboardsToDraw = 0;
}

void NullRenderer::DrawTerrainPolygon(Polygon *a4, bool transparent, bool clampAtTextureBorders) {}

void NullRenderer::DrawProjectile(float srcX, float srcY, float a3, float a4,
                                  float dstX, float dstY, float a7, float a8,
                                  GraphicsImage *texture) {}

TextureRenderId NullRenderer::CreateTexture(RgbaImageView image) {
    return TextureRenderId(1);
}

void NullRenderer::DeleteTexture(TextureRenderId id) {}
void NullRenderer::UpdateTexture(TextureRenderId id, RgbaImageView image) {}

void NullRenderer::Update_Texture(GraphicsImage *texture) {}

void NullRenderer::BeginScene2D() {}
void NullRenderer::ScreenFade(Color color, float t) {}

void NullRenderer::SetUIClipRect(unsigned int uX, unsigned int uY,
                                 unsigned int uZ, unsigned int uW) {}
void NullRenderer::ResetUIClipRect() {}

void NullRenderer::DrawTextureNew(float u, float v, GraphicsImage *tex, Color colourmask) {
    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, tex->GetName());
}

void NullRenderer::DrawTextureCustomHeight(float u, float v, GraphicsImage *tex, int height) {
    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, tex->GetName());
}

void NullRenderer::DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                                     GraphicsImage *) {}
void NullRenderer::DrawImage(GraphicsImage *, const Recti &rect, unsigned int paletteid, Color colourmask) {}

void NullRenderer::BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                                 int start_opacity, int end_opacity) {}
void NullRenderer::TexturePixelRotateDraw(float u, float v, GraphicsImage *img, int time) {}

void NullRenderer::BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) {}
void NullRenderer::EndTextNew() {}
void NullRenderer::DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) {}

void NullRenderer::FillRectFast(unsigned int uX, unsigned int uY,
                                unsigned int uWidth, unsigned int uHeight,
                                Color uColor32) {}

void NullRenderer::DrawOutdoorBuildings() {}

void NullRenderer::DrawIndoorSky(unsigned int uNumVertices, int uFaceID) {}
void NullRenderer::DrawOutdoorSky() {}
void NullRenderer::DrawOutdoorTerrain() {}

bool NullRenderer::AreRenderSurfacesOk() {
    return true;
}

RgbaImage NullRenderer::MakeViewportScreenshot(const int width, const int height) {
    return RgbaImage::solid(width, height, Color());
}

RgbaImage NullRenderer::MakeFullScreenshot() {
    return RgbaImage::solid(640, 480, Color());
}

void NullRenderer::BeginLightmaps() {}
void NullRenderer::EndLightmaps() {}
void NullRenderer::BeginLightmaps2() {}
void NullRenderer::EndLightmaps2() {}
bool NullRenderer::DrawLightmap(Lightmap *pLightmap, Vec3f *pColorMult, float z_bias) {
    return true;
}

void NullRenderer::BeginDecals() {}
void NullRenderer::EndDecals() {}
void NullRenderer::DrawDecal(Decal *pDecal, float z_bias) {}

void NullRenderer::DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3,
                                       int blend_mode) {}

void NullRenderer::DrawIndoorFaces() {}

void NullRenderer::ReleaseTerrain() {}
void NullRenderer::ReleaseBSP() {}

void NullRenderer::DrawTwodVerts() {}

bool NullRenderer::ReloadShaders() { return true; }

void NullRenderer::DoRenderBillboards_D3D() {}

void NullRenderer::beginOverlays() {}
void NullRenderer::endOverlays() {}
void NullRenderer::flushAndScale() {}
void NullRenderer::swapBuffers() {
    openGLContext->swapBuffers();
}
