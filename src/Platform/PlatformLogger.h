#pragma once

#include <memory>

#include "PlatformEnums.h"

// TODO(captainurist): this should be just a callback, adding a log factory to platform was a bad idea after all.
/**
 * Platform-specific logger that takes all the quirks of a specific platform into account (e.g. calling
 * `OutputDebugString` / `WriteConsole` on Windows).
 *
 * Note that platform logger doesn't provide an API for log formatting and log category management. If needed, this
 * functionality should be added at the next abstraction layer.
 *
 * The two existing log categories are provided to differentiate logging calls made by user code and by the platform
 * implementation itself.
 */
class PlatformLogger {
 public:
    virtual ~PlatformLogger() = default;

    static std::unique_ptr<PlatformLogger> createStandardLogger();

    virtual void setLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) = 0;
    virtual PlatformLogLevel logLevel(PlatformLogCategory category) const = 0;

    /**
     * Logs provided message. The message will be ignored if the provided log level is lower than the log level set
     * for the provided category. It is advised to also do the log level check at the call site to avoid message
     * formatting overhead:
     * \code
     * if (logger->logLevel(APPLICATION_LOG) <= LOG_DEBUG)
     *     logger->log(APPLICATION_LOG, LOG_DEBUG, fmt::format("blablabla {}", s1).c_str());
     * \endcode
     *
     * This function is thread-safe.
     *
     * @param category                  Message log category.
     * @param logLevel                  Message log level.
     * @param message                   Message to log.
     */
    virtual void log(PlatformLogCategory category, PlatformLogLevel logLevel, const char *message) = 0;
};
