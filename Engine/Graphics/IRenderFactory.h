#pragma once
#include <memory>
#include <string>

#include "Application/GameConfig.h"

class IRender;
class OSWindow;

enum class RendererType {
    DirectDraw,
    OpenGL
};

namespace Graphics {
    class IRenderFactory {
     public:
        std::shared_ptr<IRender> Create(std::shared_ptr<Application::GameConfig> config, std::shared_ptr<OSWindow> window);
    };
}  // namespace Graphics
