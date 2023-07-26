#pragma once

#include <memory>

struct BloodsplatContainer;
struct DecalBuilder;
class Logger;
namespace Io {
    class Mouse;
}
class Nuklear;
class ParticleEngine;
struct SpellFxRenderer;
class Vis;

class EngineIocContainer {
 public:
    static Logger *ResolveLogger();
    static DecalBuilder *ResolveDecalBuilder();
    static BloodsplatContainer *ResolveBloodsplatContainer();
    static SpellFxRenderer *ResolveSpellFxRenderer();
    static std::shared_ptr<Io::Mouse> ResolveMouse();
    static std::shared_ptr<Nuklear> ResolveNuklear();
    static std::shared_ptr<ParticleEngine> ResolveParticleEngine();
    static Vis *ResolveVis();

 private:
     static DecalBuilder *decal_builder;
     static BloodsplatContainer *bloodspalt_container;
     static SpellFxRenderer *spell_fx_renderer;
     static std::shared_ptr<Io::Mouse> mouse;
     static std::shared_ptr<Nuklear> nuklear;
     static std::shared_ptr<ParticleEngine> particle_engine;
     static Vis *vis;
};


extern Logger *logger;
