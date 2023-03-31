#include "MediaLogger.h"

extern "C" {
#include <libavutil/log.h>
}

#include <cassert>
#include <mutex>

#include "Engine/Engine.h" // TODO(captainurist): drop!

static std::mutex GlobalMediaLoggerMutex;
static MediaLogger *GlobalMediaLoggerInstance = nullptr;

static void GlobalMediaLoggerCallback(void *ptr, int logLevel, const char *format, va_list args) {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    assert(GlobalMediaLoggerInstance);

    GlobalMediaLoggerInstance->Log(ptr, logLevel, format, args);
}

MediaLogger::MediaLogger(Logger *logger): logger_(logger) {
    assert(logger);
}

void MediaLogger::Log(void *ptr, int logLevel, const char *format, va_list args) {
    if (!engine->config->debug.VerboseLogging.value()) // TODO(captainurist): just handle log levels & log categories properly
        return;

    LogState& state = stateByThreadId_[std::this_thread::get_id()];

    char buffer[2048];
    int status = av_log_format_line2(ptr, logLevel, format, args, buffer, sizeof(buffer), &state.prefixFlag);
    if (status < 0) {
        logger_->Info("av_log_format_line2 failed with error code {}", status);
    } else {
        state.message += buffer;
        if (state.message.ends_with('\n')) {
            logger_->Info("{}", state.message.c_str());
            state.message.clear();
        }
    }
}

void MediaLogger::SetGlobalMediaLogger(MediaLogger *logger) {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    GlobalMediaLoggerInstance = logger;

    if (logger) {
        av_log_set_callback(&GlobalMediaLoggerCallback);
    } else {
        av_log_set_callback(nullptr);
    }
}

MediaLogger *MediaLogger::GlobalMediaLogger() {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    return GlobalMediaLoggerInstance;
}
