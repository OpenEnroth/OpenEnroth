#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/Graphics/Direct3D/Render.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"

using Graphics::IRenderFactory;
using Graphics::Configuration;

IRender *IRenderFactory::Create(Configuration *config) {
    switch (config->renderer_type) {
        case RendererType::DirectDraw:
            return new Render(config);

        case RendererType::OpenGl:
            return new RenderOpenGL(config);
    }
    return nullptr;
}

Configuration *IRenderFactory::ConfigureRender() {
    auto render_config = new Graphics::Configuration(
        config->renderer_name);
    render_config->is_fullscreen = !engine_config->RunInWindow();
    render_config->is_tinting = OS_GetAppInt("Tinting", 1) != 0;
    render_config->is_using_colored_lights = OS_GetAppInt("Colored Lights", 0) != 0;

    return render_config;
}