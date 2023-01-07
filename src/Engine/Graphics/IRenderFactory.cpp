#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/IocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#ifdef DDRAW_ENABLED
#include "Engine/Graphics/Direct3D/Render.h"
#endif


using EngineIoc = Engine_::IocContainer;
using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<Application::GameConfig> config) {
    RendererType rendererType = RendererType::OpenGL;
    std::shared_ptr<IRender> renderer = nullptr;

    if (config->graphics.Renderer.Get() == "OpenGLES")
        rendererType = RendererType::OpenGLES;
#ifdef DDRAW_ENABLED
    else if (config->graphics.Renderer.Get() == "DirectDraw")
        rendererType = RendererType::DirectDraw;
#endif

    switch (rendererType) {
#ifdef DDRAW_ENABLED
        case RendererType::DirectDraw:
            logger->Info("Initializing DirectDraw renderer...");
            renderer = std::make_shared<Render>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            config->graphics.Renderer.Set("DirectDraw");
            break;
#endif

        case RendererType::OpenGL:
            logger->Info("Initializing OpenGL renderer...");
            renderer = std::make_shared<RenderOpenGL>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            config->graphics.Renderer.Set("OpenGL");
            break;

        case RendererType::OpenGLES:
            logger->Info("Initializing OpenGL ES renderer...");
            renderer = std::make_shared<RenderOpenGL>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            config->graphics.Renderer.Set("OpenGLES");
            break;

        default:
            break;
    }

    return renderer;
}
