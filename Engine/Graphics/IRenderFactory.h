#pragma once
#include <memory>
#include <string>

class IRender;
class OSWindow;

namespace Graphics {

class IRenderFactory {
 public:
    std::shared_ptr<IRender> Create(
        std::shared_ptr<OSWindow> window,
        const std::string &renderer_name,
        bool is_fullscreen
    );
};

}  // namespace Graphics
