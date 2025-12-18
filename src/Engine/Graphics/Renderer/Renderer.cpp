#include "Renderer.h"

#include <memory>

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
