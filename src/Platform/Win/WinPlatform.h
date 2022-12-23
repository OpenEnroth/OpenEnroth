#pragma once

#include <string>

#include "Platform/Sdl/SdlPlatform.h"

class WinPlatform : public SdlPlatform {
 public:
    explicit WinPlatform(PlatformLogLevel platformLogLevel);

    virtual std::string WinQueryRegistry(const std::string &path) const override;
};
