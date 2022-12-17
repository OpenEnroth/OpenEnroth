#include "WinPlatform.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

WinPlatform::WinPlatform(Log *log) : SdlPlatform(log) {}

void WinPlatform::WinEnsureConsole() const {
    if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
    }
}

std::unique_ptr<Platform> Platform::CreateStandardPlatform(Log *log) {
    return std::make_unique<WinPlatform>(log);
}
