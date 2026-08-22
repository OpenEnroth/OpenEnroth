#include "PathResolver.h"

#include <string>
#include <vector>

#include "Library/Logger/Logger.h"
#include "Library/Environment/Interface/Environment.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"

static const std::vector<std::string_view> globalValidateList = {
    {"anims/magic7.vid"},
    {"anims/might7.vid"},
    {"data/bitmaps.lod"},
//    {"data/d3dbitmap.hwl"}, // We're not using HWL textures, so these are not required.
//    {"data/d3dsprite.hwl"},
    {"data/events.lod"},
    {"data/games.lod"},
    {"data/icons.lod"},
    {"data/sprites.lod"},
    {"sounds/audio.snd"}
};

struct PathResolutionConfig {
    const char *overrideEnvKey = nullptr;
    const std::vector<const char *> registryKeys;

    constexpr PathResolutionConfig(const char* overrideEnvKey, std::initializer_list<const char*> registryKeys)
        : overrideEnvKey(overrideEnvKey), registryKeys(registryKeys) {}
};

static const PathResolutionConfig mm6Config = {
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

static const PathResolutionConfig mm7Config = {
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

static const PathResolutionConfig mm8Config = {
    mm8PathOverrideKey,
    {
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM8/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/GOGMM8/PATH",
        "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath"
    }
};

static std::vector<NativePath> resolvePaths(Environment *environment, const PathResolutionConfig &config) {
    // If we have a path override then it'll be the only path we'll check.
    std::string envPath = environment->getenv(config.overrideEnvKey);
    if (!envPath.empty()) {
        MM_INFO("Path override provided, '{}={}'.", config.overrideEnvKey, envPath);
        return {NativePath::fromWtf8(envPath)};
    }

    std::vector<NativePath> result;

    // Otherwise we check PWD first.
    result.push_back(os::cwd());

    // Then we check paths from registry on Windows,...
    for (const char *registryKey : config.registryKeys) {
        if (registryKey) {
            std::string registryPath = environment->queryRegistry(registryKey);
            if (!registryPath.empty())
                result.push_back(NativePath::fromWtf8(registryPath));
        }
    }

#ifdef __ANDROID__
    // ...Android storage paths on Android,...
    std::string externalPath = environment->path(PATH_ANDROID_STORAGE_EXTERNAL);
    if (!externalPath.empty())
        result.push_back(NativePath::fromWtf8(externalPath));
    std::string internalPath = environment->path(PATH_ANDROID_STORAGE_INTERNAL);
    if (!internalPath.empty())
        result.push_back(NativePath::fromWtf8(internalPath));
    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (ANDROID && result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");
#endif

#ifdef __APPLE__
    // ...or Library/Application Support in home on macOS.
    std::string home = environment->path(PATH_HOME);
    if (!home.empty())
        result.push_back(NativePath::fromWtf8(home + "/Library/Application Support/OpenEnroth"));
#endif

    return result;
}

std::vector<NativePath> resolveMm6Paths(Environment *environment) {
    return resolvePaths(environment, mm6Config);
}

std::vector<NativePath> resolveMm7Paths(Environment *environment) {
    return resolvePaths(environment, mm7Config);
}

std::vector<NativePath> resolveMm8Paths(Environment *environment) {
    return resolvePaths(environment, mm8Config);
}

bool validateMm7Path(const NativePath &dataPath, std::string *missingFile) {
    DirectoryFileSystem dirFs(dataPath);
    LowercaseFileSystem lowerFs(&dirFs);

    for (std::string_view entry : globalValidateList) {
        if (!lowerFs.exists(entry)) {
            *missingFile = entry;
            return false;
        }
    }

    return true;
}

NativePath resolveMm7UserPath(Environment *environment) {
#ifdef _WINDOWS
    std::string savedGames = environment->path(PATH_WINDOWS_SAVED_GAMES);
    if (savedGames.empty())
        return {}; // Shouldn't really happen.
    return NativePath::fromWtf8(fmt::format("{}/OpenEnroth", savedGames));
#elif __ANDROID__
    return NativePath::fromWtf8(fmt::format("{}/.openenroth", environment->path(PATH_ANDROID_STORAGE_INTERNAL)));
#else // Mac & linux
    return NativePath::fromWtf8(fmt::format("{}/.openenroth", environment->path(PATH_HOME)));
#endif
}
