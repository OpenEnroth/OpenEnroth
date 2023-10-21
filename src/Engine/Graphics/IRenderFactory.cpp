#include "Engine/Graphics/IRenderFactory.h"

#include "Application/GameConfig.h"

#include "Library/Logger/Logger.h"

#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#include "Engine/Graphics/RenderNull.h"

using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<GameConfig> config) {
    RendererType rendererType = config->graphics.Renderer.value();

    switch (rendererType) {
        case RENDERER_OPENGL:
            logger->info("Initializing OpenGL renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis()
            );

        case RENDERER_OPENGL_ES:
            logger->info("Initializing OpenGL ES renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis()
            );

        case RENDERER_NULL:
            logger->info("Initializing null renderer...");
            return std::make_shared<RenderNull>(
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
