#pragma once
#include <memory>
#include <string>

#include "Application/GameConfig.h"

class IRender;
class OSWindow;

enum class RendererType {
    DirectDraw,
    OpenGL,
    OpenGLES
};

namespace Graphics {
    class IRenderFactory {
     public:
        std::shared_ptr<IRender> Create(std::shared_ptr<Application::GameConfig> config);
    };
}  // namespace Graphics
