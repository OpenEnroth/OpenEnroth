#pragma once

#include <cstdarg>

#include <unordered_map>
#include <string>
#include <thread>

#include "Library/Logger/Logger.h"

class FFmpegLogProxy {
 public:
    explicit FFmpegLogProxy(Logger *logger);
    ~FFmpegLogProxy();

    void log(void *ptr, int level, const char *format, va_list args);

 private:
    struct LogState {
        int prefixFlag = 1;
        std::string message;
    };

 private:
    Logger *_logger = nullptr;
    std::unordered_map<std::thread::id, LogState> _stateByThreadId;
};
