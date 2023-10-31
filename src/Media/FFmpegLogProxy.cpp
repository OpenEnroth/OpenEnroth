#include "FFmpegLogProxy.h"

extern "C" {
#include <libavutil/log.h>
}

#include <cassert>
#include <mutex>

#include "FFmpegLogSource.h"

// Category should be a global so that it's registered at program startup.
static constinit FFmpegLogSource globalFFmpegLogSource;
static LogCategory globalFFmpegLogCategory("ffmpeg", &globalFFmpegLogSource);

// Access to global logger ptr must be serialized, so we need a global mutex.
static std::mutex globalFFmpegLogMutex;
static FFmpegLogProxy *globalFFmpegLogger = nullptr;

static void ffmpegLogCallback(void *ptr, int level, const char *format, va_list args) {
    auto guard = std::lock_guard(globalFFmpegLogMutex);

    assert(globalFFmpegLogger);
    globalFFmpegLogger->log(ptr, level, format, args);
}

FFmpegLogProxy::FFmpegLogProxy(Logger *logger): _logger(logger) {
    assert(logger);
    assert(globalFFmpegLogger == nullptr);
    globalFFmpegLogger = this;

    av_log_set_callback(&ffmpegLogCallback);
}

FFmpegLogProxy::~FFmpegLogProxy() {
    auto guard = std::lock_guard(globalFFmpegLogMutex);

    assert(globalFFmpegLogger == this);
    av_log_set_callback(&av_log_default_callback);
    globalFFmpegLogger = nullptr;
}

void FFmpegLogProxy::log(void *ptr, int level, const char *format, va_list args) {
    LogState &state = _stateByThreadId[std::this_thread::get_id()];

    char buffer[4096];
    int status = av_log_format_line2(ptr, level, format, args, buffer, sizeof(buffer), &state.prefixFlag);
    if (status < 0) {
        _logger->trace(globalFFmpegLogCategory, "av_log_format_line2 failed with error code {}", status);
    } else {
        state.message += buffer;
        if (state.message.ends_with('\n')) {
            state.message.pop_back(); // Drop the '\n'.
            _logger->log(globalFFmpegLogCategory, FFmpegLogSource::translateFFmpegLogLevel(level), "{}", state.message);
            state.message.clear();
        }
    }
}










