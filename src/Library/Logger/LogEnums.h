#pragma once

#include "Library/Serialization/SerializationFwd.h"

/**
 * Log level as used by `Logger`.
 */
enum class LogLevel {
    LOG_NONE, // Special log level essentially meaning "please don't log anything." Trying to log a message with this
              // log level will always drop it.
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL,
};
using enum LogLevel;
MM_DECLARE_SERIALIZATION_FUNCTIONS(LogLevel)

/**
 * Same as `toString`, but usable from constant-initialized code. The serialization tables are built at dynamic
 * initialization time, so `toString` throws when called before that - which is exactly when `FallbackLogSink` runs.
 *
 * `LogEnums.cpp` feeds this into the serialization table, so the two can't drift apart.
 *
 * TODO(captainurist): drop this once the serialization lib is constinit-friendly.
 */
constexpr const char *logLevelName(LogLevel level) {
    switch (level) {
    case LOG_NONE:      return "none";
    case LOG_TRACE:     return "trace";
    case LOG_DEBUG:     return "debug";
    case LOG_INFO:      return "info";
    case LOG_WARNING:   return "warning";
    case LOG_ERROR:     return "error";
    case LOG_CRITICAL:  return "critical";
    default:            return "?";
    }
}

namespace detail {
constexpr int LOG_NONE_BARRIER = static_cast<int>(LOG_CRITICAL) + 1;
} // namespace detail
