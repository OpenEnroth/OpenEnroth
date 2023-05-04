#include "Engine/EngineIocContainer.h"

#include "Arcomage/Arcomage.h"

#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/SpellFxRenderer.h"

#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Vis.h"

#include "Engine/Graphics/Level/Decoration.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/Player.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Engine/Spells/CastSpellInfo.h"

#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/PlayerFrameTable.h"

#include "Io/Mouse.h"

using Io::Mouse;

Logger *logger = nullptr;

Logger *EngineIocContainer::ResolveLogger() {
    if (!logger) {
        logger = new Logger();
    }
    return logger;
}

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
LightmapBuilder *EngineIocContainer::lightmap_builder = nullptr;
std::shared_ptr<Mouse> EngineIocContainer::mouse = nullptr;
std::shared_ptr<Nuklear> EngineIocContainer::nuklear = nullptr;
std::shared_ptr<ParticleEngine> EngineIocContainer::particle_engine = nullptr;
Vis *EngineIocContainer::vis = nullptr;
