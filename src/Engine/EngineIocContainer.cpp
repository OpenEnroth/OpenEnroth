#include "Engine/EngineIocContainer.h"

#include "Arcomage/Arcomage.h"

#include "Engine/Events.h"
#include "Engine/MapsLongTimer.h"
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



void IntegrityTest() {
    // ToDo(_): make platform independent
    // static_assert(sizeof(BLVDoor) == 0x50, "Wrong type size");
    // static_assert(sizeof(Particle_sw) == 0x68, "Wrong type size");
    // static_assert(sizeof(Particle) == 0x68, "Wrong type size");
    // static_assert(sizeof(ParticleEngine) == 0xE430, "Wrong type size");
    // static_assert(sizeof(Lightmap) == 0xC1C, "Wrong type size");
    // static_assert(sizeof(Vis_SelectionList) == 0x2008, "Wrong type size");
    // static_assert(sizeof(Vis) == 0x20D0, "Wrong type size");
    // static_assert(sizeof(ProjectileAnim) == 0x1C, "Wrong type size");
    // static_assert(sizeof(IndoorCameraD3D_Vec3) == 0x10, "Wrong type size");
    // static_assert(sizeof(IndoorCameraD3D_Vec4) == 0x18, "Wrong type size");  // should be 14 (10 vec3 + 4 vdtor)  but 18 coz of
                                                                             // his +4 from own vdtor, but it is odd since vdtor
                                                                             // already present from vec3
    // static_assert(sizeof(NPCData) == 0x4C, "Wrong type size");
    // static_assert(sizeof(NPCStats) == 0x17FFC, "Wrong type size");
    // static_assert(sizeof(BspRenderer) == 0x53740, "Wrong type size");
    // static_assert(sizeof(ViewingParams) == 0x26C, "Wrong type size");
    // static_assert(sizeof(Bloodsplat) == 0x28, "Wrong type size");
    // static_assert(sizeof(BloodsplatContainer) == 0xA0C, "Wrong type size");
    // static_assert(sizeof(_2devent) == 0x34, "Wrong type size");
    // static_assert(sizeof(StorylineText) == 0x160, "Wrong type size");
    // static_assert(sizeof(Decal) == 0xC20, "Wrong type size");
    // static_assert(sizeof(MonsterStats) == 0x5BA0, "Wrong type size");

    // ToDo(_): move to usage place
    static_assert(sizeof(OverlayDesc) == 8, "Wrong type size");
    static_assert(sizeof(DecorationDesc) == 84, "Wrong type size");
    static_assert(sizeof(PlayerFrame) == 10, "Wrong type size");
    static_assert(sizeof(RenderVertexSoft) == 0x30, "Wrong type size");
    static_assert(sizeof(Timer) == 0x28, "Wrong type size");
    static_assert(sizeof(OtherOverlay) == 0x14, "Wrong type size");
    static_assert(sizeof(ItemGen) == 0x24, "Wrong type size");
    //static_assert(sizeof(SpriteObject) == 0x70, "Wrong type size");
    static_assert(sizeof(Chest) == 0x14CC, "Wrong type size");
    //static_assert(sizeof(SpellData) == 0x14, "Wrong type size");
    //static_assert(sizeof(SpellBuff) == 0x10, "Wrong type size");
    static_assert(sizeof(AIDirection) == 0x1C, "Wrong type size");
    static_assert(sizeof(ActorJob) == 0xC, "Wrong type size");
    static_assert(sizeof(LevelDecoration) == 0x20, "Wrong type size");
    static_assert(sizeof(BLVFaceExtra) == 0x24, "Wrong type size");
    static_assert(sizeof(BLVLightMM7) == 0x10, "Wrong type size");
    static_assert(sizeof(PlayerBuffAnim) == 0x10, "Wrong type size");
    //static_assert(sizeof(StationaryLight) == 0xC, "Wrong type size");
    //static_assert(sizeof(MobileLight) == 0x12, "Wrong type size");
    //static_assert(sizeof(stru141_actor_collision_object) == 0xA8, "Wrong type size");
    static_assert(sizeof(ActionQueue) == 0x7C, "Wrong type size");
    //static_assert(sizeof(PaletteManager) == 0x267AF0, "Wrong type size");
    static_assert(sizeof(TrailParticle) == 0x18, "Wrong type size");
    static_assert(sizeof(EventIndex) == 0xC, "Wrong type size");
    //static_assert(sizeof(MapsLongTimer) == 0x20, "Wrong type size");
    static_assert(sizeof(SavegameHeader) == 0x64, "Wrong type size");
    static_assert(sizeof(FactionTable) == 0x1EF1, "Wrong type size");
    //static_assert(sizeof(PartyTimeStruct) == 0x678, "Wrong type size");
    //static_assert(sizeof(CastSpellInfo) == 0x14, "Wrong type size");
    static_assert(sizeof(ArcomageCard) == 0x6C, "Wrong type size");
    static_assert(sizeof(Vec3s) == 6, "Wrong type size");
    static_assert(sizeof(BLVFaceExtra) == 36, "Wrong type size");
    static_assert(sizeof(LevelDecoration) == 32, "Wrong type size");
    static_assert(sizeof(BLVLightMM7) == 16, "Wrong type size");
    static_assert(sizeof(BSPNode) == 8, "Wrong type size");
    static_assert(sizeof(DDM_DLV_Header) == 40, "Wrong type size");
    //static_assert(sizeof(SpriteObject) == 112, "Wrong type size");
    static_assert(sizeof(Chest) == 5324, "Wrong type size");
    static_assert(sizeof(MapEventVariables) == 0xC8, "Wrong type size");
    static_assert(sizeof(BLVMapOutline) == 12, "Wrong type size");
}
