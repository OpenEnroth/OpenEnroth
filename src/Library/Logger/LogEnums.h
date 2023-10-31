#pragma once

#include "Library/Serialization/SerializationFwd.h"

/**
 * Log level as used by `Logger`.
 */
enum class LogLevel {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
};
using enum LogLevel;
MM_DECLARE_SERIALIZATION_FUNCTIONS(LogLevel)
