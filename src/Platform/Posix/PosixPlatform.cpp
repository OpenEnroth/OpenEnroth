#include <memory>

#include "Platform/Sdl/SdlPlatform.h"
#include "Platform/Sdl/SdlLogger.h"
#include "Platform/Platform.h"
#include "Platform/PlatformLogger.h"

std::unique_ptr<Platform> Platform::createStandardPlatform(PlatformLogger *logger) {
    return std::make_unique<SdlPlatform>(logger);
}

std::unique_ptr<PlatformLogger> PlatformLogger::createStandardLogger() {
    return std::make_unique<SdlLogger>();
}
