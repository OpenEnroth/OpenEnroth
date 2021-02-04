#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/IocContainer.h"
#include "Engine/Graphics/IRenderConfig.h"
#include "Engine/Graphics/IRenderConfigFactory.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#ifdef _WINDOWS
#include "Engine/Graphics/Direct3D/Render.h"
#endif

#include "Platform/OSWindow.h"

using EngineIoc = Engine_::IocContainer;
using Graphics::IRenderFactory;
using Graphics::IRenderConfig;
using Graphics::IRenderConfigFactory;

std::shared_ptr<IRender> IRenderFactory::Create(
    std::shared_ptr<OSWindow> window,
    const std::string &renderer_name,
    bool is_fullscreen
) {
    IRenderConfigFactory renderConfigFactory;
    auto config = renderConfigFactory.Create(renderer_name, is_fullscreen);
    config->render_width = window->GetWidth();
    config->render_height = window->GetHeight();

    std::shared_ptr<IRender> renderer;
    switch (config->renderer_type) {
#ifdef _WINDOWS
        case RendererType::DirectDraw:
            renderer = std::make_shared<Render>(
                window,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            break;
#endif

        case RendererType::OpenGl:
            renderer = std::make_shared<RenderOpenGL>(
                window,
                EngineIoc::ResolveDecalBuilder(),
                EngineIoc::ResolveLightmapBuilder(),
                EngineIoc::ResolveSpellFxRenderer(),
                EngineIoc::ResolveParticleEngine(),
                EngineIoc::ResolveVis(),
                EngineIoc::ResolveLogger()
            );
            break;
    }

    if (renderer) {
        if (renderer->Configure(config)) {
            return renderer;
        }
    }

    return nullptr;
}
