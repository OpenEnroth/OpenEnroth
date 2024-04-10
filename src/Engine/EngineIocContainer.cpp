#include "Engine/EngineIocContainer.h"

#include <memory>

#include "Engine/SpellFxRenderer.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Vis.h"

#include "Io/Mouse.h"

using Io::Mouse;

DecalBuilder *EngineIocContainer::ResolveDecalBuilder() {
    if (!decal_builder) {
        decal_builder = new DecalBuilder();
    }
    return decal_builder;
}

BloodsplatContainer *EngineIocContainer::ResolveBloodsplatContainer() {
    if (!bloodspalt_container) {
        bloodspalt_container = new BloodsplatContainer();
    }
    return bloodspalt_container;
}

SpellFxRenderer *EngineIocContainer::ResolveSpellFxRenderer() {
    if (!spell_fx_renderer) {
        spell_fx_renderer = new SpellFxRenderer(ResolveParticleEngine());
    }

    return spell_fx_renderer;
}

std::shared_ptr<Mouse> EngineIocContainer::ResolveMouse() {
    if (!mouse) {
        mouse = std::make_shared<Mouse>();
    }

    return mouse;
}

std::shared_ptr<Nuklear> EngineIocContainer::ResolveNuklear() {
    if (!nuklear) {
        nuklear = std::make_shared<Nuklear>();
    }

    return nuklear;
}

std::shared_ptr<ParticleEngine> EngineIocContainer::ResolveParticleEngine() {
    if (!particle_engine) {
        particle_engine = std::make_shared<ParticleEngine>();
    }

    return particle_engine;
}

Vis *EngineIocContainer::ResolveVis() {
    if (!vis) {
        vis = new Vis();
    }
    return vis;
}

DecalBuilder *EngineIocContainer::decal_builder = nullptr;
BloodsplatContainer *EngineIocContainer::bloodspalt_container = nullptr;
SpellFxRenderer *EngineIocContainer::spell_fx_renderer = nullptr;
std::shared_ptr<Mouse> EngineIocContainer::mouse = nullptr;
std::shared_ptr<Nuklear> EngineIocContainer::nuklear = nullptr;
std::shared_ptr<ParticleEngine> EngineIocContainer::particle_engine = nullptr;
Vis *EngineIocContainer::vis = nullptr;
