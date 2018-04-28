#pragma once

struct BloodsplatContainer;
struct DecalBuilder;
class LightmapBuilder;
class Log;
class Mouse;
class Keyboard;
class ParticleEngine;
struct SpellFxRenderer;
class Vis;

namespace Engine_ {

class IocContainer {
 public:
    static Log *ResolveLogger();
    static DecalBuilder *ResolveDecalBuilder();
    static BloodsplatContainer *ResolveBloodsplatContainer();
    static SpellFxRenderer *ResolveSpellFxRenderer();
    static LightmapBuilder *ResolveLightmapBuilder();
    static Mouse *ResolveMouse();
    static Keyboard *ResolveKeyboard();
    static ParticleEngine *ResolveParticleEngine();
    static Vis *ResolveVis();

 private:
     static DecalBuilder *decal_builder;
     static BloodsplatContainer *bloodspalt_container;
     static SpellFxRenderer *spell_fx_renderer;
     static LightmapBuilder *lightmap_builder;
     static Mouse *mouse;
     static Keyboard *keyboard;
     static ParticleEngine *particle_engine;
     static Vis *vis;
};

}  // namespace Engine_


extern Log *logger;
