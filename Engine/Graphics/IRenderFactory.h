#pragma once

#include <memory>
#include <string>

class IRender;
namespace Graphics {

class IRenderFactory {
 public:
    std::shared_ptr<IRender> Create(const std::string &renderer_name, bool is_fullscreen);
};

}  // namespace Graphics
