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

void Renderer::DrawTextureCustomHeight(float u, float v, GraphicsImage *img, int customHeight) {
    if (!img)
        return;

    Sizei renderDims = GetRenderDimensions();
    int x = static_cast<int>(u * renderDims.w);
    int y = static_cast<int>(v * renderDims.h + 0.5f);
    int width = img->width();

    Recti srcRect(0, 0, width, customHeight);
    Recti dstRect(x, y, width, customHeight);
    DrawQuad2D(img, srcRect, dstRect, colorTable.White);
}

void Renderer::DrawTextureOffset(int x, int y, int offsetX, int offsetY, GraphicsImage *img) {
    if (!img)
        return;

    Sizei renderDims = GetRenderDimensions();
    float u = static_cast<float>(x - offsetX) / renderDims.w;
    float v = static_cast<float>(y - offsetY) / renderDims.h;
    DrawTextureNew(u, v, img);
}

void Renderer::DrawFromSpriteSheet(GraphicsImage *texture, const Recti &srcRect, Pointi targetPoint, Color color) {
    if (!texture)
        return;

    Recti dstRect(targetPoint.x, targetPoint.y, srcRect.w, srcRect.h);
    DrawQuad2D(texture, srcRect, dstRect, color);
}

void Renderer::FillRectFast(int x, int y, int width, int height, Color color) {
    if (!_solidFillTexture)
        _solidFillTexture = GraphicsImage::Create(RgbaImage::solid(1, 1, colorTable.White));
    if (!_solidFillTexture)
        return;

    Recti srcRect(0, 0, 1, 1);
    Recti dstRect(x, y, width, height);
    DrawQuad2D(_solidFillTexture, srcRect, dstRect, color);
}
