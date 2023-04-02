#include <vector>

#include "Application/GamePathResolver.h"

#include "Engine/EngineIocContainer.h"
#include "Library/Logger/Logger.h"
#include "Platform/Platform.h"

#include "Utility/System.h"

static std::string _resolvePath(Platform *platform, const char *envVarOverride, const std::vector<const wchar_t *> &registryKeys);

std::string resolveMm6Path(Platform *platform) {
    return _resolvePath(
        platform,
        mm6PathOverrideKey,
        {
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207661253/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM6/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic\x00AE VI/1.0/AppPath",
        }
    );
}


std::string resolveMm7Path(Platform *platform) {
    return _resolvePath(
        platform,
        mm7PathOverrideKey,
        {
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207658916/Path",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
        }
    );
}


std::string resolveMm8Path(Platform *platform) {
    return _resolvePath(
        platform,
        mm8PathOverrideKey,
        {
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM8/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
        }
    );
}


static std::string _resolvePath(
    Platform *platform,
    const char *envVarOverride,
    const std::vector<const wchar_t *> &registryKeys
) {
#ifdef __ANDROID__
    // TODO: find a better way to deal with paths and remove this android specific block.
    std::string result = platform->StoragePath(ANDROID_STORAGE_EXTERNAL);
    if (result.empty()) {
        result = platform->StoragePath(ANDROID_STORAGE_INTERNAL);
    }

    return result;
#else
    std::string result = env(envVarOverride);

    if (!result.empty()) {
        EngineIocContainer::ResolveLogger()->info("Path override provided: {}={}", envVarOverride, result);
        return result;
    }

    for (auto key : registryKeys) {
        result = platform->winQueryRegistry(key);
        if (!result.empty()) {
            return result;
        }
    }

    return "";
#endif
}
