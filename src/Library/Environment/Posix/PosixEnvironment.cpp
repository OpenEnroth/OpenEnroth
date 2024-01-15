#include "PosixEnvironment.h"

#include <cstdlib>
#include <memory>

std::string PosixEnvironment::path(EnvironmentPath path) const {
    if (path == PATH_HOME) {
        return getenv("HOME");
    } else {
        return {};
    }
}

std::string PosixEnvironment::getenv(std::string_view key) const {
    const char *result = std::getenv(key.data());
    if (result)
        return result;
    return {};
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<PosixEnvironment>();
}

Environment::GamePaths GamePaths PosixEnvironment::getGamePaths(const PathResolutionConfig& /*config*/) const {
    std::string home = path(PATH_HOME);
    if home.empty() ? return {}
                    : { home + "/Library/Application Support/OpenEnroth" };
}
