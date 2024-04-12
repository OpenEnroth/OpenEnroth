#include "RendererFactory.h"

#include <cassert>
#include <memory>

#include "Library/Logger/Logger.h"

#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/Renderer/OpenGLRenderer.h"
#include "Engine/Graphics/Renderer/NullRenderer.h"

std::unique_ptr<Renderer> RendererFactory::createRenderer(RendererType type, std::shared_ptr<GameConfig> config) {
    switch (type) {
    case RENDERER_OPENGL:
        logger->info("Initializing OpenGL renderer...");
        return std::make_unique<OpenGLRenderer>(
            config,
            EngineIocContainer::ResolveDecalBuilder(),
            EngineIocContainer::ResolveSpellFxRenderer(),
            EngineIocContainer::ResolveParticleEngine(),
            EngineIocContainer::ResolveVis()
        );

    case RENDERER_OPENGL_ES:
        logger->info("Initializing OpenGL ES renderer...");
        return std::make_unique<OpenGLRenderer>(
            config,
            EngineIocContainer::ResolveDecalBuilder(),
            EngineIocContainer::ResolveSpellFxRenderer(),
            EngineIocContainer::ResolveParticleEngine(),
            EngineIocContainer::ResolveVis()
        );

    default:
        assert(false);
        [[fallthrough]];

    case RENDERER_NULL:
        logger->info("Initializing null renderer...");
        return std::make_unique<NullRenderer>(
            config,
            EngineIocContainer::ResolveDecalBuilder(),
            EngineIocContainer::ResolveSpellFxRenderer(),
            EngineIocContainer::ResolveParticleEngine(),
            EngineIocContainer::ResolveVis()
        );
    }
}
