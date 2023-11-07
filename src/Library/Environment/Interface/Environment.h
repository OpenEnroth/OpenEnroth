#pragma once

#include <string>
#include <memory>

#include "EnvironmentEnums.h"

class Environment {
 public:
    virtual ~Environment() = default;

    /**
     * @return                          Newly created standard `Environment` instance.
     */
    static std::unique_ptr<Environment> createStandardEnvironment();

    /**
     * Windows-only function for querying the registry. Always returns an empty string on non-Windows systems.
     *
     * @param path                      Registry path to query.
     * @return                          Value at the given path, or an empty string in case of an error.
     */
    virtual std::string queryRegistry(const std::string &path) const = 0;

    /**
     * Accessor for various system paths.
     *
     * @param path                      Path to get.
     */
    virtual std::string path(EnvironmentPath path) const = 0;

    /**
     * Same as `std::getenv`, but takes & returns UTF8-encoded keys and values on all platforms.
     *
     * Returns an empty string for non-existent environment variables, and thus doesn't distinguish between empty and
     * non-existent values (and you shouldn't, either).
     *
     * @param key                       UTF8-encoded name of the environment variable to query.
     * @return                          UTF8-encoded value of the environment variable.
     */
    virtual std::string getenv(const std::string &key) const = 0;
};
