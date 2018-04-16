#pragma once

#include <string>

namespace Graphics
{
    enum class RendererType
    {
        DirectDraw,
        OpenGl
    };

    class Configuration
    {
        public:
            Configuration(const std::string &renderer_name);

            inline bool IsFullscreen() const { return is_fullscreen; }


            int render_width = 640;
            int render_height = 480;
            bool is_fullscreen = false;

            RendererType renderer_type = RendererType::DirectDraw;
    };
}