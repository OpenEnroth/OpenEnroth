#include "MediaLogger.h"

extern "C" {
#include <libavutil/log.h>
}

#include <cassert>
#include <mutex>

#include "Library/Logger/Logger.h"
#include "Platform/PlatformEnums.h"

static std::mutex GlobalMediaLoggerMutex;
static MediaLogger *GlobalMediaLoggerInstance = nullptr;

static void GlobalMediaLoggerCallback(void *ptr, int logLevel, const char *format, va_list args) {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    assert(GlobalMediaLoggerInstance);

    GlobalMediaLoggerInstance->log(ptr, logLevel, format, args);
}

MediaLogger::MediaLogger(Logger *logger): _logger(logger) {
    assert(logger);
}

void MediaLogger::log(void *ptr, int logLevel, const char *format, va_list args) {
    if (!_logger->shouldLog(LOG_VERBOSE))
        return;

    LogState &state = _stateByThreadId[std::this_thread::get_id()];

    char buffer[2048];
    int status = av_log_format_line2(ptr, logLevel, format, args, buffer, sizeof(buffer), &state.prefixFlag);
    if (status < 0) {
        _logger->verbose("av_log_format_line2 failed with error code {}", status);
    } else {
        state.message += buffer;
        if (state.message.ends_with('\n')) {
            _logger->verbose("{}", state.message);
            state.message.clear();
        }
    }
}

void MediaLogger::setGlobalMediaLogger(MediaLogger *logger) {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    GlobalMediaLoggerInstance = logger;

    if (logger) {
        av_log_set_callback(&GlobalMediaLoggerCallback);
    } else {
        av_log_set_callback(nullptr);
    }
}

MediaLogger *MediaLogger::globalMediaLogger() {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    return GlobalMediaLoggerInstance;
}
