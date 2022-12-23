#pragma once

#include <string_view>

enum class PlatformLogLevel {
    LogVerbose,
    LogDebug,
    LogInfo,
    LogWarning,
    LogError,
    LogCritical
};
using enum PlatformLogLevel;

enum class PlatformLogCategory {
    PlatformLog,
    ApplicationLog
};
using enum PlatformLogCategory;

/**
 * Platform-specific logger that takes all the quirks of a specific platform into account (e.g. calling
 * `OutputDebugString` / `WriteConsole` on Windows).
 *
 * Note that platform logger doesn't provide log formatting and log category management. If needed, this
 * functionality should be added at the next abstraction layer.
 *
 * The two existing categories are provided to differentiate logging calls made via this interface, and the ones
 * made by the platform implementation itself.
 */
class PlatformLogger {
 public:
    virtual ~PlatformLogger() = default;

    virtual void SetLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) = 0;
    virtual PlatformLogLevel LogLevel(PlatformLogCategory category) const = 0;

    /**
     * Logs provided message using `ApplicationLog` category. The message will be ignored if the provided log level
     * is lower than the log level set for `ApplicationLog` category.
     *
     * @param logLevel                  Message log level.
     * @param message                   Message to log.
     */
    virtual void Log(PlatformLogLevel logLevel, const char* message) = 0;
};
