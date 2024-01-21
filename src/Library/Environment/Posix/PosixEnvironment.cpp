#include "PosixEnvironment.h"

#include "Environment/Interface/utils.h"

#include <cstdlib>
#include <memory>


std::string PosixEnvironment::path(EnvironmentPath path) const {
    return path == EnvironmentPath::PATH_HOME ? getenv("HOME")
                                              : {};
}


std::string PosixEnvironment::getenv(std::string_view key) const {
    return getStringOr(std::getenv(key.data()));
}



std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<PosixEnvironment>();
}

Environment::GamePaths PosixEnvironment::getGamePaths(const PathResolutionConfig& /*config*/) const {
    std::string home = path(PATH_HOME);
    return home.empty() ? {}
                        : { home + "/Library/Application Support/OpenEnroth" };
}
