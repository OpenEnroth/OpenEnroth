#include "FFmpegLogSource.h"

extern "C" {
#include <libavutil/log.h>
}

#include <cassert>

LogLevel FFmpegLogSource::level() const {
    return translateFFmpegLogLevel(av_log_get_level());
}

void FFmpegLogSource::setLevel(LogLevel level) {
    av_log_set_level(translateLoggerLogLevel(level));
}

LogLevel FFmpegLogSource::translateFFmpegLogLevel(int level) {
    // Best effort translation here.
    if (level <= AV_LOG_QUIET) {
        return LOG_NONE;
    } else if (level <= AV_LOG_PANIC) {
        return LOG_CRITICAL;
    } else if (level <= AV_LOG_FATAL) {
        return LOG_CRITICAL; // AV_LOG_PANIC & AV_LOG_FATAL are both translated into LOG_CRITICAL.
    } else if (level <= AV_LOG_ERROR) {
        return LOG_ERROR;
    } else if (level <= AV_LOG_WARNING) {
        return LOG_WARNING;
    } else if (level <= AV_LOG_INFO) {
        return LOG_INFO;
    } else if (level <= AV_LOG_VERBOSE) {
        return LOG_DEBUG;
    } else if (level <= AV_LOG_DEBUG) {
        return LOG_DEBUG; // AV_LOG_DEBUG & AV_LOG_VERBOSE are both translated into LOG_DEBUG.
    } else {
        return LOG_TRACE;
    }
}

int FFmpegLogSource::translateLoggerLogLevel(LogLevel level) {
    switch (level) {
    case LOG_TRACE:     return AV_LOG_TRACE;
    case LOG_DEBUG:     return AV_LOG_DEBUG; // max(AV_LOG_DEBUG, AV_LOG_VERBOSE)
    case LOG_INFO:      return AV_LOG_INFO;
    case LOG_WARNING:   return AV_LOG_WARNING;
    case LOG_ERROR:     return AV_LOG_ERROR;
    case LOG_CRITICAL:  return AV_LOG_FATAL; // max(AV_LOG_PANIC, AV_LOG_FATAL)
    case LOG_NONE:      return AV_LOG_QUIET;
    default:
        assert(false);
        return AV_LOG_TRACE;
    }
}
