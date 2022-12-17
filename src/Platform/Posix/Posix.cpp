#include "Platform/Posix/Posix.h"

#include "Platform/Sdl/SdlPlatform.h"

std::unique_ptr<Platform> Platform::CreateStandardPlatform(Log *log) {
    return std::make_unique<SdlPlatform>(log);
}
