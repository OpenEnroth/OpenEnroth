#pragma once
#include <memory>
#include <string>

#include "Application/GameConfig.h"

#include "RendererType.h"

class IRender;

namespace Graphics {
    class IRenderFactory {
     public:
        std::shared_ptr<IRender> Create(std::shared_ptr<GameConfig> config);
    };
}  // namespace Graphics
