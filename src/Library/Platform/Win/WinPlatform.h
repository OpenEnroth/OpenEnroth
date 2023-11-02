#pragma once

#include <string>

#include "Library/Platform/Sdl/SdlPlatform.h"

class WinPlatform : public SdlPlatform {
 public:
    using SdlPlatform::SdlPlatform;

    virtual std::string winQueryRegistry(const std::wstring &path) const override;
};
