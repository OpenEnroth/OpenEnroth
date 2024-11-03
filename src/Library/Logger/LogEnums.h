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

namespace detail {
constexpr int LOG_NONE_BARRIER = static_cast<int>(LOG_CRITICAL) + 1;
} // namespace detail
