#pragma once

#include <cstdarg>
#include <unordered_map>
#include <string>
#include <thread>

#include "Library/Logger/Logger.h"

class Logger;

// TODO(captainurist): rework when I get to logging, this should just be a way to hook into av_log, an API taking std::function.
class MediaLogger {
 public:
    explicit MediaLogger(Logger *logger);

    void log(void *ptr, int logLevel, const char *format, va_list args);

    static void setGlobalMediaLogger(MediaLogger *logger);
    static MediaLogger *globalMediaLogger();

 private:
    struct LogState {
        int prefixFlag = 1;
        std::string message;
    };

 private:
    Logger *_logger = nullptr;
    std::unordered_map<std::thread::id, LogState> _stateByThreadId;
};
