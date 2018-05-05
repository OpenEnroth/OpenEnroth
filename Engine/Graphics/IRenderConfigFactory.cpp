#include "Engine/Graphics/IRenderConfigFactory.h"

#include "Platform/Api.h"

using Graphics::IRenderConfigFactory;
using Graphics::IRenderConfig;


std::shared_ptr<IRenderConfig> IRenderConfigFactory::Clone(
    std::shared_ptr<const IRenderConfig> other
) {
    return std::make_shared<IRenderConfig>(*other.get());
}

std::shared_ptr<IRenderConfig> IRenderConfigFactory::Create(
    const std::string &renderer_name,
    bool is_fullscreen
) {
    auto config = std::make_shared<IRenderConfig>();

    if (renderer_name == "DirectDraw") {
        config->renderer_type = RendererType::DirectDraw;
    }
    if (renderer_name == "OpenGL") {
        config->renderer_type = RendererType::OpenGl;
    }

    config->is_fullscreen = is_fullscreen;
    config->is_tinting = OS_GetAppInt("Tinting", 1) != 0;
    config->is_using_colored_lights = OS_GetAppInt("Colored Lights", 0) != 0;

    return config;
}
