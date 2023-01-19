#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/IocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#ifdef DDRAW_ENABLED
#include "Engine/Graphics/Direct3D/Render.h"
#endif


using EngineIoc = Engine_::IocContainer;
using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<Application::GameConfig> config) {
    RendererType rendererType = config->graphics.Renderer.Get();

#ifndef DDRAW_ENABLED
    if (config->graphics.Renderer.Get() == RendererType::DirectDraw)
        rendererType = RendererType::OpenGL;
#endif

    switch (rendererType) {
#ifdef DDRAW_ENABLED
        case RendererType::DirectDraw:
            logger->Info("Initializing DirectDraw renderer...");
            return std::make_shared<Render>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
#endif

        case RendererType::OpenGL:
            logger->Info("Initializing OpenGL renderer...");
            return std::make_shared<RenderOpenGL>(
                config,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
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
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
    }

    assert(false);
    return nullptr; // Make the compiler happy.
}
