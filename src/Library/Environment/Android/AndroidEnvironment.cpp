#include "AndroidEnvironment.h"

#include <SDL.h>

std::string AndroidEnvironment::path(EnvironmentPath path) const {
    const char *result = nullptr;
    if (path == PATH_ANDROID_STORAGE_INTERNAL) {
        result = SDL_AndroidGetInternalStoragePath();
    } else if (path == PATH_ANDROID_STORAGE_EXTERNAL) {
        result = SDL_AndroidGetExternalStoragePath();
    }

    // Each application on Android is executed under dedicated user so PATH_HOME is useless.

    if (result)
        return result;
    return {};
}

std::string AndroidEnvironment::getenv(std::string_view key) const {
    const char *result = SDL_getenv(key.data());
    if (result)
        return result;
    return {};
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<AndroidEnvironment>();
}

Environment::GamePaths AndroidEnvironment::getGamePaths(const PathResolutionConfig& config) const {
    Environment::GamePaths result;
    result.reserve(2);
    // ...Android storage paths on Android,...
    if (std::string path = path(PATH_ANDROID_STORAGE_EXTERNAL); !path.empty())
        result.emplace_back(path);

    if (std::string path = path(PATH_ANDROID_STORAGE_INTERNAL); !path.empty())
        result.emplace_back(path);

    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (ANDROID && result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");

    // ...or Library/Application Support in home on macOS.

    return result;
}
