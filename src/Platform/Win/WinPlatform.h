#pragma once

#include <string>

#include "Platform/Sdl/SdlPlatform.h"

class WinPlatform : public SdlPlatform {
 public:
    using SdlPlatform::SdlPlatform;

    virtual std::string WinQueryRegistry(const std::wstring &path) const override;
};
