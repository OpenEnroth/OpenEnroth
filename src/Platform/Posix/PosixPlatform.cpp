#include <memory>

#include "Platform/Sdl/SdlPlatform.h"

std::unique_ptr<Platform> Platform::createStandardPlatform(Logger *logger) {
    return std::make_unique<SdlPlatform>(logger);
}
