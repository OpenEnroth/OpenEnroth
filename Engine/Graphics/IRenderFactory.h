#pragma once

#include "Engine/Graphics/Configuration.h"

class IRender;
namespace Graphics {

class IRenderFactory {
 public:
    static IRender *Create(Configuration *config);
};

}  // namespace Graphics
