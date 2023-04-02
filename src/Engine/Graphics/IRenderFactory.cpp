#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"

using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<GameConfig> config) {
    RendererType rendererType = config->graphics.Renderer.value();

    switch (rendererType) {
        case RendererType::OpenGL:
            logger->info("Initializing OpenGL renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis(),
                EngineIocContainer::ResolveLogger()
            );

        case RendererType::OpenGLES:
            logger->info("Initializing OpenGL ES renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIocContainer::ResolveDecalBuilder(),
                EngineIocContainer::ResolveSpellFxRenderer(),
                EngineIocContainer::ResolveParticleEngine(),
                EngineIocContainer::ResolveVis(),
                EngineIocContainer::ResolveLogger()
            );

        default:
            return nullptr;
    }
}
