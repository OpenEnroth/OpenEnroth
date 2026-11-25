#include "PathResolver.h"

#include <string>
#include <utility>
#include <vector>

#include "Library/Logger/Logger.h"
#include "Library/Environment/Interface/Environment.h"
#include "Library/FileSystem/Native/NativeFileSystem.h"
#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"

#include "Utility/System/Fs.h"

// MM7's data folder layout. Also used as the default for any data path that isn't detected as MM8, see
// `detectGameType` below.
static const std::vector<std::string_view> mm7ValidateList = {
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

// MM8's data folder layout differs from MM7's: videos are named differently, and there's no `events.lod` /
// `sounds` dir in a vanilla install.
static const std::vector<std::string_view> mm8ValidateList = {
    {"anims/magicdod.vid"},
    {"anims/mightdod.vid"},
    {"data/bitmaps.lod"},
    {"data/games.lod"},
    {"data/icons.lod"},
    {"data/sprites.lod"}
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

static std::vector<Path> resolvePaths(Environment *environment, const PathResolutionConfig &config) {
    // If we have a path override then it'll be the only path we'll check.
    std::string envPath = environment->getenv(config.overrideEnvKey);
    if (!envPath.empty()) {
        MM_INFO("Path override provided, '{}={}'.", config.overrideEnvKey, envPath);
        return {Path(std::move(envPath))};
    }

    std::vector<Path> result;

    // Otherwise we check PWD first.
    result.push_back(fs::cwd());

    // Then we check paths from registry on Windows,...
    for (const char *registryKey : config.registryKeys) {
        if (registryKey) {
            std::string registryPath = environment->queryRegistry(registryKey);
            if (!registryPath.empty())
                result.push_back(Path(std::move(registryPath)));
        }
    }

#ifdef __ANDROID__
    // ...Android storage paths on Android,...
    std::string externalPath = environment->path(PATH_ANDROID_STORAGE_EXTERNAL);
    if (!externalPath.empty())
        result.push_back(Path(std::move(externalPath)));
    std::string internalPath = environment->path(PATH_ANDROID_STORAGE_INTERNAL);
    if (!internalPath.empty())
        result.push_back(Path(std::move(internalPath)));
    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (ANDROID && result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");
#endif

#ifdef __APPLE__
    // ...or Library/Application Support in home on macOS.
    std::string home = environment->path(PATH_HOME);
    if (!home.empty())
        result.push_back(Path(std::move(home)) / "Library/Application Support/OpenEnroth");
#endif

    return result;
}

std::vector<Path> resolveMm6Paths(Environment *environment) {
    return resolvePaths(environment, mm6Config);
}

std::vector<Path> resolveMm7Paths(Environment *environment) {
    return resolvePaths(environment, mm7Config);
}

std::vector<Path> resolveMm8Paths(Environment *environment) {
    return resolvePaths(environment, mm8Config);
}

// Note: `data/games.lod` is not a useful signal here, even though it's present in every MM6/7/8 install and its
// LOD header carries a version tag (see `LodVersion`) - that tag identifies the "default new game" save-state
// format, which is `LOD_VERSION_MM6_GAME` for all three games, not the content format of the install. Detect
// the game version from a file that's actually unique to each game's layout instead.
GameType detectGameType(FileSystem &fs) {
    return fs.exists("anims/magicdod.vid") ? GAME_TYPE_MM8 : GAME_TYPE_MM7;
}

GameType detectGameType(PathView dataPath) {
    NativeFileSystem dirFs(dataPath);
    LowercaseFileSystem lowerFs(&dirFs);
    return detectGameType(lowerFs);
}

bool validateDataPath(PathView dataPath, std::string *missingFile) {
    NativeFileSystem dirFs(dataPath);
    LowercaseFileSystem lowerFs(&dirFs);

    const std::vector<std::string_view> &validateList = detectGameType(lowerFs) == GAME_TYPE_MM8 ? mm8ValidateList : mm7ValidateList;

    for (std::string_view entry : validateList) {
        if (!lowerFs.exists(entry)) {
            *missingFile = entry;
            return false;
        }
    }

    return true;
}

Path resolveMm7UserPath(Environment *environment) {
#ifdef _WINDOWS
    std::string savedGames = environment->path(PATH_WINDOWS_SAVED_GAMES);
    if (savedGames.empty())
        return fs::cwd(); // Shouldn't really happen.
    return Path(std::move(savedGames)) / "OpenEnroth";
#elif __ANDROID__
    return Path(environment->path(PATH_ANDROID_STORAGE_INTERNAL)) / ".openenroth";
#else // Mac & linux
    return Path(environment->path(PATH_HOME)) / ".openenroth";
#endif
}
