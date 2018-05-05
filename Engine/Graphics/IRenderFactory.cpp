#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/Graphics/IRenderConfig.h"
#include "Engine/Graphics/IRenderConfigFactory.h"
#include "Engine/Graphics/Direct3D/Render.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"

using Graphics::IRenderFactory;
using Graphics::IRenderConfig;
using Graphics::IRenderConfigFactory;

std::shared_ptr<IRender> IRenderFactory::Create(const std::string &renderer_name, bool is_fullscreen) {
    IRenderConfigFactory renderConfigFactory;
    auto config = renderConfigFactory.Create(renderer_name, is_fullscreen);

    std::shared_ptr<IRender> renderer;
    switch (config->renderer_type) {
        case RendererType::DirectDraw:
            renderer = std::make_shared<Render>();
            break;

        case RendererType::OpenGl:
            renderer = std::make_shared<RenderOpenGL>();
            break;
    }
    if (renderer) {
        if (renderer->Configure(config)) {
            return renderer;
        }
    }

    return nullptr;
}
