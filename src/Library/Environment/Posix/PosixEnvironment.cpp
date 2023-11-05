#include "PosixEnvironment.h"

#include <cstdlib>
#include <memory>

std::string PosixEnvironment::queryRegistry(const std::string &path) const {
    return {};
}

std::string PosixEnvironment::path(EnvironmentPath path) const {
    if (path == PATH_HOME) {
        return getenv("HOME");
    } else {
        return {};
    }
}

std::string PosixEnvironment::getenv(const std::string &key) const {
    const char *result = std::getenv(key.c_str());
    if (result)
        return result;
    return {};
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<PosixEnvironment>();
}
