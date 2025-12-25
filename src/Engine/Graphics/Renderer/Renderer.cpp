#include "Renderer.h"

#include <memory>

#include "Engine/Graphics/Image.h"

Renderer *render = nullptr;

Renderer::Renderer(
    std::shared_ptr<GameConfig> config,
    DecalBuilder *decal_builder,
    SpellFxRenderer *spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis *vis
) {
    this->config = config;
    this->decal_builder = decal_builder;
    this->spell_fx_renderer = spellfx;
    this->particle_engine = particle_engine;
    this->vis = vis;

    uFogColor = Color();
    memset(pBillboardRenderListD3D, 0, sizeof(pBillboardRenderListD3D));
    pSortedBillboardRenderListD3D.fill(nullptr);
    uNumBillboardsToDraw = 0;
    drawcalls = 0;
}

Renderer::~Renderer() = default;

void Renderer::DrawTextureNew(float u, float v, GraphicsImage *img, Color colourmask) {
    if (!img)
        return;

    Sizei renderDims = GetRenderDimensions();
    int x = static_cast<int>(u * renderDims.w);
    int y = static_cast<int>(v * renderDims.h + 0.5f);
    int width = img->width();
    int height = img->height();

    Recti srcRect(0, 0, width, height);
    Recti dstRect(x, y, width, height);
    DrawQuad2D(img, srcRect, dstRect, colourmask);
}

void Renderer::DrawQuad2D(GraphicsImage *texture, const Recti &srcRect, Pointi dstPoint, Color color) {
    Recti dstRect(dstPoint.x, dstPoint.y, srcRect.w, srcRect.h);
    DrawQuad2D(texture, srcRect, dstRect, color);
}

GraphicsImage *Renderer::solidFillTexture() {
    if (!_solidFillTexture)
        _solidFillTexture = GraphicsImage::Create(RgbaImage::solid(1, 1, colorTable.White));
    return _solidFillTexture;
}

void Renderer::FillRectFast(int x, int y, int width, int height, Color color) {
    Recti srcRect(0, 0, 1, 1);
    Recti dstRect(x, y, width, height);
    DrawQuad2D(solidFillTexture(), srcRect, dstRect, color);
}
