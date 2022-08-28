#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/IocContainer.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#ifdef _WINDOWS
#include "Engine/Graphics/Direct3D/Render.h"
#endif

#include "Platform/OSWindow.h"

using EngineIoc = Engine_::IocContainer;
using Graphics::IRenderFactory;

std::shared_ptr<IRender> IRenderFactory::Create(std::shared_ptr<Application::GameConfig> config, std::shared_ptr<OSWindow> window) {
    RendererType rendererType = RendererType::OpenGL;
    std::shared_ptr<IRender> renderer = nullptr;

#ifdef _WINDOWS
    if (config->graphics.Renderer.Get() == "DirectDraw")
        rendererType = RendererType::DirectDraw;
#endif

    switch (rendererType) {
#ifdef _WINDOWS
        case RendererType::DirectDraw:
            renderer = std::make_shared<Render>(
                config,
                window,
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
            renderer = std::make_shared<RenderOpenGL>(
                config,
                window,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            config->graphics.Renderer.Set("OpenGL");
            break;

        default:
            break;
    }

    return renderer;
}
