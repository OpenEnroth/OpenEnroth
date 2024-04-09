#pragma once

#include <string>
#include <memory>

#include "EnvironmentEnums.h"

/**
 * Base class akin to `Platform` that provides an abstraction for the process's environment.
 *
 * All strings accepted by and returned from methods of this class are UTF8-encoded.
 *
 * Why is this class not a part of `Platform`? Mainly for the following reasons:
 * - `Platform` handles an unrelated domain (UI and window management). Using a `NullPlatform` while still relying on
 *   `Environment` working as usual looks like a perfectly valid use case.
 * - Posix and Windows `Environment` implementations don't depend on SDL, but if `Environment` was a part of `Platform`
 *   then we'd still drag SDL with it (and into our unit tests).
 */
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
     * @param path                      UTF8-encoded registry path to query.
     * @return                          UTF8-encoded value at the given path, or an empty string in case of an error.
     */
    [[nodiscard]] virtual std::string queryRegistry(const std::string &path) const = 0;

    /**
     * Accessor for various system paths.
     *
     * @param path                      Path to get.
     * @return                          UTF8-encoded path, or an empty string in case of an error.
     */
    [[nodiscard]] virtual std::string path(EnvironmentPath path) const = 0;

    /**
     * Same as `std::getenv`, but takes & returns UTF8-encoded keys and values on all platforms.
     *
     * Note that on Windows `std::getenv` doesn't switch to UTF8 even if `UnicodeCrt` is used
     * (aka `std::setlocale(LC_ALL, ".UTF-8")`).
     *
     * Returns an empty string for non-existent environment variables, and thus doesn't distinguish between empty and
     * non-existent values (and you shouldn't, either).
     *
     * @param key                       UTF8-encoded name of the environment variable to query.
     * @return                          UTF8-encoded value of the environment variable.
     */
    [[nodiscard]] virtual std::string getenv(const std::string &key) const = 0;

    /**
     * Same as POSIX `setenv(key, value, 1)`. Takes UTF8-encoded keys and values on all platforms.
     *
     * @param key                       UTF8-encoded name of the environment variable to set.
     * @param value                     UTF8-encoded value of the environment variable.
     */
    virtual void setenv(const std::string &key, const std::string &value) const = 0;
};
