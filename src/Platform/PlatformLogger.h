#pragma once

#include <memory>

#include "PlatformEnums.h"

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

    static std::unique_ptr<PlatformLogger> CreateStandardLogger(PlatformLoggerOptions options);

    virtual void SetLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) = 0;
    virtual PlatformLogLevel LogLevel(PlatformLogCategory category) const = 0;

    /**
     * Logs provided message. The message will be ignored if the provided log level is lower than the log level set
     * for the provided category. It is advised to also do the log level check at the call site to avoid message
     * formatting overhead:
     * \code
     * if (logger->LogLevel(ApplicationLog) <= LogDebug)
     *     logger->Log(ApplicationLog, LogDebug, FormatMessage("blablabla %s %s", s1, s2).c_str());
     * \endcode
     *
     * This function is thread-safe.
     *
     * @param category                  Message log category.
     * @param logLevel                  Message log level.
     * @param message                   Message to log.
     */
    virtual void Log(PlatformLogCategory category, PlatformLogLevel logLevel, const char *message) = 0;
};
