#include "GamePathResolver.h"

#include "Library/Environment/Interface/PathResolutionConfig.h"

namespace {
constexpr PathResolutionConfig mm6Config {
    mm6PathOverrideKey,
    {
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207661253/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM6/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic\xC2\xAE VI/1.0/AppPath", // \xC2\xAE is (R) in utf-8.
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207661253/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM6/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic\xC2\xAE VI/1.0/AppPath"
    }
};


constexpr PathResolutionConfig mm7Config {
    mm7PathOverrideKey,
    {
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207658916/Path",
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207658916/Path",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM7/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic VII/1.0/AppPath"
    }
};


constexpr PathResolutionConfig mm8Config {
    mm8PathOverrideKey,
    {
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM8/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM8/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath"
    }
};
} // namespace

Environment::GamePaths resolveMm6Paths(const Environment &environment) {
    return environment.resolveGamePath(mm6Config);
}

Environment::GamePaths resolveMm7Paths(const Environment &environment) {
    return environment.resolveGamePath(mm7Config);
}

Environment::GamePaths resolveMm8Paths(const Environment &environment) {
    return environment.resolveGamePath(mm8Config);
}
