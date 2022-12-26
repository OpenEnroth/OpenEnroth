#pragma once

#include <cstdarg>

#include <unordered_map>
#include <string>
#include <thread>

#include "Library/Logger/Logger.h"

class MediaLogger {
 public:
    explicit MediaLogger(Logger *logger);

    void Log(void *ptr, int logLevel, const char *format, va_list args);

    static void SetGlobalMediaLogger(MediaLogger *logger);
    static MediaLogger *GlobalMediaLogger();

 private:
    struct LogState {
        int prefixFlag = 1;
        std::string message;
    };

 private:
    Logger *logger_ = nullptr;
    std::unordered_map<std::thread::id, LogState> stateByThreadId_;
};
