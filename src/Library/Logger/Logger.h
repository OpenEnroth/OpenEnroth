#pragma once

#include <string_view>
#include <utility>

#include "Platform/PlatformLogger.h"
#include "Utility/Format.h"

class Logger {
 public:
    // TODO(captainurist): this should go to ctor, but that's not doable right now because of a shitload of static
    // variables that call EngineIoc::ResolveLogger.
    PlatformLogger *BaseLogger() const;
    void SetBaseLogger(PlatformLogger *baseLogger);

    template<class... Args>
    void Log(PlatformLogLevel logLevel, fmt::format_string<Args...> fmt, Args&&... args) {
        LogV(logLevel, fmt, fmt::make_format_args(std::forward<Args>(args)...));
    }

    template<class... Args>
    void Info(fmt::format_string<Args...> fmt, Args&&... args) {
        Log(LOG_INFO, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void Warning(fmt::format_string<Args...> fmt, Args&&... args) {
        Log(LOG_WARNING, fmt, std::forward<Args>(args)...);
    }

 private:
    void LogV(PlatformLogLevel logLevel, fmt::string_view fmt, fmt::format_args args);

 private:
    PlatformLogger *baseLogger_ = nullptr;
};
