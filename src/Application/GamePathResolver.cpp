#include <vector>

#include "Application/GamePathResolver.h"

#include "Library/Logger/Logger.h"
#include "Library/Environment/Interface/Environment.h"

static std::string _resolvePath(Environment *environment, const char *envVarOverride, const std::vector<const char *> &registryKeys);

std::string resolveMm6Path(Environment *environment) {
    return _resolvePath(
        environment,
        mm6PathOverrideKey,
        {
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207661253/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM6/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic\x00AE VI/1.0/AppPath",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207661253/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM6/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic\x00AE VI/1.0/AppPath",
        }
    );
}


std::string resolveMm7Path(Environment *environment) {
    return _resolvePath(
        environment,
        mm7PathOverrideKey,
        {
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/Games/1207658916/Path",
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207658916/Path",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM7/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic VII/1.0/AppPath",
        }
    );
}


std::string resolveMm8Path(Environment *environment) {
    return _resolvePath(
        environment,
        mm8PathOverrideKey,
        {
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM8/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM8/PATH",
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
        }
    );
}

static std::string _resolvePath(Environment *environment, const char *envVarOverride, const std::vector<const char *> &registryKeys) {
#ifdef __ANDROID__
    // TODO: find a better way to deal with paths and remove this android specific block.
    std::string result = environment->path(PATH_ANDROID_STORAGE_EXTERNAL);
    if (result.empty())
        result = environment->path(PATH_ANDROID_STORAGE_INTERNAL);
    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");
    return result;
#else
    std::string envPath = environment->getenv(envVarOverride);
    if (!envPath.empty()) {
        logger->info("Path override provided: {}={}", envVarOverride, envPath);
        return envPath;
    }

    for (auto key : registryKeys) {
        envPath = environment->queryRegistry(key);
        if (!envPath.empty()) {
            return envPath;
        }
    }

    return "";
#endif
}
