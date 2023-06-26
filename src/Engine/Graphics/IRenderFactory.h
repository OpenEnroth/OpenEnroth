#pragma once

#include <memory>
#include <string>

#include "RendererType.h"

class IRender;
class GameConfig;

namespace Graphics {
    class IRenderFactory {
     public:
        std::shared_ptr<IRender> Create(std::shared_ptr<GameConfig> config);
    };
}  // namespace Graphics
