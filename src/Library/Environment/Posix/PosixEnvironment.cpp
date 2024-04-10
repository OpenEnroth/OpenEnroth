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

void PosixEnvironment::setenv(const std::string &key, const std::string &value) const {
    ::setenv(key.c_str(), value.c_str(), 1); // Errors are ignored.
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<PosixEnvironment>();
}
