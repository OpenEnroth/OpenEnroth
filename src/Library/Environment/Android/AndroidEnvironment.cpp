#include "AndroidEnvironment.h"

#include <SDL.h>

#include "Library/Environment/Interface/utils.h"


std::string AndroidEnvironment::path(EnvironmentPath env_path) const {
    const char* result = nullptr;
    switch (env_path) {
    case EnvironmentPath::PATH_ANDROID_STORAGE_INTERNAL: {
        result = SDL_AndroidGetInternalStoragePath();
        break;
    }
    case EnvironmentPath::PATH_ANDROID_STORAGE_EXTERNAL: {
        result = SDL_AndroidGetExternalStoragePath();
        break;
    }
    }
    return getStringOr(result);
}

std::string AndroidEnvironment::getenv(std::string_view key) const {
    return getStringOr(SDL_getenv(key.data()));
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<AndroidEnvironment>();
}

Environment::GamePaths AndroidEnvironment::getGamePaths(const PathResolutionConfig& config) const {
    Environment::GamePaths result;
    result.reserve(2);
    // ...Android storage paths on Android,...
    if (std::string path_item = path(EnvironmentPath::PATH_ANDROID_STORAGE_EXTERNAL); !path_item.empty()) {
        result.emplace_back(path_item);
    }
        

    if (std::string path_item = path(EnvironmentPath::PATH_ANDROID_STORAGE_INTERNAL); !path_item.empty()) {
        result.emplace_back(path_item);
    }
        
    // TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
    //if (ANDROID && result.empty())
    //    platform->showMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");

    // ...or Library/Application Support in home on macOS.

    return result;
}
