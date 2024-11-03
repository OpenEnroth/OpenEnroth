#pragma once

#include <cassert>

#include <spdlog/common.h> // NOLINT: not a C header.

#include "LogEnums.h"

inline spdlog::level::level_enum translateLogLevel(LogLevel level) {
    switch (level) {
    case LOG_TRACE:     return spdlog::level::trace;
    case LOG_DEBUG:     return spdlog::level::debug;
    case LOG_INFO:      return spdlog::level::info;
    case LOG_WARNING:   return spdlog::level::warn;
    case LOG_ERROR:     return spdlog::level::err;
    case LOG_CRITICAL:  return spdlog::level::critical;
    case LOG_NONE:      return spdlog::level::off;
    default:
        assert(false);
        return spdlog::level::trace;
    }
}
