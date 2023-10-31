#include <vector>

#include "Application/GamePathResolver.h"

#include "Library/Logger/Logger.h"
#include "Platform/Platform.h"

static std::string _resolvePath(Platform *platform, const char *envVarOverride, const std::vector<const wchar_t *> &registryKeys);

std::string resolveMm6Path(Platform *platform) {
    return _resolvePath(
        platform,
        mm6PathOverrideKey,
        {
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207661253/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM6/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic\x00AE VI/1.0/AppPath",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207661253/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM6/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic\x00AE VI/1.0/AppPath",
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
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207658916/Path",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM7/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic VII/1.0/AppPath",
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
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM8/PATH",
            L"HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
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
    std::string result = platform->storagePath(ANDROID_STORAGE_EXTERNAL);
    if (result.empty())
        result = platform->storagePath(ANDROID_STORAGE_INTERNAL);
    if (result.empty())
        platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");
    return result;
#else
    // TODO (captainurist): we should consider reading Unicode (utf8) strings from win32 registry, as it might contain paths
    // curretnly we convert all strings out of registry into CP_ACP (default windows ansi)
    // it is later on passed to std::filesystem that should be ascii on windows as well
    // this means we will can't handle win32 unicode paths at the time
    const char *envPathStr = std::getenv(envVarOverride);

    std::string envPath{};
    if (envPathStr) {
        envPath = envPathStr;
    }

    if (!envPath.empty()) {
        logger->info("Path override provided: {}={}", envVarOverride, envPathStr);
        return envPath;
    }

    for (auto key : registryKeys) {
        envPath = platform->winQueryRegistry(key);
        if (!envPath.empty()) {
            return envPath;
        }
    }

    return "";
#endif
}
