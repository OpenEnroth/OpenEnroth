#pragma once

#include <string>

namespace Graphics {

enum class RendererType {
    DirectDraw,
    OpenGl
};

class IRenderConfig {
 public:
    inline bool IsFullscreen() const { return is_fullscreen; }


    int render_width = 640;
    int render_height = 480;

    bool is_fullscreen = false;
    bool is_tinting = true;
    bool is_using_specular = false;
    bool is_using_colored_lights = false;
    bool is_using_fog = false;

    RendererType renderer_type = RendererType::DirectDraw;
};

}  // namespace Graphics
