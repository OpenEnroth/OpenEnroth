#pragma once

#include "Platform/Sdl/SdlPlatform.h"

class WinPlatform : public SdlPlatform {
public:
    WinPlatform(Log *log);

    virtual void WinEnsureConsole() const override;
};
