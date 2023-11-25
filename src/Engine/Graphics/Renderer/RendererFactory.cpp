#include "RendererFactory.h"

#include "Application/GameConfig.h"

#include "Library/Logger/Logger.h"

#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/Renderer/OpenGLRenderer.h"
#include "Engine/Graphics/Renderer/NullRenderer.h"

std::shared_ptr<Renderer> RendererFactory::Create(std::shared_ptr<GameConfig> config) {
    RendererType rendererType = config->graphics.Renderer.value();

    switch (rendererType) {
        case RENDERER_OPENGL:
            logger->info("Initializing OpenGL renderer...");
            return std::make_shared<OpenGLRenderer>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis()
            );

        case RENDERER_OPENGL_ES:
            logger->info("Initializing OpenGL ES renderer...");
            return std::make_shared<OpenGLRenderer>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis()
            );

        case RENDERER_NULL:
            logger->info("Initializing null renderer...");
            return std::make_shared<NullRenderer>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis()
            );

        default:
            return nullptr;
    }
}
