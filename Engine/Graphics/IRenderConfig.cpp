#include "Engine/Graphics/Configuration.h"

using Graphics::Configuration;

Configuration::Configuration(const std::string &renderer_name) {
    if (renderer_name == "DirectDraw") {
        renderer_type = RendererType::DirectDraw;
    }
    if (renderer_name == "OpenGL") {
        renderer_type = RendererType::OpenGl;
    }
}
