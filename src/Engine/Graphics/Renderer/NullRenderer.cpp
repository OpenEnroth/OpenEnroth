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
    return BaseRenderer::Reinitialize(firstInit);
}

RgbaImage NullRenderer::ReadScreenPixels() {
    return RgbaImage::solid(Color(), 640, 480);
}

void NullRenderer::ClearTarget(Color uColor) {}

void NullRenderer::Present() {
    swapBuffers();
}

void NullRenderer::BeginLines2D() {}
void NullRenderer::EndLines2D() {}
void NullRenderer::RasterLine2D(Pointi a, Pointi b, Color acolor, Color bcolor) {}

void NullRenderer::BeginScene3D() {
    // TODO(captainurist): doesn't belong here.
    uNumBillboardsToDraw = 0;
}

void NullRenderer::DrawProjectile(float srcX, float srcY, float a3, float a4,
                                  float dstX, float dstY, float a7, float a8,
                                  GraphicsImage *texture) {}

TextureRenderId NullRenderer::CreateTexture(RgbaImageView image) {
    return TextureRenderId(1);
}

void NullRenderer::DeleteTexture(TextureRenderId id) {}

void NullRenderer::BeginScene2D() {}
void NullRenderer::ScreenFade(Color color, float t) {}

void NullRenderer::SetUIClipRect(const Recti &rect) {}
void NullRenderer::ResetUIClipRect() {}

void NullRenderer::DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, const Recti &dstRect, Color color) {
    if (texture && engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_2D_TEXTURE, texture->name());
}

void NullRenderer::DrawImage(GraphicsImage *, const Recti &rect, int paletteid, Color colourmask) {}

void NullRenderer::BlendTextures(int a2, int a3, GraphicsImage *a4, GraphicsImage *a5, int t,
                                 int start_opacity, int end_opacity) {}

void NullRenderer::BeginTextNew(GraphicsImage *main, GraphicsImage *shadow) {}
void NullRenderer::EndTextNew() {}
void NullRenderer::DrawTextNew(int x, int y, int w, int h, float u1, float v1, float u2, float v2, int isshadow, Color colour) {}

void NullRenderer::DrawOutdoorBuildings() {}

void NullRenderer::DrawIndoorSky(int uNumVertices, int uFaceID) {}
void NullRenderer::DrawOutdoorSky() {}
void NullRenderer::DrawOutdoorTerrain() {}

RgbaImage NullRenderer::MakeViewportScreenshot(const int width, const int height) {
    return RgbaImage::solid(Color(), width, height);
}

RgbaImage NullRenderer::MakeFullScreenshot() {
    return RgbaImage::solid(Color(), 640, 480);
}

void NullRenderer::BeginDecals() {}
void NullRenderer::EndDecals() {}
void NullRenderer::DrawDecal(Decal *pDecal, float z_bias) {}

void NullRenderer::DrawIndoorFaces() {}

void NullRenderer::ReleaseTerrain() {}
void NullRenderer::ReleaseBSP() {}

void NullRenderer::DrawTwodVerts() {}

bool NullRenderer::ReloadShaders() { return true; }

void NullRenderer::DoRenderBillboards_D3D() {
    SortBillboards();
}

void NullRenderer::beginOverlays() {}
void NullRenderer::endOverlays() {}
void NullRenderer::flushAndScale() {}
void NullRenderer::swapBuffers() {
    openGLContext->swapBuffers();
}
