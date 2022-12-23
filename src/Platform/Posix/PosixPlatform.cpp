#include <memory>

#include "Platform/Sdl/SdlPlatform.h"

std::unique_ptr<Platform> Platform::CreateStandardPlatform(PlatformLogLevel platformLogLevel, PlatformCreationOptions /*options*/) {
    return std::make_unique<SdlPlatform>(platformLogLevel);
}
