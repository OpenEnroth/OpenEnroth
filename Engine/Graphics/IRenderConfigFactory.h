#pragma once

#include <memory>
#include <string>

#include "Engine/Graphics/IRenderConfig.h"

namespace Graphics {

class IRenderConfigFactory {
 public:
    std::shared_ptr<IRenderConfig> Create(const std::string &renderer_name, bool is_fullscreen);
    std::shared_ptr<IRenderConfig> Clone(std::shared_ptr<const IRenderConfig> other);
};

}  // namespace Graphics
