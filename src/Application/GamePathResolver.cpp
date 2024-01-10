#include "GamePathResolver.h"

#include "Library/Logger/Logger.h"
#include "Library/Environment/Interface/Environment.h"

#include "filesystem"

namespace {

struct PathResolutionConfig {
    using RegistryKeys = std::initializer_list<std::string_view>;

    std::string_view overrideEnvKey;
    RegistryKeys registryKeys;
};


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


std::vector<std::string> resolvePaths(const Environment &environment, const PathResolutionConfig &config) {
    // If we have a path override then it'll be the only path we'll check.
    std::string_view envKey = config.overrideEnvKey;
    if (std::string envPath = environment.getenv(envKey); !envPath.empty()) {
        logger->info("Path override provided, '{}={}'.", envKey, envPath);
        return { envPath };
    }

    std::vector<std::string> result;
#ifdef _WIN32
    std::size_t pathSize = config.registryKeys.size() + 1;
#elif __APPLE__
    constexpr std::size_t pathSize = 2;
#elif __ANDROID__
    constexpr std::size_t pathSize = 3;
#else
    constexpr std::size_t pathSize = 1;
#endif

    result.reserve(pathSize);

#ifdef _WIN32
    // Then we check paths from registry on Windows,...
    for (const auto& registryKey : config.registryKeys) {
        if (std::string registryPath = environment.queryRegistry(registryKey); !registryPath.empty()) {
            result.emplace_back(registryPath);
        }
    }
#elif __APPLE__
    std::string home = environment.path(PATH_HOME);
    if (!home.empty())
        result.emplace_back(home + "/Library/Application Support/OpenEnroth");
#elif __ANDROID__
    // ...Android storage paths on Android,...
    if (std::string path = environment.path(PATH_ANDROID_STORAGE_EXTERNAL); !path.empty())
        result.emplace_back(path);

    if (std::string path = environment.path(PATH_ANDROID_STORAGE_INTERNAL); !path.empty())
        result.emplace_back(path);

    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (ANDROID && result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");

    // ...or Library/Application Support in home on macOS.
#endif

    result.emplace_back(std::filesystem::current_path().string());
    return result;
}
} // namespace

std::vector<std::string> resolveMm6Paths(const Environment &environment) {
    return resolvePaths(environment, mm6Config);
}

std::vector<std::string> resolveMm7Paths(const Environment &environment) {
    return resolvePaths(environment, mm7Config);
}

std::vector<std::string> resolveMm8Paths(const Environment &environment) {
    return resolvePaths(environment, mm8Config);
}
