#include "MediaLogger.h"

#include <cassert>

#include <Engine/Engine.h> // TODO(captainurist): drop!

extern "C" {
#include <libavutil/log.h>
}

static constinit std::mutex GlobalMediaLoggerMutex;
static constinit MediaLogger *GlobalMediaLoggerInstance = nullptr;

static void GlobalMediaLoggerCallback(void *ptr, int logLevel, const char *format, va_list args) {
    auto lock = std::unique_lock(GlobalMediaLoggerMutex);

    assert(GlobalMediaLoggerInstance);

    GlobalMediaLoggerInstance->Log(ptr, logLevel, format, args);
}

MediaLogger::MediaLogger(::Log *logger): logger_(logger) {
    assert(logger);
}

void MediaLogger::Log(void *ptr, int logLevel, const char *format, va_list args) {
    if (!engine->config->debug.VerboseLogging.Get()) // TODO(captainurist)
        return;

    LogState& state = stateByThreadId_[std::this_thread::get_id()];

    char buffer[2048];
    int status = av_log_format_line2(ptr, logLevel, format, args, buffer, sizeof(buffer), &state.prefixFlag);
    if (status < 0) {
        logger_->Write(LogInfo, "av_log_format_line2 failed with error code %d", status);
    } else {
        state.message += buffer;
        if (state.message.ends_with('\n')) {
            logger_->Write(LogInfo, "%s", state.message.c_str());
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
