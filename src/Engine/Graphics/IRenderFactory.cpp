#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/IocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"

using EngineIoc = Engine_::IocContainer;
using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<Application::GameConfig> config) {
    RendererType rendererType = config->graphics.Renderer.Get();

    switch (rendererType) {
        case RendererType::OpenGL:
            logger->Info("Initializing OpenGL renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );

        case RendererType::OpenGLES:
            logger->Info("Initializing OpenGL ES renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );

        default:
            return nullptr;
    }
}
