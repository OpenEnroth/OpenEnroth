#pragma once

#include <cstdarg>

#include <unordered_map>
#include <string>
#include <thread>

class FFmpegLogProxy {
 public:
    FFmpegLogProxy();
    ~FFmpegLogProxy();

    void log(void *ptr, int level, const char *format, va_list args);

 private:
    struct LogState {
        int prefixFlag = 1;
        std::string message;
    };

 private:
    std::unordered_map<std::thread::id, LogState> _stateByThreadId;
};
