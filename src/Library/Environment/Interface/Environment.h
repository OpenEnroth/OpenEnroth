#pragma once

#include <string>
#include <memory>

#include "EnvironmentEnums.h"

/**
 * Base class akin to `Platform` that provides an abstraction for the process's environment.
 *
 * Strings accepted by and returned from methods of this class are WTF8-encoded on Windows - the underlying wide
 * strings are converted to and from WTF8. On POSIX they are just byte strings that are passed through as-is - POSIX
 * doesn't guarantee UTF8 in paths, or anywhere else.
 *
 * Why is this class not a part of `Platform`? Mainly for the following reasons:
 * - `Platform` handles an unrelated domain (UI and window management). Using a `NullPlatform` while still relying on
 *   `Environment` working as usual looks like a perfectly valid use case.
 * - Posix and Windows `Environment` implementations don't depend on SDL, but if `Environment` was a part of `Platform`
 *   then we'd still drag SDL with it (and into our unit tests).
 * - `Environment` doesn't depend on `Logger`.
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
     * @param path                      Registry path to query, WTF8-encoded on Windows.
     * @return                          Value at the given path, WTF8-encoded on Windows, or an empty string in case
     *                                  of an error.
     */
    [[nodiscard]] virtual std::string queryRegistry(const std::string &path) const = 0;

    /**
     * Accessor for various system paths.
     *
     * @param path                      Path to get.
     * @return                          Path, WTF8-encoded on Windows, or an empty string in case of an error.
     */
    [[nodiscard]] virtual std::string path(EnvironmentPath path) const = 0;

    /**
     * Same as `std::getenv`, but takes & returns WTF8-encoded keys and values on Windows.
     *
     * Note that on Windows `std::getenv` doesn't switch to UTF8 even if `UnicodeCrt` is used
     * (aka `std::setlocale(LC_ALL, ".UTF-8")`).
     *
     * Returns an empty string for non-existent environment variables, and thus doesn't distinguish between empty and
     * non-existent values (and you shouldn't, either).
     *
     * @param key                       Name of the environment variable to query, WTF8-encoded on Windows.
     * @return                          Value of the environment variable, WTF8-encoded on Windows.
     */
    [[nodiscard]] virtual std::string getenv(const std::string &key) const = 0;

    /**
     * Same as POSIX `setenv(key, value, 1)`. Takes WTF8-encoded keys and values on Windows.
     *
     * @param key                       Name of the environment variable to set, WTF8-encoded on Windows.
     * @param value                     Value of the environment variable, WTF8-encoded on Windows.
     */
    virtual void setenv(const std::string &key, const std::string &value) const = 0;
};
