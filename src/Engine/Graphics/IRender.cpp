#include "IRender.h"

IRender::IRender(
    std::shared_ptr<GameConfig> config,
    DecalBuilder *decal_builder,
    SpellFxRenderer *spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis *vis,
    Logger *logger
) {
    this->config = config;
    this->decal_builder = decal_builder;
    this->spell_fx_renderer = spellfx;
    this->particle_engine = particle_engine;
    this->vis = vis;
    this->log = logger;

    pActiveZBuffer = 0;
    uFogColor = Color();
    hd_water_current_frame = 0;
    memset(pBillboardRenderListD3D, 0, sizeof(pBillboardRenderListD3D));
    uNumBillboardsToDraw = 0;
    drawcalls = 0;
}

IRender::~IRender() = default;
